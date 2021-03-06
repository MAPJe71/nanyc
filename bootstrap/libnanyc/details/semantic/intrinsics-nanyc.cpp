#include "semantic-analysis.h"
#include "deprecated-error.h"
#include "type-check.h"
#ifdef YUNI_OS_UNIX
#include <unistd.h>
#endif
#include "details/ir/emit.h"
#include "ref-unref.h"

using namespace Yuni;


namespace ny {
namespace semantic {
namespace intrinsic {

namespace {


bool intrinsicReinterpret(Analyzer& seq, uint32_t lvid) {
	assert(seq.pushedparams.func.indexed.size() == 2);
	uint32_t lhs    = seq.pushedparams.func.indexed[0].lvid;
	uint32_t tolvid = seq.pushedparams.func.indexed[1].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, tolvid});
	// copy the type, without any check
	auto& spare = seq.cdeftable.substitute(lvid);
	spare.import(cdef);
	spare.qualifiers = cdef.qualifiers;
	if (seq.canGenerateCode())
		ir::emit::copy(seq.out, lvid, lhs);
	auto& lvidinfo = seq.frame->lvids(lvid);
	lvidinfo.synthetic = false;
	if (canBeAcquired(seq, cdef) and seq.canGenerateCode()) { // re-acquire the object
		ir::emit::ref(seq.out, lvid);
		lvidinfo.autorelease = true;
		lvidinfo.scope = seq.frame->scope;
	}
	return true;
}


bool intrinsicMemcheckerHold(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToVoid();
	uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdefptr = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
	if (not cdefptr.isRawPointer())
		return seq.complainIntrinsicParameter("__nanyc_memchecker_hold", 0, cdefptr, "'__pointer'");
	uint32_t sizelvid = seq.pushedparams.func.indexed[1].lvid;
	auto& cdefsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, sizelvid});
	if (not cdefsize.isBuiltinU64())
		return seq.complainIntrinsicParameter("__nanyc_memchecker_hold", 0, cdefsize, "'__u64'");
	if (seq.canGenerateCode())
		ir::emit::memory::hold(seq.out, ptrlvid, sizelvid);
	return true;
}


using BuiltinIntrinsic = bool (*)(Analyzer&, uint32_t);

static const std::unordered_map<AnyString, std::pair<uint32_t, BuiltinIntrinsic>> builtinDispatch = {
	{"__reinterpret",            { 2,  &intrinsicReinterpret, }},
	{"__nanyc_memchecker_hold",  { 2,  &intrinsicMemcheckerHold, }},
};


} // anonymous namespace


Tribool::Value nanycSpecifics(Analyzer& analyzer, const AnyString& name, uint32_t lvid, bool produceError) {
	assert(not name.empty());
	assert(analyzer.frame != nullptr);
	auto it = builtinDispatch.find(name);
	if (unlikely(it == builtinDispatch.end())) {
		if (produceError)
			complain::unknownIntrinsic(name);
		return Tribool::Value::indeterminate;
	}
	// checking for parameters
	uint32_t count = it->second.first;
	if (unlikely(not analyzer.checkForIntrinsicParamCount(name, count)))
		return Tribool::Value::no;
	analyzer.frame->lvids(lvid).synthetic = false;
	// intrinsic builtin found !
	return ((it->second.second))(analyzer, lvid) ? Tribool::Value::yes : Tribool::Value::no;
}


} // namespace intrinsic
} // namespace semantic
} // namespace ny
