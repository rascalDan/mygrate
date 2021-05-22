#ifndef MYGRATE_EVENTSOURCEBASE_H
#define MYGRATE_EVENTSOURCEBASE_H

#include "eventHandlerBase.h"

namespace MyGrate {
	class EventSourceBase {
	public:
		virtual void readEvents(EventHandlerBase &) = 0;
	};
}

#endif
