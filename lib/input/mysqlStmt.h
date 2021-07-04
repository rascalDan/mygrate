#ifndef MYGRATE_INPUT_MYSQLSTMT_H
#define MYGRATE_INPUT_MYSQLSTMT_H

#include "dbConn.h"
#include "dbRecordSet.h"
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <mysql.h>

namespace MyGrate {
	class DbValue;
}
namespace MyGrate::Input {
	using StmtPtr = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>;

	class MySQLPrepStmt : public DbPrepStmt {
	public:
		MySQLPrepStmt(const char * const q, MYSQL * c);
		void execute(const std::initializer_list<DbValue> & vs) override;

		std::size_t rows() const override;

		RecordSetPtr recordSet() override;

		CursorPtr cursor() override;

	private:
		StmtPtr stmt;
	};
}

#endif
