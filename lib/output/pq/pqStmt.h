#ifndef MYGRATE_OUTPUT_PQ_PQSTMT_H
#define MYGRATE_OUTPUT_PQ_PQSTMT_H

#include "dbConn.h"
#include "dbRecordSet.h"
#include <cstddef>
#include <initializer_list>
#include <libpq-fe.h>
#include <memory>
#include <string>

namespace MyGrate {
	class DbValue;
}
namespace MyGrate::Output::Pq {
	class PqConn;

	using ResPtr = std::unique_ptr<PGresult, decltype(&PQclear)>;

	class PqPrepStmt : public DbPrepStmt {
	public:
		PqPrepStmt(const char * const q, std::size_t n, PqConn * c);

		void execute(const std::initializer_list<DbValue> & vs) override;

		std::size_t rows() const override;

		RecordSetPtr recordSet() override;

	private:
		static std::string prepareAsNeeded(const char * const q, std::size_t n, PqConn * c);

		PGconn * conn;
		std::string name;
		ResPtr res;
	};
}

#endif
