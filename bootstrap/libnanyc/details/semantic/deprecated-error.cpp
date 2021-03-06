#include "semantic-analysis.h"
#include "overloaded-func-call-resolution.h"
#include "details/errors/errors.h"

using namespace Yuni;


namespace ny {
namespace semantic {
namespace complain {


bool classdef(const Classdef& cdef, const char* usertxt) {
	auto entry = (ice() << usertxt << ": " << cdef.clid << ' ');
	auto* seq = Logs::userHandler<Analyzer>();
	if (seq)
		cdef.print(entry, seq->cdeftable);
	return false;
}


bool unknownIntrinsic(const AnyString& name) {
	error() << "unknown intrinsic '!!" << name << '\'';
	return false;
}


bool classNotInstanciated(const Atom& atom) {
	auto err = ice();
	auto* seq = Logs::userHandler<Analyzer>();
	if (seq)
		err << "class not instanciated: " << atom.caption(seq->cdeftable);
	if (debugmode)
		err << ", atomid: " << atom.atomid;
	return false;
}


bool classOrFuncExpected(const Classdef& cdef) {
	auto e = (error() << "class or function expected, got '");
	auto* seq = Logs::userHandler<Analyzer>();
	if (seq)
		cdef.print(e.data().message, seq->cdeftable, false);
	e << "' instead";
	return false;
}


bool classRequired() {
	error() << "type class required";
	return false;
}


bool canNotAllocateClassNullAtom(const Classdef& cdef, uint32_t lvid) {
	auto* seq = Logs::userHandler<Analyzer>();
	auto err = (error() << "cannot instanciate object of type '");
	if (seq)
		cdef.print(err.data().message, seq->cdeftable, false);
	else
		err << "<invalid sequence builder>";
	err << '\'';
	if (debugmode)
		err << " (null atom, probably due to previous error)";
	if (seq and seq->frame)
		seq->frame->invalidate(lvid);
	return false;
}


bool invalidClassSelf(const AnyString& identifier) {
	auto entry = ice();
	entry << "identify: invalid 'self' object for '" << identifier;
	entry << "' from '";
	auto* seq = Logs::userHandler<Analyzer>();
	if (seq and seq->frame)
		entry << seq->frame->atom.caption();
	else
		entry << "<invalid sequence builder/frame>";
	entry << "' (class expected)";
	return false;
}


bool noproperty(const AnyString& identifier) {
	error() << "no property found for '" << identifier << '\'';
	return false;
}


bool ambigousPropertyCall(const AnyString& identifier) {
	error() << "ambigous property call for '" << identifier << '\'';
	return false;
}


bool invalidTypedef(const Classdef& cdef) {
	auto entry = (error() << "invalid typedef definition");
	if (debugmode)
		entry << " (got null atom, " << cdef.clid << ')';
	return false;
}


bool returnTypeMismatch(const Classdef& expected, const Classdef& usertype) {
	auto err = (error() << "type mismatch in 'return' statement");
	auto* seq = Logs::userHandler<Analyzer>();
	if (seq) {
		auto hint = err.hint();
		if (debugmode) {
			usertype.print((hint << "got '%" << usertype.clid << ':'), seq->cdeftable);
			expected.print((hint << "', expected '%" << expected.clid << ':'), seq->cdeftable);
		}
		else {
			usertype.print((hint << "got '"), seq->cdeftable);
			expected.print((hint << "', expected '"), seq->cdeftable);
		}
		hint << '\'';
	}
	return false;
}


bool returnTypeImplicitConversion(const Classdef& expected, const Classdef& usertype, uint32_t line,
								  uint32_t offset) {
	auto err = (error() << "implicit conversion not supported in 'return'");
	auto* seq = Logs::userHandler<Analyzer>();
	if (seq) {
		auto hint = err.hint();
		if (debugmode) {
			usertype.print((hint << "got '%" << usertype.clid << ':'), seq->cdeftable);
			expected.print((hint << "', expected '%" << expected.clid << ':'), seq->cdeftable);
		}
		else {
			usertype.print((hint << "got '"), seq->cdeftable);
			expected.print((hint << "', expected '"), seq->cdeftable);
		}
		hint << '\'';
		if (line != 0)
			hint.origins().location.pos.line   = line;
		if (offset != 0)
			hint.origins().location.pos.offset = offset;
	}
	return false;
}


bool returnTypeMissing(const Classdef* expected, const Classdef* usertype) {
	if (expected) {
		String tstr;
		auto* seq = Logs::userHandler<Analyzer>();
		if (seq)
			expected->print(tstr, seq->cdeftable);
		error() << "return-statement with no value, in function returning '" << tstr << '\'';
	}
	else if (usertype)
		error() << "return-statement with a value, in function returning 'void'";
	else
		ice() << "mismatch return";
	return false;
}


bool returnMultipleTypes(const Classdef& expected, const Classdef& usertype, uint32_t line, uint32_t offset) {
	auto err = (error() << "multiple incompatible types for 'return'");
	auto* seq = Logs::userHandler<Analyzer>();
	if (seq) {
		auto hint = err.hint();
		if (debugmode) {
			usertype.print((hint << "got '%" << usertype.clid << ':'), seq->cdeftable);
			expected.print((hint << "', expected '%" << expected.clid << ':'), seq->cdeftable);
		}
		else {
			usertype.print((hint << "got '"), seq->cdeftable);
			expected.print((hint << "', expected '"), seq->cdeftable);
		}
		hint << '\'';
		if (line != 0)
			hint.origins().location.pos.line   = line;
		if (offset != 0)
			hint.origins().location.pos.offset = offset;
	}
	return false;
}


bool typedefCircularReference(const Atom& original, const Atom& responsible) {
	auto err = (error() << "cannot resolve type alias '");
	auto* seq = Logs::userHandler<Analyzer>();
	if (seq)
		original.retrieveCaption(err.data().message, seq->cdeftable);
	err << "': circular dependcy";
	if (responsible.atomid != original.atomid and seq) {
		auto hint = (err.hint() << "when trying to resolve '");
		responsible.retrieveCaption(hint.data().message, seq->cdeftable);
		hint << '\'';
	}
	return false;
}


bool typedefNotResolved(const Atom& original) {
	auto err = (error() << "cannot resolve type alias '");
	auto* seq = Logs::userHandler<Analyzer>();
	if (seq)
		original.retrieveCaption(err.data().message, seq->cdeftable);
	err << '\'';
	return false;
}


bool typedefRefDeclaredAfter(const Atom& original, const Atom& responsible) {
	auto err = (error() << "cannot resolve type alias");
	auto* seq = Logs::userHandler<Analyzer>();
	if (seq) {
		err << " '";
		original.retrieveCaption(err.data().message, seq->cdeftable);
		err << "': type alias '";
		responsible.retrieveCaption(err.data().message, seq->cdeftable);
		err << "' not resolved yet";
		auto hint = (err.hint() << "when trying to resolve '");
		responsible.retrieveCaption(hint.data().message, seq->cdeftable);
		hint << '\'';
	}
	return false;
}


bool multipleDefinitions(const Atom& atom, const AnyString& deffor) {
	auto err = (error() << atom.keyword() << ' ');
	atom.retrieveFullname(err.data().message);
	err << ": multiple definition for " << deffor;
	return false;
}


bool redeclared(const AnyString& name, uint32_t previousDeclaration) {
	auto err = (error() << "redeclaration of '" << name << '\'');
	err << " in '";
	auto* seq = Logs::userHandler<Analyzer>();
	if (seq) {
		err << seq->cdeftable.keyword(seq->frame->atom) << ' ';
		seq->frame->atom.retrieveCaption(err.data().message, seq->cdeftable);
	}
	err << '\'';
	auto suggest = (err.hint() << "previous declaration of '" << name << '\'');
	if (seq) {
		auto& lr = seq->frame->lvids(previousDeclaration);
		lr.fillLogEntryWithLocation(suggest);
	}
	return false;
}


bool multipleOverloads(uint32_t lvid) {
	auto* seq = Logs::userHandler<Analyzer>();
	auto* frame = seq->frame;
	assert(frame != nullptr);
	auto err = (error() << "ambigous resolution");
	if (not frame->lvids(lvid).resolvedName.empty())
		err << " for '" << frame->lvids(lvid).resolvedName << "'";
	CLID clid{frame->atomid, lvid};
	if (debugmode)
		err << ' ' << clid;
	auto it = frame->partiallyResolved.find(clid);
	if (it != frame->partiallyResolved.end()) {
		auto& solutions = it->second;
		for (size_t i = 0; i != solutions.size(); ++i) {
			auto& atom = solutions[i].get();
			auto hint = err.hint();
			hint.message.origins.location.resetFromAtom(atom);
			if (seq) {
				hint << '\'';
				hint << seq->cdeftable.keyword(atom) << ' ';
				hint << atom.caption(seq->cdeftable);
				hint << "' is a suitable candidate";
				if (debugmode)
					hint << " {atomid: " << atom.atomid << '}';
			}
		}
	}
	return false;
}


bool multipleOverloads(uint32_t lvid, const std::vector<std::reference_wrapper<Atom>>& solutions
		, OverloadedFuncCallResolver& resolver) {
	assert(solutions.size() == resolver.suitable.size());
	auto* seq = Logs::userHandler<Analyzer>();
	auto* frame = seq->frame;
	String varname;
	for (auto l = frame->lvids(lvid).referer; l != 0; l = frame->lvids(l).referer) {
		auto& part = frame->lvids(l).resolvedName;
		if (not part.empty()) {
			AnyString keyword;
			AnyString partname;
			Atom::extractNames(keyword, partname, part);
			String str;
			if (not keyword.empty())
				str << keyword << ' ';
			str << partname;
			if (not varname.empty())
				str << '.';
			varname.prepend(str);
		}
	}
	auto err = error();
	if (resolver.suitableCount > 0) {
		// generating the error
		err << "ambiguous call to overloaded function '" << varname;
		seq->overloadMatch.printInputParameters(err.data().message);
		err << '\'';
		for (size_t i = 0; i != solutions.size(); ++i) {
			if (resolver.suitable[i]) {
				auto& atom = solutions[i].get();
				auto hint = err.hint();
				hint.message.origins.location.resetFromAtom(atom);
				hint << '\'';
				hint << seq->cdeftable.keyword(atom) << ' ';
				hint << atom.caption(seq->cdeftable);
				hint << "' is a suitable candidate";
				if (debugmode)
					hint << " {b-atomid: " << atom.atomid << '}';
				hint.appendEntry(resolver.subreports[i]); // subreport can be null
			}
		}
	}
	else {
		err << "no matching function to call '" << varname;
		seq->overloadMatch.printInputParameters(err.data().message);
		err << '\'';
		for (size_t i = 0; i != solutions.size(); ++i) {
			auto& atom = solutions[i].get();
			//if (atom.flags(Atom::Flags::suggestInReport))
			{
				auto hint  = (err.hint() << "see ");
				hint.message.origins.location.resetFromAtom(atom);
				hint << '\'';
				hint << seq->cdeftable.keyword(atom) << ' ';
				atom.retrieveCaption(hint.text(), seq->cdeftable);
				hint << '\'';
				hint.appendEntry(resolver.subreports[i]); // subreport can be null
			}
		}
	}
	return false;
}


bool typesDoNotMatch(const Classdef& from, const Classdef& to) {
	auto* seq = Logs::userHandler<Analyzer>();
	auto err = error();
	err << "type '";
	if (seq)
		from.print(err, seq->cdeftable);
	err << "' do not match with '";
	if (seq)
		to.print(err, seq->cdeftable);
	err << '\'';
	return false;
}


bool selfMissingForPropertyCall(const Atom& property, uint32_t self) {
	auto e = ice();
	e << "invalid 'self' for calling property '" << property.caption() << '\'';
	if (debugmode) {
		e << " [self: ";
		if (self != (uint32_t) - 1)
			e << self;
		else
			e << "(uint32_t) -1";
		e << ']';
	}
	return false;
}


bool parameterTypeHasVanished(const Analyzer& seq, uint32_t i) {
	auto& frame = *seq.frame;
	auto e = ice();
	e << "type has vanished for parameter " << i;
	e << " for " << frame.atom.caption(seq.cdeftable);
	return false;
}


} // namespace complain


Logs::Report emitReportEntry(void* self, Logs::Level level) {
	auto& sb = *(reinterpret_cast<Analyzer*>(self));
	switch (level) {
		default:
			break;
		// [[fallthru]]
		case Logs::Level::error:
		case Logs::Level::ICE: {
			sb.success = false;
			break;
		}
	}
	auto entry = sb.report.fromErrLevel(level);
	if (debugmode) {
		if (level == Logs::Level::error or level == Logs::Level::ICE) {
			if (sb.cursor and sb.currentSequence.isCursorValid(**sb.cursor)) {
				uint32_t offset = sb.currentSequence.offsetOf(**sb.cursor);
				auto h = entry.hint();
				h << "dump opcodes at +" << offset << " [from Analyzer ";
				h << self;
				if (sb.signatureOnly)
					h << ", signature only";
				h << "]\n";
				auto* map = &(sb.cdeftable.originalTable().atoms);
				ir::isa::printExtract(h.message.message, sb.currentSequence, offset, map);
			}
		}
	}
	return entry;
}


void retriveReportMetadata(void* self, Logs::Level, const AST::Node*, String& filename, uint32_t& line,
						   uint32_t& offset) {
	auto& sb = *(reinterpret_cast<Analyzer*>(self));
	filename = sb.currentFilename;
	line     = sb.currentLine;
	offset   = sb.currentOffset;
}


bool Analyzer::complainBuiltinIntrinsicDoesNotAccept(const AnyString& name, const AnyString& what) {
	error() << "builtin intrinsic '" << name << "' does not accept " << what;
	return false;
}


bool Analyzer::complainIntrinsicWithNamedParameters(const AnyString& name) {
	assert(not pushedparams.func.named.empty() and "Uh ?");
	auto err = (error());
	err << "piko: intrinsic '" << name << "': named parameters are not allowed";
	if (debugmode) {
		for (auto nmparm : pushedparams.func.named)
			err.hint() << "parameter name '" << nmparm.name << '\'';
	}
	return false;
}


bool Analyzer::complainIntrinsicWithGenTypeParameters(const AnyString& name) {
	error() << "intrinsic '" << name << "': generic type parameters are not allowed";
	return false;
}


bool Analyzer::complainIntrinsicParameterCount(const AnyString& name, uint32_t count) {
	uint32_t c = static_cast<uint32_t>(pushedparams.func.indexed.size());
	assert(c != count);
	auto err = (error() << "intrinsic '" << name << "': ");
	if (unlikely(c > count))
		err << "too many parameters";
	else
		err << "not enough parameters";
	err << " (got " << c << " instead of " << count << ')';
	return false;
}


bool Analyzer::complainIntrinsicParameter(const AnyString& name, uint32_t pindex, const Classdef& got,
		const AnyString& expected) {
	success = false;
	auto& element = pushedparams.func.indexed[pindex];
	auto err = (error() << "intrinsic '" << name << "': invalid type for parameter " <<
				(1 + pindex) << ", got '");
	got.print(err, cdeftable);
	err << "', expected ";
	if (not expected.empty())
		err << expected;
	else
		err << "non-void builtin (__bool, __i32, ...)";
	err.message.origins.location.filename   = currentFilename;
	err.message.origins.location.pos.line   = element.line;
	err.message.origins.location.pos.offset = element.offset;
	uint32_t index = 0;
	for (auto& p : pushedparams.func.indexed) {
		auto& cdef = cdeftable.classdef(CLID{frame->atomid, p.lvid});
		auto h = (err.hint() << "received pushed parameter " << (++index) << " as '");
		cdef.print(h, cdeftable);
		h << '\'';
	}
	return false;
}


bool Analyzer::complainOperand(const ir::Instruction& operands, AnyString msg) {
	success = false;
	auto message = ice();
	// example: ice: unexpected opcode 'resolveAttribute': from 'ref %4 = resolve %3."()"'
	if (not msg.empty())
		message << msg;
	else
		message << "unexpected opcode";
	message << ": '" << ir::isa::print(currentSequence, operands) << '\'';
	// stop reading the opcodes
	currentSequence.invalidateCursor(*cursor);
	return false;
}


void Analyzer::complainUnusedVariable(const AtomStackFrame& frame, uint32_t lvid) const {
	auto& lvidinfo = frame.lvids(lvid);
	if (not lvidinfo.errorReported) {
		auto err = warning();
		lvidinfo.fillLogEntryWithLocation(err);
		err << "unused variable '" << lvidinfo.userDefinedName << "' in '";
		err << cdeftable.keyword(frame.atom) << ' ';
		err << frame.atom.caption(cdeftable);
		err << '\'';
		if (debugmode)
			err << " {atomid: " << frame.atomid << '}';
	}
}


bool Analyzer::complainInvalidMemberRequestNonClass(const AnyString& name, CType kind) const {
	assert(not name.empty());
	success = false;
	auto e = (error() << "request member '");
	if (name.first() == '^')
		e << "operator " << AnyString{name, 1, name.size() - 1};
	else
		e << name;
	e << "' on a non-class type '" << ny::toString(kind) << '\'';
	return false;
}


bool Analyzer::complainUnknownBuiltinType(const AnyString& name) const {
	error() << "unknown builtin type '" << name << '\'';
	return false;
}


bool Analyzer::complainInvalidSelfRefForVariableAssignment(uint32_t lvid) const {
	assert(frame != nullptr);
	auto& cdef = cdeftable.classdef(CLID{frame->atomid, lvid});
	auto ce = (ice() << "invalid 'self' reference for variable assignment (self.= <expr>) ");
	cdef.print(ce.data().message, cdeftable, false);
	return false;
}


bool Analyzer::complainMissingOperator(Atom& atom, const AnyString& name) const {
	auto ce = (ice() << "missing operator '" << name << "' for '");
	ce << atom.keyword() << ' ';
	atom.retrieveFullname(ce.data().message);
	ce << '\'';
	return false;
}


bool Analyzer::complainInvalidType(const char* origin, const Classdef& from, const Classdef& to) {
	auto err = (error() << origin << "cannot convert '");
	if (debugmode)
		err << from.clid << ':';
	from.print(err.data().message, cdeftable, false);
	err << "' to '";
	if (debugmode)
		err << to.clid << ':';
	to.print(err.data().message, cdeftable, false);
	err << '\'';
	return false;
}


bool Analyzer::complainCannotCall(Atom& atom, FuncOverloadMatch& overloadMatch) {
	auto err = (error() << "cannot call '" << cdeftable.keyword(atom) << ' ');
	atom.retrieveCaption(err.data().message, cdeftable);
	err << '\'';
	// no match, re-launching the process with error-enabled logging for user-reporting
	overloadMatch.report = &err;
	overloadMatch.validateWithErrReport(atom);
	return false;
}


void Analyzer::complainPushedSynthetic(const CLID& clid, uint32_t paramindex,
		const AnyString& paramname) {
	auto err = error();
	err << "cannot push synthetic object '";
	if (debugmode)
		err << clid << ' ';
	auto& cdef = cdeftable.classdef(clid);
	cdef.print(err.message.message, cdeftable, false);
	if (paramname.empty())
		err << "' for parameter " << paramindex;
	else
		err << "' for parameter '" << paramname << '\'';
}


void Analyzer::complainInvalidParametersAfterSignatureMatching(Atom& atom,
		FuncOverloadMatch& overloadMatch) {
	// fail - try again to produce error message, hint, and any suggestion
	auto err = (error());
	err << "invalid parameters, failed to instanciate '" << atom.keyword() << ' ';
	atom.retrieveFullname(err.data().message);
	err << '\'';
	overloadMatch.report = &err;
	overloadMatch.validateWithErrReport(atom);
}


} // namespace semantic
} // namespace ny
