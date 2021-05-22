#ifndef MYGRATE_OUTPUT_PQ_UPDATEDATABASE_H
#define MYGRATE_OUTPUT_PQ_UPDATEDATABASE_H

#include "pqConn.h"
#include <string>

namespace MyGrate::Output::Pq {
	class UpdateDatabase : PqConn {
	public:
		UpdateDatabase(const char * const str, std::string prefix);

	private:
		std::string prefix;
	};
}

#endif
