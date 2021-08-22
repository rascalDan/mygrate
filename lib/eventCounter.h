#ifndef MYGRATE_EVENTCOUNTER_H
#define MYGRATE_EVENTCOUNTER_H

#include "mariadb_repl.h"
#include <array>

namespace MyGrate {
	class EventCounter {
	public:
		using Counters = std::array<unsigned long, ENUM_END_EVENT>;

		void tick(mariadb_rpl_event);
		const Counters & getAll() const;
		unsigned long get(mariadb_rpl_event) const;
		bool operator>=(const EventCounter &) const;

	protected:
		Counters counters {};
	};
}

#endif
