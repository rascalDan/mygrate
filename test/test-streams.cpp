#define BOOST_TEST_MODULE StreamSupport
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "bitset.h"
#include "dbTypes.h"
#include "helpers.h"
#include "mariadb_repl.h"
#include <array>
#include <cstddef>
#include <ctime>
#include <iosfwd>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
struct timespec;
struct tm;
#include <streamSupport.h>

BOOST_FIXTURE_TEST_SUITE(stream, std::stringstream);

template<typename In> using ToStream = std::tuple<In, std::string_view>;
BOOST_DATA_TEST_CASE(bytes,
		boost::unit_test::data::make<ToStream<std::byte>>({
				//{0x00_b, "0x00"},
				{0x10_b, "0x10"},
				{0xFE_b, "0xfe"},
				{0xff_b, "0xff"},
		}),
		in, exp)
{
	*this << in;
	BOOST_CHECK_EQUAL(this->str(), exp);
}

BOOST_DATA_TEST_CASE(tms,
		boost::unit_test::data::make<ToStream<tm>>({
				{make_tm(2016, 1, 4, 12, 13, 14), "2016-01-04 12:13:14"},
				{make_tm(2016, 12, 31, 0, 0, 1), "2016-12-31 00:00:01"},
		}),
		in, exp)
{
	*this << in;
	BOOST_CHECK_EQUAL(this->str(), exp);
}

BOOST_DATA_TEST_CASE(rts,
		boost::unit_test::data::make<ToStream<MyGrate::DateTime>>({
				{{{2016, 1, 4}, {12, 13, 14}}, "2016-01-04 12:13:14"},
				{{{2016, 12, 31}, {0, 0, 1}}, "2016-12-31 00:00:01"},
		}),
		in, exp)
{
	*this << in;
	BOOST_CHECK_EQUAL(this->str(), exp);
}

BOOST_DATA_TEST_CASE(tss,
		boost::unit_test::data::make<ToStream<timespec>>({
				{{0, 0}, "0.000000000"},
				{{0, 1}, "0.000000001"},
				{{1, 0}, "1.000000000"},
				{{123, 0}, "123.000000000"},
				{{123, 999999999}, "123.999999999"},
		}),
		in, exp)
{
	*this << in;
	BOOST_CHECK_EQUAL(this->str(), exp);
}
constexpr std::array<std::byte, 3> somebits {0x00_b, 0xFF_b, 0xa1_b};
BOOST_DATA_TEST_CASE(bss,
		boost::unit_test::data::make<ToStream<MyGrate::BitSet>>({
				{{MyGrate::BitSet {somebits}}, "[0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,1,0,1]"},
		}),
		in, exp)
{
	*this << in;
	BOOST_CHECK_EQUAL(this->str(), exp);
}

BOOST_AUTO_TEST_CASE(mariadb_string)
{
	std::string buf {"test"};
	MARIADB_STRING str {buf.data(), buf.length()};
	*this << str;
	BOOST_CHECK_EQUAL(this->str(), buf);
}

BOOST_AUTO_TEST_CASE(array)
{
	std::array<int, 3> arr {1, 123456, -78910};
	*this << arr;
	BOOST_CHECK_EQUAL(this->str(), "[1,123456,-78910]");
}

BOOST_AUTO_TEST_CASE(vector)
{
	std::vector<int> arr {1, 123456, -78910};
	*this << arr;
	BOOST_CHECK_EQUAL(this->str(), "[1,123456,-78910]");
}

BOOST_AUTO_TEST_SUITE_END();
