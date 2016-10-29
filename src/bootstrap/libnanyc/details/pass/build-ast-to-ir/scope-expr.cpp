#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/utils/check-for-valid-identifier-name.h"

using namespace Yuni;



namespace Nany
{
namespace IR
{
namespace Producer
{

	namespace {


	bool visitScope(Scope& scope, AST::Node& node, bool allowScope)
	{
		return allowScope ? scope.visitASTExprScope(node)
			: unexpectedNode(node, "invalid scope declaration [expr/continuation]");
	}


	bool visitASTExprRegister(Scope& scope, AST::Node& node, LVID& localvar)
	{
		assert(not node.text.empty());
		localvar = node.text.to<uint32_t>();
		assert(localvar != 0);
		return node.children.empty() or scope.visitASTExprContinuation(node, localvar);
	}


	} // anonymous




	bool Scope::visitASTExprIdentifier(AST::Node& node, LVID& localvar)
	{
		// value fetching
		emitDebugpos(node);
		auto& out = sequence();
		// allocate a local variable to receive information about the current identifier
		uint32_t rid = out.emitStackalloc(nextvar(), nyt_any);
		uint32_t offset = out.opcodeCount();
		out.emitIdentify(rid, node.text, localvar);
		// the result of the current expression is now the new allocated local variable
		localvar = rid;
		// promotion to identify:set
		// since it is an assignment (=, +=, ...), only if the current node has children and
		// if another identifier was previously given
		bool shouldPromote = (/*self*/ localvar != 0 and not node.children.empty() and lastIdentifyOpcOffset != 0)
			and (node.text == "=");

		if (not shouldPromote)
		{
			// remember the last opcode offset
			lastIdentifyOpcOffset = offset;
			// children
			return node.children.empty() or visitASTExprContinuation(node, localvar);
		}
		auto& operands = out.at<IR::ISA::Op::identify>(lastIdentifyOpcOffset);
		assert(operands.opcode == static_cast<uint32_t>(IR::ISA::Op::identify));
		if (operands.opcode == static_cast<uint32_t>(IR::ISA::Op::identify))
		{
			operands.opcode = static_cast<uint32_t>(IR::ISA::Op::identifyset);
			lastIdentifyOpcOffset = 0;
		}
		assert(not node.children.empty());
		return visitASTExprContinuation(node, localvar);
	}


	bool Scope::visitASTExprIdOperator(AST::Node& node, LVID& localvar)
	{
		if (unlikely((node.children.size() != 1) or (node.children[0].rule != AST::rgFunctionKindOpname)))
			return unexpectedNode(node, "[expr/operator]");
		emitDebugpos(node);
		auto& out = sequence();
		uint32_t rid = out.emitStackalloc(nextvar(), nyt_any);
		ShortString64 idname;
		idname << '^' << node.children[0].text;
		out.emitIdentify(rid, idname, localvar);
		localvar = rid;
		return true;
	}


	bool Scope::visitASTExprContinuation(AST::Node& node, LVID& localvar, bool allowScope)
	{
		bool success = true;
		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgIdentifier: success &= visitASTExprIdentifier(child, localvar); break;
				case AST::rgExprValue:
				case AST::rgExprGroup:  success &= visitASTExpr(child, localvar); break;
				case AST::rgCall:       success &= visitASTExprCall(&child, localvar, &node); break;
				case AST::rgNumber:     success &= visitASTExprNumber(child, localvar); break;
				case AST::rgNew:        success &= visitASTExprNew(child, localvar); break;
				case AST::rgExprSubDot:
				case AST::rgTypeSubDot: success &= visitASTExprSubDot(child, localvar); break;
				// strings
				case AST::rgString:     success &= visitASTExprString(child, localvar); break;
				// when directly called from an expr, this rule is generated by the compiler itself
				case AST::rgStringLiteral: success &= visitASTExprStringLiteral(child, localvar); break;
				case AST::rgChar:       success &= visitASTExprChar(child, localvar); break;
				// special stuff
				case AST::rgTypeof:     success &= visitASTExprTypeof(child, localvar); break;
				case AST::rgIntrinsic:  success &= visitASTExprIntrinsic(child, localvar); break;
				case AST::rgIf:         success &= visitASTExprIfExpr(child, localvar); break;
				case AST::rgWhile:      success &= visitASTExprWhile(child); break;
				case AST::rgExprTemplate:
				case AST::rgExprTypeTemplate: success &= visitASTExprTemplate(child, localvar); break;
				case AST::rgIn:         success &= visitASTExprIn(child, localvar); break;
				case AST::rgFunction:   success &= visitASTExprClosure(child, localvar); break;
				case AST::rgObject:     success &= visitASTExprObject(child, localvar); break;
				case AST::rgAttributes: success &= visitASTAttributes(child); break;
				case AST::rgFunctionKindOperator: success &= visitASTExprIdOperator(child, localvar); break;
				case AST::rgArray:      success &= visitASTArray(child, localvar); break;
				// scope may appear in expr (when expr are actually statements)
				case AST::rgScope:      success &= visitScope(*this, child, allowScope); break;
				// special for internal AST manipulation
				case AST::rgRegister:   success &= visitASTExprRegister(*this, child, localvar); break;
				// unittest declarations
				case AST::rgUnittest:   success &= visitASTUnitTest(child); break;
				default:
					success = unexpectedNode(child, "[expr/continuation]");
			}
		}
		return success;
	}


	void Scope::emitExprAttributes(uint32_t& localvar)
	{
		assert(!!attributes);
		auto& attrs = *attributes;
		// allow to push a synthetic object (type)
		if (unlikely(attrs.flags(Attributes::Flag::pushSynthetic)))
		{
			// do not report errors
			attrs.flags -= Attributes::Flag::pushSynthetic;
			if (debugmode)
				sequence().emitComment(String("#[__nanyc_synthetic: %") << localvar << ']');
			sequence().emitPragmaSynthetic(localvar, false);
		}
	}


	bool Scope::visitASTExpr(AST::Node& orignode, LVID& localvar, bool allowScope)
	{
		assert(not orignode.children.empty());

		// reset the value of the localvar, result of the expr
		localvar = 0;

		// expr
		// |   expr-value
		AST::Node* nodeptr = &orignode;
		AST::Node* attrnode = nullptr;
		if (orignode.rule == AST::rgExpr)
		{
			switch (orignode.children.size())
			{
				case 1:
				{
					// do not take into consideration this node
					// (it will generate useless scopes)
					auto& firstchild = orignode.children[0];
					if (firstchild.rule == AST::rgExprValue)
						nodeptr = &firstchild;
					break;
				}
				case 2:
				{
					auto& firstchild = orignode.children[0];
					if (firstchild.rule == AST::rgAttributes)
					{
						auto& sndchild = orignode.children[1];
						if (sndchild.rule == AST::rgExprValue)
						{
							nodeptr = &sndchild;
							attrnode = &firstchild; // do not forget to visit this node
						}
					}
					break;
				}
			}
		}

		auto& node = *nodeptr;
		assert(node.rule == AST::rgExpr or node.rule == AST::rgExprValue
			or node.rule == AST::rgExprGroup or node.rule == AST::rgTypeDecl);
		assert(not node.children.empty());

		// always creating a new scope for a expr
		IR::Producer::Scope scope{*this};
		if (unlikely(attrnode))
			scope.visitASTAttributes(*attrnode);

		bool r = scope.visitASTExprContinuation(node, localvar, allowScope);
		if (r and localvar != 0 and localvar != (uint32_t) -1)
		{
			scope.emitTmplParametersIfAny();
			scope.emitDebugpos(node);
			scope.sequence().emitEnsureTypeResolved(localvar);

			if (unlikely(!!scope.attributes))
				scope.emitExprAttributes(localvar);
		}
		// transmit the last identify opcode offset to allow to report propset
		switch (orignode.rule)
		{
			case AST::rgExprValue:
			case AST::rgExprGroup: lastIdentifyOpcOffset = scope.lastIdentifyOpcOffset;
			default: break;
		}
		return r;
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
