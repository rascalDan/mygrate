#ifndef MYGRATE_INPUT_REPLSTREAM_H
#define MYGRATE_INPUT_REPLSTREAM_H

#include "../eventSourceBase.h"
#include "mysqlConn.h"

namespace MyGrate::Input {
	class ReplicationStream : public EventSourceBase, MySQLConn {
	public:
		using MySQLConn::MySQLConn;

		void readEvents(EventHandlerBase &) override;
	};
}

#endif
