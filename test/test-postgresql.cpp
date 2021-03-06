#define BOOST_TEST_MODULE PostgreSQL
#include <boost/mpl/list.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "helpers.h"
#include "testdb-postgresql.h"
#include <cstddef>
#include <dbConn.h>
#include <dbRecordSet.h>
#include <dbStmt.h>
#include <dbTypes.h>
#include <fixedString.h>
#include <helpers.h>
#include <memory>
#include <output/pq/pqConn.h>
#include <output/pq/writePqCopyStrm.h>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <variant>

const auto CONNSTR {MyGrate::getenv("MYGRATE_POSTGRESQL_CONNSTR", "user=postgres")};

BOOST_AUTO_TEST_CASE(simple)
{
	BOOST_CHECK_THROW(([]() {
		MyGrate::Output::Pq::PqConn {"nonsense"};
	}()),
			MyGrate::Output::Pq::PqErr);
	MyGrate::Output::Pq::PqConn c {CONNSTR};
	BOOST_CHECK_NO_THROW(c.query("SET application_name = ''"));
	BOOST_CHECK_NO_THROW(c.query("SET application_name = 'something'"));
	BOOST_CHECK_THROW(c.query("SET application_name = "), MyGrate::Output::Pq::PqErr);
	// BOOST_CHECK_THROW(c.query("SET application_name = $1", {}), std::logic_error);
	BOOST_CHECK_NO_THROW(c.query("SET application_name = 'something'", {}));
}

using SomeUpdate = MyGrate::DbStmt<"UPDATE foo SET blah = $2 WHERE bar = $1">;
using SomeUpdateRtn = MyGrate::DbStmt<"UPDATE foo SET blah = $2 WHERE bar = $1 RETURNING x">;
using SomeShow = MyGrate::DbStmt<"SHOW all">;
using SomeIns = MyGrate::DbStmt<"INSERT INTO foo VALUES(..., $87)">;

static_assert(SomeUpdate::paramCount(MyGrate::ParamMode::DollarNum) == 2);
static_assert(std::is_same_v<SomeUpdate::Return, std::size_t>);
static_assert(SomeUpdateRtn::paramCount(MyGrate::ParamMode::DollarNum) == 2);
static_assert(std::is_same_v<SomeUpdateRtn::Return, MyGrate::RecordSetPtr>);
static_assert(SomeShow::paramCount(MyGrate::ParamMode::DollarNum) == 0);
static_assert(SomeIns::paramCount(MyGrate::ParamMode::DollarNum) == 87);
static_assert(std::is_same_v<SomeIns::Return, std::size_t>);

BOOST_AUTO_TEST_CASE(stmt)
{
	MyGrate::Output::Pq::PqConn c {CONNSTR};
	const auto rs {SomeShow::execute(&c)};
	BOOST_REQUIRE(rs);
	BOOST_REQUIRE_EQUAL(rs->columns(), 3);
	BOOST_REQUIRE_GT(rs->rows(), 1);
	BOOST_CHECK_EQUAL(std::get<std::string_view>(rs->at(0, 0)), "allow_system_table_mods");
	BOOST_CHECK_EQUAL(std::get<std::string_view>(rs->at(0, 1)), "off");
}

BOOST_AUTO_TEST_CASE(stmt_reuse)
{
	MyGrate::Output::Pq::PqConn c {CONNSTR};
	for (int x = 0; x < 10; x++) {
		const auto rs {SomeShow::execute(&c)};
		BOOST_REQUIRE(rs);
		BOOST_CHECK_EQUAL(rs->columns(), 3);
		BOOST_CHECK_GT(rs->rows(), 1);
	}
}

BOOST_AUTO_TEST_CASE(mock)
{
	MyGrate::Testing::PqConnDB db;
	auto mdb = db.mock();
	auto rs = MyGrate::DbStmt<"SELECT CURRENT_DATABASE()">::execute(&mdb);
	BOOST_CHECK_EQUAL(rs->at(0, 0).get<std::string_view>().substr(0, 13), "mygrate_test_");

	mdb.query("CREATE TABLE test(c text)");
	BOOST_CHECK_NO_THROW(mdb.query("INSERT INTO test VALUES($1)", {1}));
	BOOST_CHECK_NO_THROW(mdb.query("INSERT INTO test VALUES($1)", {"string_view"}));
	BOOST_CHECK_NO_THROW(mdb.query("INSERT INTO test VALUES($1)", {nullptr}));
	BOOST_CHECK_NO_THROW(mdb.query("INSERT INTO test VALUES($1)", {1.2}));
	BOOST_CHECK_THROW(mdb.query("INSERT INTO test VALUES($1)", {timespec {}}), std::logic_error);
	auto rscount = MyGrate::DbStmt<"SELECT COUNT(*) FROM test">::execute(&mdb);
	BOOST_CHECK_EQUAL(rscount->at(0, 0).operator unsigned int(), 4);
}

BOOST_AUTO_TEST_CASE(mock_schema)
{
	MyGrate::Testing::PqConnDB db {ROOT "/db/schema.sql"};
	auto mdb = db.mock();
	auto rs = MyGrate::DbStmt<"SELECT COUNT(*) FROM mygrate.source">::execute(&mdb);
	BOOST_CHECK_EQUAL(rs->at(0, 0).operator unsigned int(), 0);
}

BOOST_FIXTURE_TEST_SUITE(ms, MemStream)

BOOST_DATA_TEST_CASE(write_strings,
		boost::unit_test::data::make({
				std::make_tuple("", ""),
				{"simple", "simple"},
				{"simple with spaces", "simple with spaces"},
				{"simple\twith\ttabs", "simple\\011with\\011tabs"},
				{"\ttab start", "\\011tab start"},
				{"tab end\t", "tab end\\011"},
				{"tab\t\t\t\tmany", "tab\\011\\011\\011\\011many"},
		}),
		in, exp)
{
	MyGrate::Output::Pq::WritePqCopyStream c {s};
	c(in);
	flush();

	BOOST_REQUIRE(out);
	BOOST_CHECK_EQUAL(out, exp);
}

using IntTypes = boost::mpl::list<int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t>;
BOOST_AUTO_TEST_CASE_TEMPLATE(write_ints, T, IntTypes)
{
	MyGrate::Output::Pq::WritePqCopyStream c {s};
	c(T {1});
	flush();

	BOOST_REQUIRE(out);
	BOOST_CHECK_EQUAL(len, 1);
	BOOST_CHECK_EQUAL(out, "1");
}

using FloatTypes = boost::mpl::list<float, double>;
BOOST_AUTO_TEST_CASE_TEMPLATE(write_floats, T, FloatTypes)
{
	MyGrate::Output::Pq::WritePqCopyStream c {s};
	c(T {static_cast<T>(1.1F)});
	flush();

	BOOST_REQUIRE(out);
	BOOST_CHECK_EQUAL(len, 3);
	BOOST_CHECK_EQUAL(out, "1.1");
}

BOOST_AUTO_TEST_CASE(write_blob)
{
	MyGrate::Output::Pq::WritePqCopyStream c {s};
	std::array<std::byte, 10> b {0x00_b, 0x10_b, 0x12_b, 0x30_b, 0x90_b, 0xaa_b, 0xff_b};
	c(b);
	flush();

	BOOST_REQUIRE(out);
	BOOST_CHECK_EQUAL(len, 23);
	BOOST_CHECK_EQUAL(out, R"B(\\x0010123090AAFF000000)B");
}

BOOST_AUTO_TEST_SUITE_END()
