#ifndef MYGRATE_EVENTSOURCEBASE_H
#define MYGRATE_EVENTSOURCEBASE_H

#include "eventHandlerBase.h"
#include <memory>

namespace MyGrate {
	class EventSourceBase {
	public:
		virtual ~EventSourceBase() = default;
		virtual void readEvents(EventHandlerBase &) = 0;
	};
	using EventSourceBasePtr = std::unique_ptr<EventSourceBase>;
}

#endif
