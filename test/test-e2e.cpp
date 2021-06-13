#define BOOST_TEST_MODULE EndToEnd
#include <boost/test/unit_test.hpp>

#include "testdb-mysql.h"
#include "testdb-postgresql.h"
#include <output/pq/updateDatabase.h>

BOOST_AUTO_TEST_CASE(e2e)
{
	const char * const target_schema {"testout"};
	using namespace MyGrate::Testing;
	MySQLDB my;
	PqConnDB pq {ROOT "/db/schema.sql"};

	auto pqm = pq.mock();

	auto out = MyGrate::Output::Pq::UpdateDatabase::createNew(&pqm, MySQLDB::SERVER, MySQLDB::USER, MySQLDB::PASSWORD,
			MySQLDB::PORT, my.mockname.c_str(), 100, target_schema);
	BOOST_CHECK_EQUAL(out.source, 1);
	auto src = out.getSource();
	BOOST_REQUIRE(src);
}
