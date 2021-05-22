#ifndef MYGRATE_OUTPUT_PQ_PQCONN_H
#define MYGRATE_OUTPUT_PQ_PQCONN_H

#include <libpq-fe.h>

namespace MyGrate::Output::Pq {
	class PqConn {
	public:
		explicit PqConn(const char * const str);
		virtual ~PqConn();

	private:
		static void notice_processor(void *, const char *);
		virtual void notice_processor(const char *) const;

		PGconn * const conn;
	};
}

#endif
