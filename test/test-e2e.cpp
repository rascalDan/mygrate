#define BOOST_TEST_MODULE EndToEnd
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "helpers.h"
#include "testdb-mysql.h"
#include "testdb-postgresql.h"
#include <compileTimeFormatter.h>
#include <condition_variable>
#include <dbConn.h>
#include <input/replStream.h>
#include <output/pq/updateDatabase.h>
#include <sql/createTestTable.h>
#include <sql/fillTestTable.h>
#include <sql/selectTestTable.h>
#include <sql/simpleCreate.h>
#include <sql/simpleDeleteSome.h>
#include <sql/simpleInsert.h>
#include <sql/simpleSelectAll.h>
#include <sql/simpleUpdate.h>
#include <sql/simpleUpdateAll.h>
#include <thread>

class TestUpdateDatabase : public MyGrate::Output::Pq::UpdateDatabase {
public:
	using MyGrate::Output::Pq::UpdateDatabase::UpdateDatabase;
	void
	afterEvent(const MyGrate::MariaDB_Event_Ptr & e) override
	{
		{
			std::lock_guard<std::mutex> lk(m);
			UpdateDatabase::afterEvent(std::move(e));
		}
		cv.notify_all();
	}

	void
	waitFor(EventCounterTarget ect)
	{
		std::unique_lock<std::mutex> lk(m);
		cv.wait(lk, [&] {
			return getProcessedCounts() >= ect;
		});
	}

private:
	std::condition_variable cv;
	std::mutex m;
};

using namespace MyGrate::Testing;
class MockSetup {
public:
	static constexpr const char * const target_schema {"testout"};
	MockSetup() : pq {ROOT "/db/schema.sql"}, pqm {pq.mock()}, mym {my.mock()} { }

	TestUpdateDatabase &
	getUpdateDatabase()
	{
		BOOST_REQUIRE(!out);
		auto & o = out.emplace(pqm.connstr.c_str(),
				MyGrate::Output::Pq::UpdateDatabase::createNew(&pqm, MySQLDB::SERVER, MySQLDB::USER, MySQLDB::PASSWORD,
						MySQLDB::PORT, my.mockname.c_str(), 100, target_schema)
						.source);
		BOOST_CHECK_EQUAL(o.source, 1);
		return o;
	}

	const MyGrate::EventSourceBasePtr &
	getSource()
	{
		BOOST_REQUIRE(out);
		if (!src) {
			src = out->getSource();
			BOOST_REQUIRE(src);
		}
		return src;
	}

	void
	run()
	{
		BOOST_REQUIRE(out);
		BOOST_REQUIRE(!repl);
		repl.emplace(&MyGrate::EventSourceBase::readEvents, getSource().get(), std::ref(*out));
	}

	void
	stopAfter(const EventCounterTarget & events)
	{
		BOOST_REQUIRE(out);
		BOOST_REQUIRE(src);
		BOOST_REQUIRE(repl);
		out->waitFor(events);
		src->stopEvents();
		repl->join();
		repl.reset();
	}

	MySQLDB my;
	PqConnDB pq;
	MyGrate::Output::Pq::PqConn pqm;
	MyGrate::Input::MySQLConn mym;

	std::optional<TestUpdateDatabase> out;
	MyGrate::EventSourceBasePtr src;
	std::optional<std::thread> repl;
};

BOOST_FIXTURE_TEST_SUITE(setup, MockSetup);

BOOST_AUTO_TEST_CASE(e2e, *boost::unit_test::timeout(15))
{
	MyGrate::sql::createTestTable::execute(&mym);
	MyGrate::sql::fillTestTable::execute(&mym);

	TestUpdateDatabase & out {getUpdateDatabase()};

	out.addTable(&mym, "session");

	BOOST_CHECK_EQUAL(MyGrate::sql::selectTestTable::execute(&pqm)->rows(), 1);

	run();

	auto upd = mym.prepare("UPDATE session SET session_id = ? WHERE id = ?", 2);
	upd->execute({"food", 1});
	auto del = mym.prepare("DELETE FROM session WHERE id = ?", 2);
	del->execute({1});
	auto ins = mym.prepare("INSERT INTO session(session_id, username, user_lvl, ip_addr, port, created, modified) \
			VALUES(?, ?, ?, ?, ?, now(), now())",
			5);
	ins->execute({"hashyhash", "testuser", "groupadm", "10.10.0.1", 2433});
	mym.query("flush logs");

	stopAfter(EventCounterTarget {}
					  .add(UPDATE_ROWS_EVENT_V1, 1)
					  .add(DELETE_ROWS_EVENT_V1, 1)
					  .add(WRITE_ROWS_EVENT_V1, 1)
					  .add(ROTATE_EVENT, 1));
}

BOOST_AUTO_TEST_CASE(txns, *boost::unit_test::timeout(15))
{
	MyGrate::sql::simpleCreate::execute(&mym);

	TestUpdateDatabase & out {getUpdateDatabase()};

	out.addTable(&mym, "test");

	run();
	MyGrate::Tx {&mym}([&]() {
		for (unsigned int x = 0; x < 10; x += 1) {
			MyGrate::sql::simpleInsert::execute(&mym, MyGrate::scprintf<"some string %?">(x));
		}
		MyGrate::sql::simpleUpdateAll::execute(&mym, "Same");
		MyGrate::sql::simpleDeleteSome::execute(&mym, 5);
	});
	stopAfter(EventCounterTarget {}
					  .add(GTID_EVENT, 1)
					  .add(WRITE_ROWS_EVENT_V1, 10)
					  .add(UPDATE_ROWS_EVENT_V1, 1)
					  .add(DELETE_ROWS_EVENT_V1, 1)
					  .add(XID_EVENT, 1));

	auto recs = MyGrate::sql::simpleSelectAll::execute(&pqm);
	BOOST_REQUIRE_EQUAL(recs->rows(), 6);
	BOOST_CHECK_EQUAL(recs->at(0, 0).get<int>(), 5);
	BOOST_CHECK_EQUAL(recs->at(5, 0).get<int>(), 10);
	BOOST_CHECK_EQUAL(recs->at(1, 1).get<std::string_view>(), "Same");
	BOOST_CHECK_EQUAL(recs->at(3, 1).get<std::string_view>(), "Same");
}

BOOST_AUTO_TEST_SUITE_END();
