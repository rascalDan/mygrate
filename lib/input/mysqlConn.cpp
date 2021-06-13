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
	MySQLErr::MySQLErr(const std::string & when, MYSQL * c) : std::runtime_error(when + ": " + mysql_error(c)) { }

	MySQLConn::MySQLConn(const char * const host, const char * const user, const char * const pass, unsigned short port,
			const char * const db) :
		st_mysql {}
	{
		mysql_init(this);
		if (!mysql_real_connect(this, host, user, pass, db, port, nullptr, 0)) {
			mysql_close(this);
			throw MySQLErr("Connecting", this);
		}
		verify<MySQLErr>(!mysql_set_character_set(this, "utf8"), "Setting char set", this);
	}

	MySQLConn::~MySQLConn()
	{
		mysql_close(this);
	}

	void
	MySQLConn::query(const char * const q)
	{
		verify<MySQLErr>(!mysql_query(this, q), q, this);
	}

	void
	MySQLConn::query(const char * const q, const std::initializer_list<DbValue> & vs)
	{
		StmtPtr stmt {mysql_stmt_init(this), &mysql_stmt_close};
		verify<MySQLErr>(!mysql_stmt_prepare(stmt.get(), q, strlen(q)), q, this);
		verify<std::logic_error>(mysql_stmt_param_count(stmt.get()) == vs.size(), "Param count mismatch");
		Bindings b {vs};
		verify<std::logic_error>(!mysql_stmt_bind_param(stmt.get(), b.binds.data()), "Param count mismatch");
		verify<MySQLErr>(!mysql_stmt_execute(stmt.get()), q, this);
	}

	DbPrepStmtPtr
	MySQLConn::prepare(const char * const q, std::size_t)
	{
		return std::make_unique<MySQLPrepStmt>(q, this);
	}

	void
	MySQLConn::beginTx()
	{
		verify<MySQLErr>(!mysql_autocommit(this, false), "Auto commit off", this);
	}

	void
	MySQLConn::commitTx()
	{
		verify<MySQLErr>(!mysql_commit(this), "Commit", this);
		verify<MySQLErr>(!mysql_autocommit(this, true), "Auto commit on", this);
	}

	void
	MySQLConn::rollbackTx()
	{
		verify<MySQLErr>(!mysql_rollback(this), "Rollback", this);
		verify<MySQLErr>(!mysql_autocommit(this, true), "Auto commit on", this);
	}
}
