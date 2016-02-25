#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	void SequenceBuilder::declareNamedVariable(const AnyString& name, LVID lvid, bool autoreleased)
	{
		auto& lr    = frame->lvids[lvid];
		LVID found  = frame->findLocalVariable(name);

		if (likely(0 == found))
		{
			lr.scope           = frame->scope;
			lr.userDefinedName = name;
			lr.file.line       = currentLine;
			lr.file.offset     = currentOffset;
			lr.file.url        = currentFilename;
			lr.hasBeenUsed     = false;
			lr.scope           = frame->scope;

			CLID clid{frame->atomid, lvid};
			autoreleased   &= frame->verify(lvid); // suppress spurious errors from previous ones
			lr.autorelease  = autoreleased and canBeAcquired(cdeftable.classdef(clid));
		}
		else
		{
			lr.errorReported = true;
			complainRedeclared(name, found);
		}
	}


	// inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::namealias>& operands)
	// see .h




} // namespace Instanciate
} // namespace Pass
} // namespace Nany
