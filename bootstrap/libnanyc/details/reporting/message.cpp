#include "message.h"
#include "details/atom/atom.h"
#include <yuni/core/system/console/console.h>

using namespace Yuni;

namespace ny {
namespace Logs {


Message& Message::createEntry(Level level) {
	auto entry = std::make_shared<Message>(level);
	MutexLocker locker{m_mutex};
	entry->origins = origins;
	entries.push_back(entry);
	return *entry;
}


void Message::appendEntry(std::unique_ptr<Message>& entry) {
	if (!!entry) {
		if (not (entry->entries.empty() and entry->level == Level::none)) {
			MutexLocker locker{m_mutex};
			hasErrors &= entry->hasErrors;
			entries.emplace_back(entry.release());
		}
	}
}


void Message::Origin::Location::resetFromAtom(const Atom& atom) {
	filename = atom.origin.filename;
	pos.line = atom.origin.line;
	pos.offset = atom.origin.offset;
}


} // namespace Logs
} // namespace ny
