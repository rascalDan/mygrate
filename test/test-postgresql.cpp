#define BOOST_TEST_MODULE PostgreSQL
#include <boost/test/unit_test.hpp>

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
	c.query("DROP TABLE IF EXISTS test");
	c.query("CREATE TABLE test(c text)");
	BOOST_CHECK_NO_THROW(c.query("INSERT INTO test VALUES($1)", {1}));
	BOOST_CHECK_NO_THROW(c.query("INSERT INTO test VALUES($1)", {"string_view"}));
	BOOST_CHECK_NO_THROW(c.query("INSERT INTO test VALUES($1)", {nullptr}));
	BOOST_CHECK_NO_THROW(c.query("INSERT INTO test VALUES($1)", {1.2}));
	BOOST_CHECK_THROW(c.query("INSERT INTO test VALUES($1)", {MyGrate::Time {}}), std::logic_error);
	c.query("DROP TABLE test");
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
}

BOOST_AUTO_TEST_CASE(mock_schema)
{
	MyGrate::Testing::PqConnDB db {ROOT "/db/schema.sql"};
	auto mdb = db.mock();
	auto rs = MyGrate::DbStmt<"SELECT COUNT(*) FROM mygrate.source">::execute(&mdb);
	BOOST_CHECK_EQUAL(rs->at(0, 0).operator unsigned int(), 0);
}
