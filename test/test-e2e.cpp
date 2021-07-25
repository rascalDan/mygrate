#define BOOST_TEST_MODULE EndToEnd
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "testdb-mysql.h"
#include "testdb-postgresql.h"
#include <input/replStream.h>
#include <output/pq/updateDatabase.h>
#include <sql/createTestTable.h>
#include <sql/fillTestTable.h>
#include <sql/selectTestTable.h>
#include <thread>

BOOST_AUTO_TEST_CASE(e2e)
{
	const char * const target_schema {"testout"};
	using namespace MyGrate::Testing;
	MySQLDB my;
	PqConnDB pq {ROOT "/db/schema.sql"};

	auto pqm = pq.mock();
	auto mym = my.mock();

	MyGrate::sql::createTestTable::execute(&mym);
	MyGrate::sql::fillTestTable::execute(&mym);

	auto out = MyGrate::Output::Pq::UpdateDatabase::createNew(&pqm, MySQLDB::SERVER, MySQLDB::USER, MySQLDB::PASSWORD,
			MySQLDB::PORT, my.mockname.c_str(), 100, target_schema);
	BOOST_CHECK_EQUAL(out.source, 1);
	auto src = out.getSource();
	BOOST_REQUIRE(src);

	out.addTable(&mym, "session");
	out.copyTableContent(&mym, "session");

	BOOST_CHECK_EQUAL(MyGrate::sql::selectTestTable::execute(&pqm)->rows(), 1);

	std::thread repl {&MyGrate::EventSourceBase::readEvents, src.get(), std::ref(out)};

	auto upd = mym.prepare("UPDATE session SET session_id = ? WHERE id = ?", 2);
	upd->execute({"food", 1});
	auto del = mym.prepare("DELETE FROM session WHERE id = ?", 2);
	del->execute({1});
	auto ins = mym.prepare("INSERT INTO session(session_id, username, user_lvl, ip_addr, port, created, modified) \
			VALUES(?, ?, ?, ?, ?, now(), now())",
			5);
	ins->execute({"hashyhash", "testuser", "groupadm", "10.10.0.1", 2433});
	mym.query("flush logs");

	sleep(1);

	src->stopEvents();
	repl.join();
}
