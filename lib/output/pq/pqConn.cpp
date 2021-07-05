#include "pqConn.h"
#include "pqBindings.h"
#include "pqStmt.h"
#include <compileTimeFormatter.h>
#include <dbConn.h>
#include <helpers.h>
#include <libpq-fe.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace MyGrate::Output::Pq {
	PqErr::PqErr(const std::string & when, PGconn * c) : std::runtime_error(when + ": " + PQerrorMessage((c))) { }
	PqErr::PqErr(const std::string & when, PGresult * r) : std::runtime_error(when + ": " + PQresultErrorMessage((r)))
	{
	}

	PqConn::PqConn(const char * const str) : connstr {str}, conn {PQconnectdb(str), PQfinish}
	{
		verify<PqErr>(PQstatus(conn.get()) == CONNECTION_OK, "Connection failure", conn.get());
		verify<PqErr>(!PQsetClientEncoding(conn.get(), "utf-8"), "Setting char set", conn.get());
		PQsetNoticeProcessor(conn.get(), notice_processor, this);
	}

	void
	PqConn::query(const char * const q)
	{
		ResPtr res {PQexec(conn.get(), q), &PQclear};
		verify<PqErr>(PQresultStatus(res.get()) == PGRES_COMMAND_OK, q, res.get());
	}

	void
	PqConn::query(const char * const q, const std::initializer_list<DbValue> & vs)
	{
		Bindings b {vs};
		ResPtr res {PQexecParams(conn.get(), q, (int)vs.size(), nullptr, b.values.data(), b.lengths.data(), nullptr, 0),
				&PQclear};
		verify<PqErr>(PQresultStatus(res.get()) == PGRES_COMMAND_OK, q, res.get());
	}

	DbPrepStmtPtr
	PqConn::prepare(const char * const q, std::size_t n)
	{
		return std::make_unique<PqPrepStmt>(q, n, this);
	}

	void
	PqConn::beginTx()
	{
		query("BEGIN");
	}

	void
	PqConn::commitTx()
	{
		query("COMMIT");
	}

	void
	PqConn::rollbackTx()
	{
		query("ROLLBACK");
	}

	FILE *
	PqConn::beginBulkUpload(const char * schema, const char * table)
	{
		ResPtr res {PQexec(conn.get(), scprintf<"COPY %?.%? FROM STDIN">(schema, table).c_str()), &PQclear};
		verify<PqErr>(PQresultStatus(res.get()) == PGRES_COPY_IN, "begin copy", res.get());
		return fopencookie(this, "w",
				{nullptr,
						[](void * cookie, const char * buf, size_t size) {
							auto conn = static_cast<PqConn *>(cookie)->conn.get();
							verify<PqErr>(PQputCopyData(conn, buf, (int)size) == 1, "copy data", conn);
							return (ssize_t)size;
						},
						nullptr,
						[](void * cookie) {
							static_cast<PqConn *>(cookie)->endBulkUpload(nullptr);
							return 0;
						}});
	}

	void
	PqConn::endBulkUpload(const char * errormsg)
	{
		int rc;
		while (!(rc = PQputCopyEnd(this->conn.get(), errormsg))) {
			sleep(1);
		}
		verify<PqErr>(rc == 1, "copy end rc", conn.get());
		ResPtr res {PQgetResult(conn.get()), &PQclear};
		verify<PqErr>(PQresultStatus(res.get()) == PGRES_COMMAND_OK, "end copy", res.get());
	}

	void
	PqConn::notice_processor(void * p, const char * n)
	{
		return static_cast<PqConn *>(p)->notice_processor(n);
	}

	void
	PqConn::notice_processor(const char *) const
	{
	}
}
