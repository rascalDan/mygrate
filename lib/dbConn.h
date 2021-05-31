#ifndef MYGRATE_DBCONN_H
#define MYGRATE_DBCONN_H

#include <dbRecordSet.h>
#include <dbTypes.h>
#include <initializer_list>

namespace MyGrate {
	class DbPrepStmt {
	public:
		virtual ~DbPrepStmt() = default;
		virtual void execute(const std::initializer_list<DbValue> &) = 0;
		virtual std::size_t rows() const = 0;
		virtual RecordSetPtr recordSet() = 0;
	};
	using DbPrepStmtPtr = std::unique_ptr<DbPrepStmt>;

	class DbConn {
	public:
		virtual ~DbConn() = default;
		virtual void query(const char * const) = 0;
		virtual void query(const char * const, const std::initializer_list<DbValue> &) = 0;

		virtual DbPrepStmtPtr prepare(const char * const, std::size_t nParams) = 0;
	};
}

#endif
