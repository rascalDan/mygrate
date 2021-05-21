#define BOOST_TEST_MODULE BitSet
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "helpers.h"
#include <bitset.h>
#include <bitset>

using namespace MyGrate;

// LCOV_EXCL_START
BOOST_TEST_DONT_PRINT_LOG_VALUE(BitSet::Iterator);
// LCOV_EXCL_STOP

BOOST_AUTO_TEST_CASE(bitset)
{
	std::byte bytes[3] {0x00_b, 0xFF_b, 0xa1_b};
	BitSet bs {bytes};

	BOOST_REQUIRE_EQUAL(bs.size(), 24);

	std::bitset<24> exp {0xa1ff00};

	auto iter {bs.begin()};

	for (auto x {0U}; x < 24 && iter != bs.end(); x++) {
		BOOST_TEST_INFO(x);
		BOOST_CHECK_EQUAL(*iter, exp.test(x));
		iter++;
	}

	BOOST_CHECK_EQUAL(iter, bs.end());
}
