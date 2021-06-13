changequote(`{{',`}}')dnl
// NAME
#include <fixedString.h>
#include <dbStmt.h>

namespace MyGrate::NS {
	using NAME = ::MyGrate::DbStmt<R"SQL(dnl
include(SOURCE))SQL">;
}
