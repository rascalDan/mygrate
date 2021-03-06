#include "testdb-mysql.h"
#include <compileTimeFormatter.h>
#include <cstdlib>
#include <helpers.h>
#include <input/mysqlConn.h>

namespace MyGrate::Testing {
	const char * const MySQLDB::SERVER {MyGrate::getenv("MYGRATE_MYSQL_SERVER", "localhost")};
	const char * const MySQLDB::USER {MyGrate::getenv("MYGRATE_MYSQL_USER", ::getenv("LOGNAME"))};
	const char * const MySQLDB::PASSWORD {::getenv("MYGRATE_MYSQL_PASSWORD")};
	const unsigned short MySQLDB::PORT {
			static_cast<unsigned short>(std::atoi(MyGrate::getenv("MYGRATE_MYSQL_PORT", "3306")))};
	std::size_t MySQLDB::mocknum;

	MySQLDB::MySQLDB() :
		MySQLConn(SERVER, USER, PASSWORD, PORT), mockname {scprintf<"mygrate_test_%?_%?">(getpid(), mocknum++)}
	{
		query(("DROP DATABASE IF EXISTS " + mockname).c_str());
		query(("CREATE DATABASE " + mockname).c_str());
	}

	MySQLDB::~MySQLDB()
	{
		query(("DROP DATABASE IF EXISTS " + mockname).c_str());
		mockname.clear();
	}

	Input::MySQLConn
	MySQLDB::mock() const
	{
		return {SERVER, USER, PASSWORD, PORT, mockname.c_str()};
	}
}
