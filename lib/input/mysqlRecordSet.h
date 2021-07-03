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

	class MySQLData {
	public:
		using ResultDataPtr = std::unique_ptr<ResultData>;
		using ResPtr = std::unique_ptr<MYSQL_RES, decltype(&mysql_free_result)>;

		explicit MySQLData(StmtPtr s);

		std::size_t columns() const;

		StmtPtr stmt;
		std::vector<MYSQL_BIND> fields;
		std::vector<ResultDataPtr> extras;
	};

	class MySQLRecordSet : public MySQLData, public RecordSet {
	public:
		using StmtResPtr = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_free_result)>;

		explicit MySQLRecordSet(StmtPtr s);

		std::size_t rows() const override;

		std::size_t columns() const override;

		DbValue at(std::size_t row, std::size_t col) const override;

	private:
		StmtResPtr stmtres;
		mutable std::size_t currentRow {static_cast<size_t>(-1)};
	};
}

#endif
