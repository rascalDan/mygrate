#ifndef MYGRATE_INPUT_MYSQLCONN_H
#define MYGRATE_INPUT_MYSQLCONN_H

#include <mysql.h>

namespace MyGrate::Input {
	class MySQLConn : public MYSQL {
	public:
		MySQLConn(const char * const host, const char * const user, const char * const pass, unsigned short port);
		~MySQLConn();
	};
}

#endif
