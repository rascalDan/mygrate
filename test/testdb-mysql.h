#ifndef MYGRATE_TESTING_MYSQL_H
#define MYGRATE_TESTING_MYSQL_H

#include <input/mysqlConn.h>

namespace MyGrate {
	namespace Testing {
		class MySQLDB : public Input::MySQLConn {
		public:
			MySQLDB();
			~MySQLDB();
		};
	}
}

#endif