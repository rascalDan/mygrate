#ifndef MYGRATE_OUTPUT_PQ_PQCONN_H
#define MYGRATE_OUTPUT_PQ_PQCONN_H

#include <cstddef>
#include <cstdio>
#include <dbConn.h>
#include <functional>
#include <initializer_list>
#include <libpq-fe.h>
#include <map>
#include <memory>
#include <string>

namespace MyGrate {
	class DbValue;
}
namespace MyGrate::Output::Pq {
	class PqErr : public std::runtime_error {
	public:
		PqErr(const std::string & when, PGconn *);
		PqErr(const std::string & when, PGresult *);
	};

	class PqConn : public DbConn {
	public:
		static constexpr auto paramMode {ParamMode::DollarNum};

		explicit PqConn(const char * const str);
		virtual ~PqConn() = default;

		void query(const char * const) override;
		void query(const char * const, const std::initializer_list<DbValue> &) override;

		DbPrepStmtPtr prepare(const char * const, std::size_t nParams) override;

		void beginTx() override;
		void commitTx() override;
		void rollbackTx() override;

		FILE * beginBulkUpload(const char * schema, const char * table);
		void endBulkUpload(const char * errormsg);

		const std::string connstr;

	private:
		static void notice_processor(void *, const char *);
		virtual void notice_processor(const char *) const;

		std::unique_ptr<PGconn, decltype(&PQfinish)> const conn;

		friend class PqPrepStmt;
		std::map<std::string, std::string, std::less<>> stmts;
	};
}

#endif
