#include "pqConn.h"
#include <helpers.h>
#include <stdexcept>

namespace MyGrate::Output::Pq {
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
	PqConn::notice_processor(void * p, const char * n)
	{
		return static_cast<PqConn *>(p)->notice_processor(n);
	}

	void
	PqConn::notice_processor(const char *) const
	{
	}
}
