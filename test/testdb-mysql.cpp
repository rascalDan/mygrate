#include "testdb-mysql.h"
#include <cstdlib>
#include <helpers.h>
#include <input/mysqlConn.h>

namespace MyGrate {
	namespace Testing {
		const auto SERVER {MyGrate::getenv("MYGRATE_MYSQL_SERVER", "localhost")};
		const auto USER {MyGrate::getenv("MYGRATE_MYSQL_USER", ::getenv("LOGNAME"))};
		const auto PASSWORD {::getenv("MYGRATE_MYSQL_PASSWORD")};
		const auto PORT {(unsigned short)std::atoi(MyGrate::getenv("MYGRATE_MYSQL_PORT", "3306"))};

		MySQLDB::MySQLDB() : MySQLConn(SERVER, USER, PASSWORD, PORT)
		{
			query("DROP DATABASE IF EXISTS mygrate_test");
			query("CREATE DATABASE mygrate_test");
			query("USE mygrate_test");
		}

		MySQLDB::~MySQLDB()
		{
			query("USE mysql");
			query("DROP DATABASE IF EXISTS mygrate_test");
		}
	}
}
