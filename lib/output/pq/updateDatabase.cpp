#include "updateDatabase.h"
#include "pqConn.h"
#include <string>
#include <utility>

namespace MyGrate::Output::Pq {
	UpdateDatabase::UpdateDatabase(const char * const str, std::string p) : PqConn {str}, prefix {std::move(p)} { }
}
