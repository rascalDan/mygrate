#ifndef MYGRATE_OUTPUT_PQ_PQCONN_H
#define MYGRATE_OUTPUT_PQ_PQCONN_H

#include <dbConn.h>
#include <dbTypes.h>
#include <initializer_list>
#include <libpq-fe.h>

namespace MyGrate::Output::Pq {
	class PqConn : public DbConn {
	public:
		explicit PqConn(const char * const str);
		virtual ~PqConn();

		void query(const char * const) override;
		void query(const char * const, const std::initializer_list<DbValue> &) override;

	private:
		static void notice_processor(void *, const char *);
		virtual void notice_processor(const char *) const;

		PGconn * const conn;
	};
}

#endif
