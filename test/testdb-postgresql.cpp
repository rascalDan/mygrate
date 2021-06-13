#include "testdb-postgresql.h"
#include <compileTimeFormatter.h>
#include <cstdlib>
#include <fstream>
#include <helpers.h>
#include <output/pq/pqConn.h>

namespace MyGrate {
	namespace Testing {
		const auto CONNSTR {MyGrate::getenv("MYGRATE_POSTGRESQL_CONNSTR", "user=postgres")};
		std::size_t PqConnDB::mocknum;

		PqConnDB::PqConnDB() : PqConn(CONNSTR), mockname {scprintf<"mygrate_test_%?_%?">(getpid(), mocknum++)}
		{
			query(("DROP DATABASE IF EXISTS " + mockname).c_str());
			query(("CREATE DATABASE " + mockname).c_str());
		}

		PqConnDB::PqConnDB(const std::string & schemaFile) : PqConnDB()
		{
			auto mdb = mock();

			std::stringstream buffer;
			buffer << std::ifstream(schemaFile).rdbuf();

			mdb.query(buffer.str().c_str());
		}

		PqConnDB::~PqConnDB()
		{
			query(("DROP DATABASE IF EXISTS " + mockname).c_str());
			mockname.clear();
		}

		Output::Pq::PqConn
		PqConnDB::mock() const
		{
			return PqConn {scprintf<"%? dbname=%?">(CONNSTR, mockname).c_str()};
		}
	}
}
