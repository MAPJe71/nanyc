#pragma once
#include "stack-frame.h"


namespace ny {
namespace semantic {


inline void LVIDInfo::fillLogEntryWithLocation(Logs::Report& entry) const {
	auto& origins = entry.origins();
	origins.location.pos.line   = file.line;
	origins.location.pos.offset = file.offset;
	origins.location.filename   = file.url;
}


inline AtomStackFrame::AtomStackFrame(Atom& atom, AtomStackFrame* previous)
	: atom(atom)
	, atomid(atom.atomid)
	, previous(previous) {
	m_locallvids.resize(atom.localVariablesCount);
}


inline uint32_t AtomStackFrame::localVariablesCount() const {
	return static_cast<uint32_t>(m_locallvids.size());
}


inline uint32_t AtomStackFrame::findLocalVariable(const AnyString& name) const {
	uint32_t count = localVariablesCount();
	for (uint32_t i = count; i--; ) {
		if (m_locallvids[i].userDefinedName == name)
			return i;
	}
	return 0u;
}


inline void AtomStackFrame::resizeRegisterCount(uint32_t count, ClassdefTableView& table) {
	if (count >= m_locallvids.size())
		m_locallvids.resize(count);
	table.substituteResize(count);
}


inline bool AtomStackFrame::verify(uint32_t lvid) const {
	assert(lvid != 0);
	return likely(not lvids(lvid).errorReported);
}


inline void AtomStackFrame::invalidate(uint32_t lvid) {
	assert(lvid != 0);
	lvids(lvid).errorReported = true;
}


inline LVIDInfo& AtomStackFrame::lvids(uint32_t i) {
	assert(i < m_locallvids.size());
	return m_locallvids[i];
}


inline const LVIDInfo& AtomStackFrame::lvids(uint32_t i) const {
	assert(i < m_locallvids.size());
	return m_locallvids[i];
}


} // namespace semantic
} // namespace ny
