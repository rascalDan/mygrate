#ifndef MYGRATE_INPUT_REPLSTREAM_H
#define MYGRATE_INPUT_REPLSTREAM_H

#include "mysqlConn.h"
#include <cstdint>
#include <eventSourceBase.h>
#include <string>

namespace MyGrate {
	class EventHandlerBase;
}

namespace MyGrate::Input {
	class ReplicationStream : public EventSourceBase, MySQLConn {
	public:
		ReplicationStream(const std::string & host, const std::string & user, const std::string & pass,
				unsigned short port, uint64_t serverid, std::string filename, uint64_t position);

		void readEvents(EventHandlerBase &) override;

	private:
		uint64_t serverid;
		std::string filename;
		uint64_t position;
	};
}

#endif
