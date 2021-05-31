#ifndef MYGRATE_INPUT_MYSQLCONN_H
#define MYGRATE_INPUT_MYSQLCONN_H

#include <cstddef>
#include <dbConn.h>
#include <dbTypes.h>
#include <initializer_list>
#include <mysql.h>

namespace MyGrate::Input {
	class MySQLConn : public MYSQL, public DbConn {
	public:
		MySQLConn(const char * const host, const char * const user, const char * const pass, unsigned short port);
		~MySQLConn();

		void query(const char * const) override;
		void query(const char * const q, const std::initializer_list<DbValue> &) override;

		DbPrepStmtPtr prepare(const char * const, std::size_t) override;
	};
}

#endif
