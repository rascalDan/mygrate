#ifndef MYGRATE_TESTING_POSTGRESQL_H
#define MYGRATE_TESTING_POSTGRESQL_H

#include <output/pq/pqConn.h>

namespace MyGrate {
	namespace Testing {
		class PqConnDB : public Output::Pq::PqConn {
		public:
			PqConnDB();
			~PqConnDB();

			Output::Pq::PqConn mock() const;

			std::string mockname;
			static std::size_t mocknum;
		};
	}
}

#endif
