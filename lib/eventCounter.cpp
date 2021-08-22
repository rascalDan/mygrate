#include "eventCounter.h"
#include <algorithm>

namespace MyGrate {
	void
	EventCounter::tick(mariadb_rpl_event e)
	{
		counters[e]++;
	}
	const EventCounter::Counters &
	EventCounter::getAll() const
	{
		return counters;
	}
	unsigned long
	EventCounter::get(mariadb_rpl_event e) const
	{
		return counters[e];
	}
	bool
	EventCounter::operator>=(const EventCounter & target) const
	{
		for (size_t e {}; e < ENUM_END_EVENT; e++) {
			if (counters[e] < target.counters[e]) {
				return false;
			}
		}
		return true;
	}
}
