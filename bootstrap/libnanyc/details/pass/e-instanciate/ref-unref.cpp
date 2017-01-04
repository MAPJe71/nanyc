#include "instanciate.h"
#include "details/ir/emit.h"
#include "details/pass/e-instanciate/ref-unref.h"

using namespace Yuni;


namespace ny {
namespace Pass {
namespace Instanciate {


void SequenceBuilder::visit(const ir::isa::Operand<ir::isa::Op::ref>& operands) {
	if (not frame->verify(operands.lvid))
		return;
	if (unlikely(frame->lvids(operands.lvid).synthetic)) {
		error() << "cannot acquire a synthetic object";
		return;
	}
	if (canGenerateCode()) {
		if (canBeAcquired(*this, operands.lvid))
			ir::emit::ref(out, operands.lvid); // manual var acquisition
	}
}


void SequenceBuilder::visit(const ir::isa::Operand<ir::isa::Op::unref>& operands) {
	tryUnrefObject(*this, operands.lvid);
}


} // namespace Instanciate
} // namespace Pass
} // namespace ny