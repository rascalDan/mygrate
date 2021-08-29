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
#include <streamSupport.h>
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
	virtual ~MockSetup()
	{
		if (src) {
			src->stopEvents();
		}
		if (repl) {
			repl->join();
			repl.reset();
		}
	}

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
		repl.emplace([&]() {
			try {
				getSource()->readEvents(*out);
			}
			catch (std::exception & ex) {
				failed = true;
				std::cerr << ex.what() << "\n";
			}
			catch (...) {
				failed = true;
				std::cerr << "bug\n";
			}
		});
	}

	void
	stopAfter(const EventCounterTarget & events)
	{
		BOOST_REQUIRE(out);
		BOOST_REQUIRE(src);
		BOOST_REQUIRE(repl);
		if (!failed) {
			out->waitFor(events);
		}
		src->stopEvents();
		repl->join();
		repl.reset();
		if (failed) {
			throw std::runtime_error("Replication thread failed");
		}
	}

	MySQLDB my;
	PqConnDB pq;
	MyGrate::Output::Pq::PqConn pqm;
	MyGrate::Input::MySQLConn mym;
	bool failed {false};

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

template<int T> struct TypeTestDetail;
#define TEST_TYPE(MYSQL_TYPE, IN, OUT, MYTYPE) \
	template<> struct TypeTestDetail<MYSQL_TYPE> { \
		using OutType = OUT; \
		static constexpr auto mytype = #MYTYPE; \
		static IN generate(size_t n); \
	}; \
	IN TypeTestDetail<MYSQL_TYPE>::generate(size_t n)

TEST_TYPE(MYSQL_TYPE_STRING, std::string, std::string_view, text)
{
	return std::string(n * 200, 'f');
}
TEST_TYPE(MYSQL_TYPE_VARCHAR, std::string, std::string_view, varchar(2048))
{
	return std::string(n * 20, 'f');
}
TEST_TYPE(MYSQL_TYPE_JSON, std::string, std::string_view, json)
{
	return MyGrate::scprintf<"{ json: %? }">(n);
}
TEST_TYPE(MYSQL_TYPE_ENUM, std::string_view, std::string_view, enum('alpha', 'beta', 'gamma'))
{
	static constexpr std::array<std::string_view, 3> vals {"alpha", "beta", "gamma"};
	return vals[n % vals.size()];
}
TEST_TYPE(MYSQL_TYPE_TINY, int8_t, int8_t, tinyint)
{
	return static_cast<int8_t>(n);
}
TEST_TYPE(MYSQL_TYPE_SHORT, int16_t, int16_t, smallint)
{
	return static_cast<int16_t>(n);
}
TEST_TYPE(MYSQL_TYPE_INT24, int32_t, int32_t, int)
{
	return static_cast<int32_t>(n);
}
TEST_TYPE(MYSQL_TYPE_YEAR, int16_t, int16_t, year)
{
	if (!n) {
		return 0;
	}
	return static_cast<int16_t>(n + 1901);
}
TEST_TYPE(MYSQL_TYPE_LONG, int32_t, int32_t, int)
{
	return static_cast<int32_t>(n);
}
TEST_TYPE(MYSQL_TYPE_LONGLONG, int64_t, int64_t, bigint)
{
	return static_cast<int64_t>(n);
}
TEST_TYPE(MYSQL_TYPE_FLOAT, float, float, float)
{
	return static_cast<float>(n);
}
TEST_TYPE(MYSQL_TYPE_DOUBLE, double, double, real)
{
	return static_cast<double>(n);
}
static struct tm
test_tm(size_t n)
{
	time_t t = time(nullptr);
	t -= static_cast<time_t>(n * 12345679);
	struct tm tm {
	};
	gmtime_r(&t, &tm);
	return tm;
}
TEST_TYPE(MYSQL_TYPE_DATETIME, MyGrate::DateTime, MyGrate::DateTime, datetime)
{
	return MyGrate::DateTime {test_tm(n)};
}
TEST_TYPE(MYSQL_TYPE_DATE, MyGrate::Date, MyGrate::Date, date)
{
	return MyGrate::Date {test_tm(n)};
}
TEST_TYPE(MYSQL_TYPE_TIME, MyGrate::Time, MyGrate::Time, time)
{
	return MyGrate::Time {test_tm(n)};
}

template<int MYSQL_TYPE, typename Test>
void
replication_data_type_impl(Test * test)
{
	using T = TypeTestDetail<MYSQL_TYPE>;
	using I = decltype(T::generate(0));
	using O = typename T::OutType;

	constexpr auto ROWS {100U};
	BOOST_TEST_INFO(T::mytype);
	test->mym.query(
			MyGrate::scprintf<"CREATE TABLE test(id INT AUTO_INCREMENT, val %?, PRIMARY KEY(id))">(T::mytype).c_str());
	TestUpdateDatabase & out {test->getUpdateDatabase()};
	out.addTable(&test->mym, "test");

	std::vector<I> vals;
	vals.reserve(ROWS);

	// insert test records
	test->run();
	MyGrate::Tx {&test->mym}([&vals, test] {
		MyGrate::DbStmt<"INSERT INTO test(val) VALUES(?)"> ins;
		for (size_t n {}; n < ROWS; n++) {
			vals.push_back(T::generate(n));
			ins.execute(&test->mym, vals.back());
		}
	});
	test->stopAfter(EventCounterTarget {}.add(WRITE_ROWS_EVENT_V1, ROWS));

	// read test records
	auto rs {MyGrate::DbStmt<"SELECT val FROM testout.test ORDER BY id">::execute(&test->pqm)};
	std::vector<O> outs;
	outs.reserve(ROWS);
	for (auto v : *rs) {
		outs.push_back(v[0]);
	}

	// Check values
	BOOST_CHECK_EQUAL_COLLECTIONS(vals.begin(), vals.end(), outs.begin(), outs.end());
}

#define TEST_MYSQL_TYPE(T) \
	BOOST_AUTO_TEST_CASE(replication_data_type_##T, *boost::unit_test::timeout(5)) \
	{ \
		replication_data_type_impl<T>(this); \
	}

TEST_MYSQL_TYPE(MYSQL_TYPE_STRING);
TEST_MYSQL_TYPE(MYSQL_TYPE_VARCHAR);
TEST_MYSQL_TYPE(MYSQL_TYPE_JSON);
// TEST_MYSQL_TYPE(MYSQL_TYPE_ENUM); // we don't have sufficient data available to support this
TEST_MYSQL_TYPE(MYSQL_TYPE_TINY);
TEST_MYSQL_TYPE(MYSQL_TYPE_SHORT);
TEST_MYSQL_TYPE(MYSQL_TYPE_LONG);
TEST_MYSQL_TYPE(MYSQL_TYPE_INT24);
TEST_MYSQL_TYPE(MYSQL_TYPE_LONGLONG);
TEST_MYSQL_TYPE(MYSQL_TYPE_FLOAT);
TEST_MYSQL_TYPE(MYSQL_TYPE_DOUBLE);
TEST_MYSQL_TYPE(MYSQL_TYPE_YEAR);
TEST_MYSQL_TYPE(MYSQL_TYPE_DATETIME);
TEST_MYSQL_TYPE(MYSQL_TYPE_DATE);
TEST_MYSQL_TYPE(MYSQL_TYPE_TIME);

BOOST_AUTO_TEST_SUITE_END();
