#include "instanciate.h"
#include "instanciate-error.h"
#include "type-check.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::commontype>& operands)
	{
		assert(frame != nullptr);

		bool ok = [&]() -> bool
		{
			if (operands.previous == 0)
				return true;
			auto atomid = frame->atomid;
			auto& from  = cdeftable.classdef(CLID{atomid, operands.previous});
			auto& to    = cdeftable.classdef(CLID{atomid, operands.lvid});

			// TODO currently types must be strictly identical
			auto match = TypeCheck::isSimilarTo(*this, from, to);
			switch (match)
			{
				case TypeCheck::Match::strictEqual:
					return true;
				case TypeCheck::Match::equal:
				case TypeCheck::Match::none:
					return complain::typesDoNotMatch(from, to);
			}
			return false;
		}();

		if (unlikely(not ok))
			frame->invalidate(operands.lvid);
	}




} // namespace Instanciate
} // namespace Pass
} // namespace Nany
