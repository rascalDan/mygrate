#include "mysqlConn.h"
#include "helpers.h"
#include <mysql.h>
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
		verify<std::runtime_error>(!mysql_set_character_set(this, "utf8"), "Set character set");
	}

	MySQLConn::~MySQLConn()
	{
		mysql_close(this);
	}

	void
	MySQLConn::query(const char * const q)
	{
		verify<std::runtime_error>(!mysql_query(this, q), q);
	}
}
