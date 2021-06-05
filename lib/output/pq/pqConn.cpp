#include "pqConn.h"
#include "pqBindings.h"
#include "pqStmt.h"
#include <dbConn.h>
#include <helpers.h>
#include <libpq-fe.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace MyGrate::Output::Pq {
	PqConn::PqConn(const char * const str) : conn {PQconnectdb(str), PQfinish}
	{
		verify<std::runtime_error>(PQstatus(conn.get()) == CONNECTION_OK, "Connection failure");
		PQsetNoticeProcessor(conn.get(), notice_processor, this);
	}

	void
	PqConn::query(const char * const q)
	{
		ResPtr res {PQexec(conn.get(), q), &PQclear};
		verify<std::runtime_error>(PQresultStatus(res.get()) == PGRES_COMMAND_OK, q);
	}

	void
	PqConn::query(const char * const q, const std::initializer_list<DbValue> & vs)
	{
		Bindings b {vs};
		ResPtr res {PQexecParams(conn.get(), q, (int)vs.size(), nullptr, b.values.data(), b.lengths.data(), nullptr, 0),
				&PQclear};
		verify<std::runtime_error>(PQresultStatus(res.get()) == PGRES_COMMAND_OK, q);
	}

	DbPrepStmtPtr
	PqConn::prepare(const char * const q, std::size_t n)
	{
		return std::make_unique<PqPrepStmt>(q, n, this);
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
