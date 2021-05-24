#include "pqConn.h"
#include <dbTypes.h>
#include <helpers.h>
#include <libpq-fe.h>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace MyGrate::Output::Pq {
	using ResPtr = std::unique_ptr<PGresult, decltype(&PQclear)>;

	PqConn::PqConn(const char * const str) : conn {PQconnectdb(str)}
	{
		verify<std::runtime_error>(PQstatus(conn) == CONNECTION_OK, "Connection failure");
		PQsetNoticeProcessor(conn, notice_processor, this);
	}

	PqConn::~PqConn()
	{
		PQfinish(conn);
	}

	void
	PqConn::query(const char * const q)
	{
		ResPtr res {PQexec(conn, q), &PQclear};
		verify<std::runtime_error>(PQresultStatus(res.get()) == PGRES_COMMAND_OK, q);
	}

	struct Bindings {
		// NOLINTNEXTLINE(hicpp-explicit-conversions)
		explicit Bindings(const std::initializer_list<DbValue> & vs)
		{
			bufs.reserve(vs.size());
			values.reserve(vs.size());
			lengths.reserve(vs.size());
			for (const auto & v : vs) {
				std::visit(*this, v);
			}
		}
		template<Stringable T>
		void
		operator()(const T & v)
		{
			bufs.emplace_back(std::to_string(v));
			const auto & vw {bufs.back()};
			values.emplace_back(vw.data());
			lengths.emplace_back(vw.length());
		}
		template<Viewable T>
		void
		operator()(const T & v)
		{
			values.emplace_back(v.data());
			lengths.emplace_back(v.size());
		}
		template<typename T>
		void
		operator()(const T &)
		{
			throw std::runtime_error("Not implemented");
		}
		void
		operator()(const std::nullptr_t &)
		{
			values.emplace_back(nullptr);
			lengths.emplace_back(0);
		}

		std::vector<std::string> bufs;
		std::vector<const char *> values;
		std::vector<int> lengths;
	};

	void
	PqConn::query(const char * const q, const std::initializer_list<DbValue> & vs)
	{
		Bindings b {vs};
		ResPtr res {PQexecParams(conn, q, (int)vs.size(), nullptr, b.values.data(), b.lengths.data(), nullptr, 0),
				&PQclear};
		verify<std::runtime_error>(PQresultStatus(res.get()) == PGRES_COMMAND_OK, q);
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
