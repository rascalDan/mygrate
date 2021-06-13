#define BOOST_TEST_MODULE MySQL
#include <boost/test/unit_test.hpp>

#include "testdb-mysql.h"
#include <cstdint>
#include <cstdlib>
#include <dbConn.h>
#include <dbRecordSet.h>
#include <dbStmt.h>
#include <dbTypes.h>
#include <fixedString.h>
#include <helpers.h>
#include <input/mysqlConn.h>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <variant>

const auto SERVER {MyGrate::getenv("MYGRATE_MYSQL_SERVER", "localhost")};
const auto USER {MyGrate::getenv("MYGRATE_MYSQL_USER", getenv("LOGNAME"))};
const auto PASSWORD {getenv("MYGRATE_MYSQL_PASSWORD")};
const auto PORT {(unsigned short)std::atoi(MyGrate::getenv("MYGRATE_MYSQL_PORT", "3306"))};

BOOST_AUTO_TEST_CASE(simple)
{
	BOOST_CHECK_THROW(([]() {
		MyGrate::Input::MySQLConn {SERVER, USER, "badpass", PORT};
	}()),
			MyGrate::Input::MySQLErr);
	MyGrate::Input::MySQLConn c {SERVER, USER, PASSWORD, PORT};
	BOOST_CHECK_NO_THROW(c.query("SET @var = ''"));
	BOOST_CHECK_NO_THROW(c.query("SET @var = 'something'"));
	BOOST_CHECK_THROW(c.query("SET @var = "), MyGrate::Input::MySQLErr);
	BOOST_CHECK_THROW(c.query("SET @var = ?", {}), std::logic_error);
	BOOST_CHECK_NO_THROW(c.query("SET @var = ''", {}));
	BOOST_CHECK_NO_THROW(c.query("SET @var = ?", {1}));
	BOOST_CHECK_NO_THROW(c.query("SET @var = ?", {"string_view"}));
	BOOST_CHECK_NO_THROW(c.query("SET @var = ?", {nullptr}));
	BOOST_CHECK_NO_THROW(c.query("SET @var = ?", {1.2}));
	BOOST_CHECK_THROW(c.query("SET @var = ?", {MyGrate::Time {}}), std::logic_error);
}

using SomeSelect = MyGrate::DbStmt<"SELECT * FROM foo">;
using SomeShow = MyGrate::DbStmt<"SHOW MASTER STATUS">;
using SomeUpdate = MyGrate::DbStmt<"UPDATE foo SET blah = ? WHERE bar = ?">;

static_assert(std::is_same_v<SomeSelect::Return, MyGrate::RecordSetPtr>);
static_assert(std::is_same_v<SomeShow::Return, MyGrate::RecordSetPtr>);
static_assert(std::is_same_v<SomeUpdate::Return, std::size_t>);
static_assert(SomeShow::paramCount(MyGrate::ParamMode::QMark) == 0);
static_assert(SomeUpdate::paramCount(MyGrate::ParamMode::QMark) == 2);

BOOST_AUTO_TEST_CASE(stmt)
{
	MyGrate::Input::MySQLConn c {SERVER, USER, PASSWORD, PORT};
	const auto rs {SomeShow::execute(&c)};
	BOOST_REQUIRE(rs);
	BOOST_REQUIRE_EQUAL(rs->rows(), 1);
	BOOST_REQUIRE_EQUAL(rs->columns(), 4);
	BOOST_CHECK(std::get<std::string_view>(rs->at(0, 0)).starts_with("mariadb"));
	BOOST_CHECK_GE(std::get<int64_t>(rs->at(0, 1)), 4);
}

BOOST_AUTO_TEST_CASE(mock)
{
	MyGrate::Testing::MySQLDB db;
	auto mdb = db.mock();
	auto rs = MyGrate::DbStmt<"SELECT DATABASE()">::execute(&mdb);
	BOOST_CHECK_EQUAL(rs->at(0, 0).get<std::string_view>().substr(0, 13), "mygrate_test_");
}
