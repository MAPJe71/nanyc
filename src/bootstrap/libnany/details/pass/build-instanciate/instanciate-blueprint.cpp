#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	inline void SequenceBuilder::adjustSettingsNewFuncdefOperator(const AnyString& name)
	{
		assert(name.size() >= 2);
		switch (name[1])
		{
			case 'd':
			{
				if (name == "^default-new")
				{
					generateClassVarsAutoInit = true; // same as '^new'
					break;
				}
				break;
			}

			case 'n':
			{
				if (name == "^new")
				{
					generateClassVarsAutoInit = true; // same as '^default-new'
					break;
				}
				break;
			}

			case 'o':
			{
				if (name == "^obj-dispose")
				{
					generateClassVarsAutoRelease = true;
					break;
				}
				if (name == "^obj-clone")
				{
					generateClassVarsAutoClone = true;
					break;
				}
				break;
			}
		}
	}



	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
	{
		auto kind = static_cast<IR::ISA::Blueprint>(operands.kind);
		switch (kind)
		{
			case IR::ISA::Blueprint::funcdef:
			case IR::ISA::Blueprint::classdef:
			{
				// reset
				pushedparams.clear();
				generateClassVarsAutoInit = false;
				generateClassVarsAutoRelease = false;
				lastOpcodeStacksizeOffset = (uint32_t) -1;

				assert(layerDepthLimit > 0);
				--layerDepthLimit;

				uint32_t atomid = operands.atomid;
				AnyString atomname = currentSequence.stringrefs[operands.name];

				if (frame != nullptr and canGenerateCode())
				{
					if (kind == IR::ISA::Blueprint::funcdef)
						out.emitBlueprintFunc(atomname, atomid);
					else
						out.emitBlueprintClass(atomname, atomid);
				}

				auto* atom = cdeftable.atoms().findAtom(atomid);
				if (unlikely(nullptr == atom))
				{
					complainOperand(IR::Instruction::fromOpcode(operands), "invalid atom");
					break;
				}

				// create new frame
				pushNewFrame(*atom);
				frame->blueprintOpcodeOffset = currentSequence.offsetOf(**cursor);

				if (kind == IR::ISA::Blueprint::funcdef)
				{
					if (atomname[0] == '^') // operator !
					{
						// some special actions must be triggered according the operator name
						adjustSettingsNewFuncdefOperator(atomname);
					}
				}
				break;
			}

			case IR::ISA::Blueprint::param: // -- function parameter
			{
				uint32_t sid  = operands.name;
				uint32_t lvid = operands.lvid;
				auto& cdef = cdeftable.substitute(lvid);
				cdef.qualifiers.ref = false;
				cdef.instance = true;
				declareNamedVariable(currentSequence.stringrefs[sid], lvid, false);
				break;
			}
			case IR::ISA::Blueprint::gentypeparam: // -- template parameter
			{
				uint32_t sid  = operands.name;
				uint32_t lvid = operands.lvid;
				auto& cdef = cdeftable.substitute(lvid);
				cdef.qualifiers.ref = false;
				cdef.instance = false;
				declareNamedVariable(currentSequence.stringrefs[sid], lvid, false);
				break;
			}
			case IR::ISA::Blueprint::paramself: // -- function parameter
			{
				// -- with automatic variable assignment for operator new
				// example: operator new (self varname) {}
				assert(frame != nullptr);
				if (unlikely(not frame->atom.isClassMember()))
				{
					error() << "automatic variable assignment is only allowed in class operator 'new'";
					break;
				}

				if (!frame->selfParameters.get())
					frame->selfParameters = std::make_unique<decltype(frame->selfParameters)::element_type>();

				uint32_t sid  = operands.name;
				uint32_t lvid = operands.lvid;
				AnyString varname = currentSequence.stringrefs[sid];
				(*frame->selfParameters)[varname].first = lvid;
				break;
			}

			case IR::ISA::Blueprint::vardef:
			{
				// update the type of the variable member
				if (frame != nullptr)
				{
					if (frame->atom.isClass())
					{
						uint32_t sid  = operands.name;
						const AnyString& varname = currentSequence.stringrefs[sid];

						Atom* atom = nullptr;
						if (1 != frame->atom.findVarAtom(atom, varname))
						{
							ICE() << "unknown variable member '" << varname << "'";
							break;
						}
						atom->returnType.clid.reclass(frame->atomid, operands.lvid);
					}
				}
				pushedparams.clear();
				break;
			}

			case IR::ISA::Blueprint::typealias:
			{
				pushedparams.clear();
				break;
			}

			case IR::ISA::Blueprint::unit:
			{
				// reset
				pushedparams.clear();
				generateClassVarsAutoInit = false;
				generateClassVarsAutoRelease = false;

				uint32_t atomid = operands.atomid;
				auto* atom = cdeftable.atoms().findAtom(atomid);
				if (unlikely(nullptr == atom))
				{
					complainOperand(IR::Instruction::fromOpcode(operands), "invalid unit atom");
					break;
				}

				pushNewFrame(*atom);
				frame->blueprintOpcodeOffset = currentSequence.offsetOf(**cursor);
				break;
			}

			case IR::ISA::Blueprint::namespacedef:
				break;
		}
	}





} // namespace Instanciate
} // namespace Pass
} // namespace Nany
