#define BOOST_TEST_MODULE PostgreSQL
#include <boost/test/unit_test.hpp>

#include <dbTypes.h>
#include <output/pq/pqConn.h>
#include <stdexcept>

BOOST_AUTO_TEST_CASE(simple)
{
	BOOST_CHECK_THROW(([]() {
		MyGrate::Output::Pq::PqConn {"nonsense"};
	}()),
			std::runtime_error);
	MyGrate::Output::Pq::PqConn c {"user=postgres"};
	BOOST_CHECK_NO_THROW(c.query("SET application_name = ''"));
	BOOST_CHECK_NO_THROW(c.query("SET application_name = 'something'"));
	BOOST_CHECK_THROW(c.query("SET application_name = "), std::runtime_error);
	// BOOST_CHECK_THROW(c.query("SET application_name = $1", {}), std::logic_error);
	BOOST_CHECK_NO_THROW(c.query("SET application_name = 'something'", {}));
	c.query("DROP TABLE IF EXISTS test");
	c.query("CREATE TABLE test(c text)");
	BOOST_CHECK_NO_THROW(c.query("INSERT INTO test VALUES($1)", {1}));
	BOOST_CHECK_NO_THROW(c.query("INSERT INTO test VALUES($1)", {"string_view"}));
	BOOST_CHECK_NO_THROW(c.query("INSERT INTO test VALUES($1)", {nullptr}));
	BOOST_CHECK_NO_THROW(c.query("INSERT INTO test VALUES($1)", {1.2}));
	BOOST_CHECK_THROW(c.query("INSERT INTO test VALUES($1)", {MyGrate::Time {}}), std::runtime_error);
	c.query("DROP TABLE test");
}
