#pragma once
#include "sequence.h"

#define NANY_PRINT_sequence_OPCODES 0
#if NANY_PRINT_sequence_OPCODES != 0
#include <iostream>
#endif



namespace Nany
{
namespace IR
{


	inline void Sequence::reserve(uint32_t N)
	{
		if (pCapacity < N)
			grow(N);
	}

	inline uint32_t Sequence::opcodeCount() const
	{
		return pSize;
	}

	inline uint32_t Sequence::capacity() const
	{
		return pCapacity;
	}


	inline const Instruction& Sequence::at(uint32_t offset) const
	{
		assert(offset < pSize);
		return pBody.get()[offset];
	}


	inline Instruction& Sequence::at(uint32_t offset)
	{
		assert(offset < pSize);
		return pBody.get()[offset];
	}


	template<ISA::Op O> inline ISA::Operand<O>& Sequence::at(uint32_t offset)
	{
		assert(offset < pSize);
		static_assert(sizeof(Instruction) >= sizeof(ISA::Operand<O>), "pSize mismatch");
		return reinterpret_cast<ISA::Operand<O>&>(pBody.get()[offset]);
	}



	template<ISA::Op O> inline const ISA::Operand<O>& Sequence::at(uint32_t offset) const
	{
		assert(offset < pSize);
		static_assert(sizeof(Instruction) >= sizeof(ISA::Operand<O>), "pSize mismatch");
		return reinterpret_cast<ISA::Operand<O>&>(pBody.get()[offset]);
	}


	template<ISA::Op O> inline ISA::Operand<O>& Sequence::emitraw()
	{
		static_assert(sizeof(Instruction) >= sizeof(ISA::Operand<O>), "pSize mismatch");
		assert(pSize + 1 < pCapacity);
		auto& result = at<O>(pSize++);
		result.opcode = static_cast<uint32_t>(O);
		return result;
	}


	template<ISA::Op O> inline ISA::Operand<O>& Sequence::emit()
	{
		reserve(pSize + 1);

		static_assert(sizeof(Instruction) >= sizeof(ISA::Operand<O>), "pSize mismatch");
		assert(pSize + 1 < pCapacity);
		auto& result = at<O>(pSize++);
		result.opcode = static_cast<uint32_t>(O);
		return result;
	}


	inline void Sequence::emitNop()
	{
		emit<ISA::Op::nop>();
	}


	inline void Sequence::emitRef(uint32_t lvid)
	{
		emit<ISA::Op::ref>().lvid = lvid;
	}


	inline uint32_t Sequence::emitAllocate(uint32_t lvid, uint32_t atomid)
	{
		auto& operands = emit<ISA::Op::allocate>();
		operands.lvid = lvid;
		operands.atomid = atomid;
		return lvid;
	}


	inline void Sequence::emitUnref(uint32_t lvid, uint32_t atomid, uint32_t instanceid)
	{
		auto& operands      = emit<ISA::Op::unref>();
		operands.lvid       = lvid;
		operands.atomid     = atomid;
		operands.instanceid = instanceid;
	}


	inline void Sequence::emitFADD(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fadd>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitFSUB(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fsub>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitFDIV(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fdiv>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitFMUL(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fmul>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitADD(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::add>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitSUB(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::sub>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitDIV(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::div>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitMUL(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::mul>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitIMUL(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::imul>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitIDIV(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::idiv>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void Sequence::emitEQ(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::eq>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitNEQ(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::neq>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void Sequence::emitFLT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::flt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitFLTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::flte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitFGT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fgt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitFGTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fgte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void Sequence::emitLT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::lt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitLTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::lte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitILT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::ilt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitILTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::ilte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitGT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::gt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitGTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::gte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitIGT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::igt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitIGTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::igte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void Sequence::emitAND(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::opand>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitOR(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::opor>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitXOR(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::opxor>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitMOD(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::opmod>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void Sequence::emitTypeIsObject(uint32_t lvid)
	{
		emit<ISA::Op::typeisobject>().lvid = lvid;
	}


	inline void Sequence::emitInheritQualifiers(uint32_t lhs, uint32_t rhs)
	{
		auto& operands   = emit<ISA::Op::inherit>();
		operands.inherit = 2;
		operands.lhs     = lhs;
		operands.rhs     = rhs;
	}


	inline void Sequence::emitPragmaSuggest(bool onoff)
	{
		auto& opc = emit<ISA::Op::pragma>();
		opc.pragma = static_cast<uint32_t>(IR::ISA::Pragma::suggest);
		opc.value.suggest = static_cast<uint32_t>(onoff);
	}

	inline void Sequence::emitPragmaBuiltinAlias(const AnyString& name)
	{
		auto& opc = emit<ISA::Op::pragma>();
		opc.pragma = static_cast<uint32_t>(IR::ISA::Pragma::builtinalias);
		opc.value.builtinalias.namesid = stringrefs.ref(name);
	}


	inline void Sequence::emitPragmaShortcircuit(bool evalvalue)
	{
		auto& opc = emit<ISA::Op::pragma>();
		opc.pragma = static_cast<uint32_t>(IR::ISA::Pragma::shortcircuit);
		opc.value.shortcircuit = static_cast<uint32_t>(evalvalue);
	}

	inline void Sequence::emitPragmaShortcircuitMetadata(uint32_t label)
	{
		auto& opc = emit<ISA::Op::pragma>();
		opc.pragma = static_cast<uint32_t>(IR::ISA::Pragma::shortcircuitOpNopOffset);
		opc.value.shortcircuitMetadata.label = label;
	}


	inline void Sequence::emitLabel(uint32_t labelid)
	{
		emit<IR::ISA::Op::label>().label = labelid;
	}


	inline void Sequence::emitPragmaFuncBody()
	{
		emit<ISA::Op::pragma>().pragma = static_cast<uint32_t>(IR::ISA::Pragma::bodystart);
	}


	inline void Sequence::emitPragmaAllowCodeGeneration(bool enabled)
	{
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::codegen;
		operands.value.codegen = static_cast<uint32_t>(enabled);
	}

	inline void Sequence::emitVisibility(nyvisibility_t visibility)
	{
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::visibility;
		operands.value.visibility = static_cast<uint32_t>(visibility);
	}

	inline uint32_t Sequence::emitStackSizeIncrease(uint32_t size)
	{
		uint32_t offset = pSize;
		emit<ISA::Op::stacksize>().add = size;
		return offset;
	}


	inline uint32_t Sequence::emitStackSizeIncrease()
	{
		return emitStackSizeIncrease(0);
	}

	inline uint32_t Sequence::emitBlueprintSize()
	{
		uint32_t offset = pSize;
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::blueprintsize;
		operands.value.blueprintsize = 0;
		return offset;
	}


	inline void Sequence::emitBlueprintClass(const AnyString& name, uint32_t atomid)
	{
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::blueprintclassdef;
		operands.value.blueprint.name   = stringrefs.ref(name);
		operands.value.blueprint.atomid = atomid;
	}

	inline uint32_t Sequence::emitBlueprintClass()
	{
		uint32_t offset = pSize;
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::blueprintclassdef;
		operands.value.blueprint.name   = 0;;
		operands.value.blueprint.atomid = (uint32_t) -1;
		return offset;
	}

	inline void Sequence::emitBlueprintFunc(const AnyString& name, uint32_t atomid)
	{
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::blueprintfuncdef;
		operands.value.blueprint.name   = stringrefs.ref(name);
		operands.value.blueprint.atomid = atomid;
	}

	inline uint32_t Sequence::emitBlueprintFunc()
	{
		uint32_t offset = pSize;
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::blueprintfuncdef;
		operands.value.blueprint.name   = 0;
		operands.value.blueprint.atomid = (uint32_t) -1;
		return offset;
	}

	inline uint32_t Sequence::emitBlueprintParam(LVID lvid, const AnyString& name)
	{
		uint32_t offset = pSize;
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::blueprintparam;
		operands.value.param.name = stringrefs.ref(name);
		operands.value.param.lvid = lvid;
		return offset;
	}

	inline uint32_t Sequence::emitBlueprintParam(LVID lvid)
	{
		uint32_t offset = pSize;
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::blueprintparam;
		operands.value.param.name = 0;
		operands.value.param.lvid = lvid;
		return offset;
	}


	inline void Sequence::emitBlueprintVardef(LVID lvid, const AnyString& name)
	{
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::blueprintvar;
		operands.value.vardef.name = stringrefs.ref(name);
		operands.value.vardef.lvid = lvid;
	}


	inline void Sequence::emitStore_u64(uint32_t lvid, uint64_t value)
	{
		auto& operands = emit<ISA::Op::storeConstant>();
		operands.lvid  = lvid;
		operands.value.u64 = value;
	}

	inline void Sequence::emitStore_f64(uint32_t lvid, double value)
	{
		auto& operands = emit<ISA::Op::storeConstant>();
		operands.lvid  = lvid;
		operands.value.f64 = value;
	}


	inline uint32_t Sequence::emitStoreText(uint32_t lvid, const AnyString& text)
	{
		auto& operands = emit<ISA::Op::storeText>();
		operands.lvid = lvid;
		operands.text = stringrefs.ref(text);
		return operands.text;
	}


	inline uint32_t Sequence::emitStackalloc(uint32_t lvid, nytype_t type)
	{
		auto& operands = emit<ISA::Op::stackalloc>();
		operands.lvid   = lvid;
		operands.type   = static_cast<uint32_t>(type);
		operands.atomid = (uint32_t) -1;
		return lvid;
	}


	inline uint32_t Sequence::emitStackalloc_u64(uint32_t lvid, nytype_t type, uint64_t value)
	{
		emitStackalloc(lvid, type);
		emitStore_u64(lvid, value);
		return lvid;
	}

	inline uint32_t Sequence::emitStackalloc_f64(uint32_t lvid, nytype_t type, double value)
	{
		emitStackalloc(lvid, type);
		emitStore_f64(lvid, value);
		return lvid;
	}


	inline uint32_t Sequence::emitStackallocText(uint32_t lvid, const AnyString& text)
	{
		emitStackalloc(lvid, nyt_pointer);
		emitStoreText(lvid, text);
		return lvid;
	}

	inline void Sequence::emitStore(uint32_t lvid, uint32_t source)
	{
		auto& operands  = emit<ISA::Op::store>();
		operands.lvid   = lvid;
		operands.source = source;
	}


	inline void Sequence::emitNamespace(const AnyString& name)
	{
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::namespacedef;
		operands.value.namespacedef = stringrefs.ref(name);
	}


	inline void Sequence::emitDebugpos(uint32_t line, uint32_t offset)
	{
		auto& operands  = emit<ISA::Op::debugpos>();
		operands.line   = line;
		operands.offset = offset;
	}


	inline void Sequence::emitPush(uint32_t lvid)
	{
		auto& operands = emit<ISA::Op::push>();
		operands.lvid  = lvid;
		operands.name  = 0;
	}


	inline uint32_t Sequence::emitMemalloc(uint32_t lvid, uint32_t regsize)
	{
		auto& operands   = emit<ISA::Op::memalloc>();
		operands.lvid    = lvid;
		operands.regsize = regsize;
		return lvid;
	}

	inline void Sequence::emitMemFree(uint32_t lvid, uint32_t regsize)
	{
		auto& operands   = emit<ISA::Op::memfree>();
		operands.lvid    = lvid;
		operands.regsize = regsize;
	}


	inline void Sequence::emitSizeof(uint32_t lvid, uint32_t type)
	{
		auto& operands = emit<ISA::Op::classdefsizeof>();
		operands.lvid  = lvid;
		operands.type  = type;
	}


	inline void Sequence::emitNameAlias(uint32_t lvid, const AnyString& name)
	{
		auto& operands = emit<ISA::Op::namealias>();
		operands.lvid  = lvid;
		operands.name  = stringrefs.ref(name);
	}


	inline void Sequence::emitReturn(uint32_t lvid)
	{
		emit<ISA::Op::ret>().lvid = lvid;
	}


	inline void Sequence::emitComment(const AnyString& text)
	{
		emit<ISA::Op::comment>().text = stringrefs.ref(text);
	}


	inline void Sequence::emitComment()
	{
		emit<ISA::Op::comment>().text = 0;
	}



	inline void Sequence::emitSelf(uint32_t self)
	{
		emit<ISA::Op::self>().self = self;
	}


	inline void Sequence::emitAssign(uint32_t lhs, uint32_t rhs, bool canDisposeLHS)
	{
		auto& operands = emit<ISA::Op::assign>();
		operands.lhs = lhs;
		operands.rhs = rhs;
		operands.disposelhs = canDisposeLHS;
	}


	inline void Sequence::emitIdentify(uint32_t lvid, const AnyString& name, uint32_t self)
	{
		auto& operands = emit<ISA::Op::identify>();
		operands.lvid  = lvid;
		operands.self  = self;
		operands.text  = stringrefs.ref(name);
	}


	inline void Sequence::emitPush(uint32_t lvid, const AnyString& name)
	{
		assert(not name.empty());
		auto& operands = emit<ISA::Op::push>();
		operands.lvid  = lvid;
		operands.name  = stringrefs.ref(name);
	}


	inline void Sequence::emitCall(uint32_t lvid, uint32_t ptr2func)
	{
		auto& operands = emit<ISA::Op::call>();
		operands.lvid  = lvid;
		operands.ptr2func = ptr2func;
		operands.instanceid = (uint32_t) -1;
	}

	inline void Sequence::emitCall(uint32_t lvid, uint32_t atomid, uint32_t instanceid)
	{
		assert(instanceid != (uint32_t) -1);
		auto& operands = emit<ISA::Op::call>();
		operands.lvid  = lvid;
		operands.ptr2func = atomid;
		operands.instanceid = instanceid;
	}


	inline void Sequence::emitIntrinsic(uint32_t lvid, const AnyString& name, uint32_t id)
	{
		auto& operands     = emit<ISA::Op::intrinsic>();
		operands.lvid      = lvid;
		operands.intrinsic = stringrefs.ref(name);
		operands.iid       = id;
	}


	inline void Sequence::emitFieldget(uint32_t lvid, uint32_t self, uint32_t fieldindex)
	{
		assert(lvid != 0 and self != 0);
		auto& operands = emit<ISA::Op::fieldget>();
		operands.lvid  = lvid;
		operands.self  = self;
		operands.var   = fieldindex;
	}


	inline void Sequence::emitFieldset(uint32_t lvid, uint32_t self, uint32_t varid)
	{
		assert(lvid != 0 and self != 0);
		auto& operands = emit<ISA::Op::fieldset>();
		operands.lvid  = lvid;
		operands.self  = self;
		operands.var   = varid;
	}


	inline void Sequence::emitScope()
	{
		emit<ISA::Op::scope>();
	}


	inline void Sequence::emitEnd()
	{
		emit<ISA::Op::end>();
	}


	inline void Sequence::emitJmp(uint32_t label)
	{
		emit<ISA::Op::jmp>().label = label;
	}

	inline void Sequence::emitJz(uint32_t lvid, uint32_t result, uint32_t label)
	{
		auto& opc  = emit<ISA::Op::jz>();
		opc.lvid   = lvid;
		opc.result = result;
		opc.label  = label;
	}

	inline void Sequence::emitJnz(uint32_t lvid, uint32_t result, uint32_t label)
	{
		auto& opc  = emit<ISA::Op::jnz>();
		opc.lvid   = lvid;
		opc.result = result;
		opc.label  = label;
	}


	inline void Sequence::emitQualifierRef(uint32_t lvid, bool flag)
	{
		auto& operands = emit<ISA::Op::qualifiers>();
		operands.lvid  = lvid;
		operands.flag  = static_cast<uint32_t>(flag);
		operands.qualifier = 1; // ref
	}


	inline void Sequence::emitQualifierConst(uint32_t lvid, bool flag)
	{
		auto& operands = emit<ISA::Op::qualifiers>();
		operands.lvid  = lvid;
		operands.flag  = static_cast<uint32_t>(flag);
		operands.qualifier = 2; // const
	}



	inline const void* Sequence::pointer(uint32_t offset) const
	{
		return reinterpret_cast<const void*>(pBody.get() + offset);
	}


	inline Yuni::String Sequence::gdbMemoryWatch(uint32_t offset) const
	{
		return YString{} << "awatch (char[" << sizeof(Instruction) << "]) *" << pointer(offset);
	}


	inline uint32_t Sequence::offsetOf(const Instruction& instr) const
	{
		assert(pSize > 0 and pCapacity > 0);
		assert(&instr >= pBody.get());
		assert(&instr <  pBody.get() + pSize);

		auto start = reinterpret_cast<std::uintptr_t>(pBody.get());
		auto end   = reinterpret_cast<std::uintptr_t>(&instr);
		assert((end - start) / sizeof(Instruction) < 512 * 1024 * 1024); // arbitrary

		uint32_t r = static_cast<uint32_t>(((end - start) / sizeof(Instruction)));
		assert(r < pSize);
		return r;
	}

	template<ISA::Op O>
	inline uint32_t Sequence::offsetOf(const ISA::Operand<O>& instr) const
	{
		static_assert(sizeof(Instruction) >= sizeof(ISA::Operand<O>), "pSize mismatch");
		return offsetOf(reinterpret_cast<const Instruction&>(instr));
	}


	inline bool Sequence::hasAtomParent() const
	{
		return (nullptr != pAtom);
	}


	inline void Sequence::invalidateCursor(const Instruction*& cursor) const
	{
		cursor = pBody.get() + pSize;
	}

	inline void Sequence::invalidateCursor(Instruction*& cursor) const
	{
		cursor = pBody.get() + pSize;
	}


	inline void Sequence::jumpToLabelForward(const Instruction*& cursor, uint32_t label) const
	{
		auto* end = pBody.get() + pSize;
		while (++cursor < end)
		{
			if (cursor->opcodes[0] == static_cast<uint32_t>(IR::ISA::Op::label))
			{
				auto& operands = reinterpret_cast<const IR::ISA::Operand<IR::ISA::Op::label>&>(*cursor);
				if (operands.label == label)
					return;
			}
		}
		// not found - the cursor is alreayd invalidated
	}


	inline void Sequence::jumpToLabelBackward(const Instruction*& cursor, uint32_t label) const
	{
		auto* base = pBody.get();
		while (cursor-- > base)
		{
			if (cursor->opcodes[0] == static_cast<uint32_t>(IR::ISA::Op::label))
			{
				auto& operands = reinterpret_cast<const IR::ISA::Operand<IR::ISA::Op::label>&>(*cursor);
				if (operands.label == label)
					return;
			}
		}
		// not found - invalidate
		invalidateCursor(cursor);
	}


	template<class T> inline void Sequence::each(T& visitor, uint32_t offset)
	{
		if (likely(offset < pSize))
		{
			auto* it = pBody.get() + offset;
			auto* end = pBody.get() + pSize;
			visitor.cursor = &it;
			for ( ; it < end; ++it)
			{
				#if NANY_PRINT_sequence_OPCODES != 0
				std::cout << "== opcode == at " << (it - pBody.get()) << "|" << (void*) it << " :: "
					<< it->opcodes[0] << ": " << IR::ISA::print(*this, *it) << '\n';
				#endif
				LIBNANY_IR_VISIT_SEQUENCE(IR::ISA::Operand, visitor, *it);
			}
		}
	}


	template<class T> inline void Sequence::each(T& visitor, uint32_t offset) const
	{
		if (likely(offset < pSize))
		{
			const auto* it = pBody.get() + offset;
			const auto* end = pBody.get() + pSize;
			visitor.cursor = &it;
			for ( ; it < end; ++it)
			{
				#if NANY_PRINT_sequence_OPCODES != 0
				std::cout << "== opcode == at " << (it - pBody.get()) << "|" << (void*) it << " :: "
					<< it->opcodes[0] << ": " << IR::ISA::print(*this, *it) << '\n';
				#endif
				LIBNANY_IR_VISIT_SEQUENCE(const IR::ISA::Operand, visitor, *it);
			}
		}
	}






} // namespace IR
} // namespace Nany