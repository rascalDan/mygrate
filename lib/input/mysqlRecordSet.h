#ifndef MYGRATE_INPUT_MYSQLRECORDSET_H
#define MYGRATE_INPUT_MYSQLRECORDSET_H

#include "mysqlStmt.h"
#include <cstddef>
#include <dbRecordSet.h>
#include <dbTypes.h>
#include <memory>
#include <mysql.h>
#include <vector>

namespace MyGrate::Input {
	class ResultData;

	class MySQLRecordSet : public RecordSet {
	public:
		using ResPtr = std::unique_ptr<MYSQL_RES, decltype(&mysql_free_result)>;
		using StmtResPtr = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_free_result)>;
		using ResultDataPtr = std::unique_ptr<ResultData>;

		explicit MySQLRecordSet(StmtPtr s);

		std::size_t rows() const override;

		std::size_t columns() const override;

		DbValue at(std::size_t row, std::size_t col) const override;

	private:
		StmtPtr stmt;
		StmtResPtr stmtres;
		std::vector<MYSQL_BIND> fields;
		std::vector<ResultDataPtr> extras;
	};
}

#endif
