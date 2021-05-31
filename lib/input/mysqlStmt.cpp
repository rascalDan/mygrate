#include "mysqlStmt.h"
#include "mysqlBindings.h"
#include "mysqlRecordSet.h"
#include <cstring>
#include <helpers.h>
#include <stdexcept>
#include <utility>
#include <vector>

namespace MyGrate::Input {
	MySQLPrepStmt::MySQLPrepStmt(const char * const q, MYSQL * c) : stmt {mysql_stmt_init(c), &mysql_stmt_close}
	{
		verify<std::runtime_error>(!mysql_stmt_prepare(stmt.get(), q, strlen(q)), q);
	}

	void
	MySQLPrepStmt::execute(const std::initializer_list<DbValue> & vs)
	{
		Bindings b {vs};
		verify<std::runtime_error>(!mysql_stmt_bind_param(stmt.get(), b.binds.data()), "Param count mismatch");
		verify<std::runtime_error>(!mysql_stmt_execute(stmt.get()), "Prepared statement execute");
	}

	std::size_t
	MySQLPrepStmt::rows() const
	{
		return mysql_stmt_affected_rows(stmt.get());
	}

	RecordSetPtr
	MySQLPrepStmt::recordSet()
	{
		return std::make_unique<MySQLRecordSet>(std::move(stmt));
	}
}
