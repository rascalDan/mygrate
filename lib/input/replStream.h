#ifndef MYGRATE_INPUT_REPLSTREAM_H
#define MYGRATE_INPUT_REPLSTREAM_H

#include "mysqlConn.h"
#include <eventSourceBase.h>

namespace MyGrate {
	class EventHandlerBase;
}

namespace MyGrate::Input {
	class ReplicationStream : public EventSourceBase, MySQLConn {
	public:
		using MySQLConn::MySQLConn;

		void readEvents(EventHandlerBase &) override;
	};
}

#endif
