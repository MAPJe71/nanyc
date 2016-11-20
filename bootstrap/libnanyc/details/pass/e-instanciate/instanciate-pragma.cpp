#include "instanciate.h"

using namespace Yuni;




namespace ny
{
namespace Pass
{
namespace Instanciate
{


	namespace {


	bool pragmaBodyStart(SequenceBuilder& seq)
	{
		assert(seq.frame != nullptr);
		assert(not seq.signatureOnly);
		assert(seq.codeGenerationLock == 0 and "any good reason to not generate code ?");
		assert(seq.out != nullptr and "no output IR sequence");

		auto& frame = *seq.frame;
		auto& atom = frame.atom;
		if (not atom.isFunction())
			return true;

		bool generateCode = seq.canGenerateCode();
		uint32_t count = atom.parameters.size();
		bool success = true;

		atom.parameters.each([&](uint32_t i, const AnyString& name, const Vardef& vardef)
		{
			// lvid for the given parameter
			uint32_t lvid  = i + 1 + 1; // 1: return type, 2: first parameter
			// the parameters are real objects
			frame.lvids(lvid).synthetic = false;
			// Parameters Deep copy (if required)
			auto& cdef = seq.cdeftable.classdef(vardef.clid);
			if (name[0] != '^')
			{
				// normal input parameter (not captured - does not start with '^')
				// clone it if necessary (only non-ref parameters)
				if (not cdef.qualifiers.ref)
				{
					if (debugmode and generateCode)
						seq.out->emitComment(String{"----- deep copy parameter "} << i << " aka " << name);
					// a register has already been reserved for cloning parameters
					uint32_t clone = 2 + count + i; // 1: return type, 2: first parameter
					// the new value is not synthetic
					frame.lvids(clone).synthetic = false;
					bool r = seq.instanciateAssignment(frame, clone, lvid, false, false, true);
					if (unlikely(not r))
						frame.invalidate(clone);
					if (seq.canBeAcquired(lvid))
					{
						frame.lvids(lvid).autorelease = true;
						frame.lvids(clone).autorelease = false;
					}
					if (generateCode)
					{
						seq.out->emitStore(lvid, clone); // register swap
						if (debugmode)
							seq.out->emitComment("--\n");
					}
				}
			}
			if (not cdef.isBuiltinOrVoid())
			{
				auto* paramatom = seq.cdeftable.findClassdefAtom(cdef);
				if (unlikely(!paramatom))
				{
					frame.invalidate(lvid);
					ice() << "invalid parameter type " << i << " for " << atom.caption(seq.cdeftable);
					success = false;
				}
			}
		});
		// generating some code on the fly
		if (atom.isSpecial() /*ctor, operators...*/ and generateCode and success)
		{
			if (seq.generateClassVarsAutoInit) // var init (called by ctor)
			{
				seq.generateClassVarsAutoInit = false;
				seq.generateMemberVarDefaultInitialization();
			}
			if (seq.generateClassVarsAutoRelease) // var release (called by dtor)
			{
				seq.generateClassVarsAutoRelease = false;
				seq.generateMemberVarDefaultDispose();
			}
			if (seq.generateClassVarsAutoClone) // var deep copy (copy ctor)
			{
				seq.generateClassVarsAutoClone = false;
				seq.generateMemberVarDefaultClone();
			}
		}
		return success;
	}


	void pragmaCodegen(SequenceBuilder& seq, bool onoff)
	{
		auto& refcount = seq.codeGenerationLock;
		if (onoff)
		{
			if (refcount > 0) // re-enable code generation
				--refcount;
		}
		else
			++refcount;
	}


	void pragmaBlueprintSize(SequenceBuilder& seq, uint32_t opcodeCount)
	{
		if (0 == seq.layerDepthLimit)
		{
			// ignore the current blueprint
			//*cursor += operands.value.blueprintsize;
			assert(seq.frame != nullptr);
			assert(seq.frame->offsetOpcodeBlueprint != (uint32_t) -1);
			auto startOffset = seq.frame->offsetOpcodeBlueprint;
			if (unlikely(opcodeCount < 3))
				return (void) (ice() << "invalid blueprint size when instanciating atom");

			// goto the end of the blueprint
			// -2: the final 'end' opcode must be interpreted
			*seq.cursor = &seq.currentSequence.at(startOffset + opcodeCount - 1 - 1);
		}
	}


	void pragmaShortcircuitMutateToBool(SequenceBuilder& seq, const ir::ISA::Operand<ir::ISA::Op::pragma>& operands)
	{
		uint32_t lvid = operands.value.shortcircuitMutate.lvid;
		uint32_t source = operands.value.shortcircuitMutate.source;

		seq.frame->lvids(lvid).synthetic = false;
		if (true)
		{
			auto& instr = *(*seq.cursor - 1);
			assert(instr.opcodes[0] == static_cast<uint32_t>(ir::ISA::Op::stackalloc));
			uint32_t sizeoflvid = instr.to<ir::ISA::Op::stackalloc>().lvid;

			// sizeof
			auto& atombool = *(seq.cdeftable.atoms().core.object[nyt_bool]);
			seq.out->emitSizeof(sizeoflvid, atombool.atomid);

			auto& opc = seq.cdeftable.substitute(lvid);
			opc.mutateToAtom(&atombool);
			opc.qualifiers.ref = true;
			// ALLOC: memory allocation of the new temporary object
			seq.out->emitMemalloc(lvid, sizeoflvid);
			seq.out->emitRef(lvid);
			seq.frame->lvids(lvid).autorelease = true;
			// reset the internal value of the object
			seq.out->emitFieldset(source, /*self*/lvid, 0); // builtin
		}
		else
			seq.out->emitStore(lvid, source);
	}


	} // anonymous namespace




	void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::pragma>& operands)
	{
		switch (operands.pragma)
		{
			case ir::ISA::Pragma::codegen:
			{
				pragmaCodegen(*this, operands.value.codegen != 0);
				break;
			}
			case ir::ISA::Pragma::bodystart:
			{
				// In 'signature only' mode, we only care about the
				// parameter user types. Everything after this opcode is unrelevant
				bool r = (!signatureOnly and pragmaBodyStart(*this)); // params deep copy, implicit var auto-init...
				if (not r)
					currentSequence.invalidateCursor(*cursor);
				break;
			}
			case ir::ISA::Pragma::blueprintsize:
			{
				pragmaBlueprintSize(*this, operands.value.blueprintsize);
				break;
			}
			case ir::ISA::Pragma::visibility:
			{
				assert(frame != nullptr);
				break;
			}
			case ir::ISA::Pragma::shortcircuitOpNopOffset:
			{
				shortcircuit.label = operands.value.shortcircuitMetadata.label;
				break;
			}
			case ir::ISA::Pragma::shortcircuitMutateToBool:
			{
				pragmaShortcircuitMutateToBool(*this, operands);
				break;
			}
			case ir::ISA::Pragma::synthetic:
			{
				uint32_t lvid = operands.value.synthetic.lvid;
				bool onoff = (operands.value.synthetic.onoff != 0);
				frame->lvids(lvid).synthetic = onoff;
				break;
			}
			case ir::ISA::Pragma::suggest:
			case ir::ISA::Pragma::builtinalias:
			case ir::ISA::Pragma::shortcircuit:
			case ir::ISA::Pragma::unknown:
				break;
		}
	}




} // namespace Instanciate
} // namespace Pass
} // namespace ny
