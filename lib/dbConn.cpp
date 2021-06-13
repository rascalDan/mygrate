#include "dbConn.h"

namespace MyGrate {
	Tx::Tx(DbConn * c) : conn {c}
	{
		conn->beginTx();
	}

	Tx::~Tx()
	{
		if (std::uncaught_exceptions()) {
			conn->rollbackTx();
		}
		else {
			conn->commitTx();
		}
	}
}
