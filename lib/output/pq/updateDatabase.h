#ifndef MYGRATE_OUTPUT_PQ_UPDATEDATABASE_H
#define MYGRATE_OUTPUT_PQ_UPDATEDATABASE_H

#include "pqConn.h"
#include <cstdint>
#include <eventHandlerBase.h>
#include <eventSourceBase.h>

namespace MyGrate::Output::Pq {
	class UpdateDatabase : public PqConn, public EventHandlerBase {
	public:
		UpdateDatabase(const char * const str, uint64_t source);

		EventSourceBasePtr getSource();

	private:
		uint64_t source;
	};
}

#endif
