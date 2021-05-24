#ifndef MYGRATE_DBCONN_H
#define MYGRATE_DBCONN_H

#include <dbTypes.h>
#include <initializer_list>

namespace MyGrate {
	class DbConn {
		virtual void query(const char * const) = 0;
		virtual void query(const char * const, const std::initializer_list<DbValue> &) = 0;
	};
}

#endif
