#define BOOST_TEST_MODULE MySQL
#include <boost/test/unit_test.hpp>

#include <dbTypes.h>
#include <input/mysqlConn.h>
#include <stdexcept>

BOOST_AUTO_TEST_CASE(simple)
{
	BOOST_CHECK_THROW(([]() {
		MyGrate::Input::MySQLConn {"192.168.1.38", "repl", "repl", 3306};
	}()),
			std::runtime_error);
	MyGrate::Input::MySQLConn c {"192.168.1.38", "repl", "r3pl", 3306};
	BOOST_CHECK_NO_THROW(c.query("SET @var = ''"));
	BOOST_CHECK_NO_THROW(c.query("SET @var = 'something'"));
	BOOST_CHECK_THROW(c.query("SET @var = "), std::runtime_error);
	BOOST_CHECK_THROW(c.query("SET @var = ?", {}), std::logic_error);
	BOOST_CHECK_NO_THROW(c.query("SET @var = ''", {}));
	BOOST_CHECK_NO_THROW(c.query("SET @var = ?", {1}));
	BOOST_CHECK_NO_THROW(c.query("SET @var = ?", {"string_view"}));
	BOOST_CHECK_NO_THROW(c.query("SET @var = ?", {nullptr}));
	BOOST_CHECK_NO_THROW(c.query("SET @var = ?", {1.2}));
	BOOST_CHECK_THROW(c.query("SET @var = ?", {MyGrate::Time {}}), std::runtime_error);
}
