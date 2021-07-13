#include "pqStmt.h"
#include "libpq-fe.h"
#include "pqBindings.h"
#include "pqConn.h"
#include "pqRecordSet.h"
#include <compileTimeFormatter.h>
#include <cstdlib>
#include <functional>
#include <helpers.h>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

namespace MyGrate::Output::Pq {
	PqPrepStmt::PqPrepStmt(const char * const q, std::size_t n, PqConn * c) :
		conn {c->conn.get()}, name {prepareAsNeeded(q, n, c)}, res {nullptr, nullptr}
	{
	}

	void
	PqPrepStmt::execute(const std::initializer_list<DbValue> & vs)
	{
		Bindings b {vs};
		res = {PQexecPrepared(
					   conn, name.c_str(), (int)vs.size(), b.values.data(), b.lengths.data(), b.formats.data(), 0),
				&PQclear};
		verify<PqErr>(PQresultStatus(res.get()) == PGRES_COMMAND_OK || PQresultStatus(res.get()) == PGRES_TUPLES_OK,
				name, conn);
	}

	std::size_t
	PqPrepStmt::rows() const
	{
		return std::strtoul(PQcmdTuples(res.get()), nullptr, 10);
	}

	RecordSetPtr
	PqPrepStmt::recordSet()
	{
		return std::make_unique<PqRecordSet>(std::move(res));
	}

	CursorPtr
	PqPrepStmt::cursor()
	{
		throw std::logic_error("Not implemented");
	}

	std::string
	PqPrepStmt::prepareAsNeeded(const char * const q, std::size_t n, PqConn * c)
	{
		if (const auto i = c->stmts.find(q); i != c->stmts.end()) {
			return i->second;
		}
		auto nam {scprintf<"pst%0x">(c->stmts.size())};
		ResPtr res {PQprepare(c->conn.get(), nam.c_str(), q, (int)n, nullptr), PQclear};
		verify<PqErr>(PQresultStatus(res.get()) == PGRES_COMMAND_OK, q, c->conn.get());
		return c->stmts.emplace(q, std::move(nam)).first->second;
	}
}
