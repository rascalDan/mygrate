#ifndef MYGRATE_INPUT_REPLSTREAM_H
#define MYGRATE_INPUT_REPLSTREAM_H

#include "mysqlConn.h"
#include <cstdint>
#include <eventCounter.h>
#include <eventSourceBase.h>
#include <string>

namespace MyGrate {
	class EventHandlerBase;
}

namespace MyGrate::Input {
	class ReplicationStream : public EventSourceBase, public MySQLConn {
	public:
		ReplicationStream(const std::string & host, const std::string & user, const std::string & pass,
				unsigned short port, uint64_t serverid, std::string filename, uint64_t position);

		void readEvents(EventHandlerBase &) override;
		void stopEvents() override;

		const EventCounter & getReceivedCounts() const;
		const EventCounter & getHandledCounts() const;

	private:
		uint64_t serverid;
		std::string filename;
		uint64_t position;
		EventCounter received, handled;
	};
}

#endif
