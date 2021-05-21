#define BOOST_TEST_MODULE BitSet
#include <boost/test/unit_test.hpp>

#include <helpers.h>
#include <stdexcept>

BOOST_AUTO_TEST_CASE(verify)
{
	BOOST_CHECK_NO_THROW(MyGrate::verify<std::runtime_error>(true, "no throw"));
	BOOST_CHECK_THROW(MyGrate::verify<std::runtime_error>(false, "throw re"), std::runtime_error);
	BOOST_CHECK_THROW(MyGrate::verify<std::logic_error>(false, "throw le"), std::logic_error);
}
