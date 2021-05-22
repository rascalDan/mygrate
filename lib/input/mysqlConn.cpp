#include "mysqlConn.h"
#include <stdexcept>

namespace MyGrate::Input {
	MySQLConn::MySQLConn(
			const char * const host, const char * const user, const char * const pass, unsigned short port) :
		st_mysql {}
	{
		mysql_init(this);
		if (!mysql_real_connect(this, host, user, pass, "", port, nullptr, 0)) {
			mysql_close(this);
			throw std::runtime_error("ConnectionError");
		}
		mysql_query(this, "SET NAMES utf8");
	}

	MySQLConn::~MySQLConn()
	{
		mysql_close(this);
	}
}
