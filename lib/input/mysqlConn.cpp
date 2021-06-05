#include "mysqlConn.h"
#include "mysqlBindings.h"
#include "mysqlStmt.h"
#include <cstddef>
#include <cstring>
#include <dbConn.h>
#include <helpers.h>
#include <memory>
#include <mysql.h>
#include <stdexcept>
#include <vector>

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

	void
	MySQLConn::query(const char * const q, const std::initializer_list<DbValue> & vs)
	{
		StmtPtr stmt {mysql_stmt_init(this), &mysql_stmt_close};
		verify<std::runtime_error>(!mysql_stmt_prepare(stmt.get(), q, strlen(q)), q);
		verify<std::logic_error>(mysql_stmt_param_count(stmt.get()) == vs.size(), "Param count mismatch");
		Bindings b {vs};
		verify<std::runtime_error>(!mysql_stmt_bind_param(stmt.get(), b.binds.data()), "Param count mismatch");
		verify<std::runtime_error>(!mysql_stmt_execute(stmt.get()), q);
	}

	DbPrepStmtPtr
	MySQLConn::prepare(const char * const q, std::size_t)
	{
		return std::make_unique<MySQLPrepStmt>(q, this);
	}
}
