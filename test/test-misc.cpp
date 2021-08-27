#define BOOST_TEST_MODULE Misc
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <cstddef>
#include <cstdint>
#include <dbTypes.h>
#include <helpers.h>
#include <mysql_types.h>
#include <stdexcept>
#include <streamSupport.h>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
namespace MyGrate {
	class BitSet;
}
namespace boost::numeric {
	class bad_numeric_cast;
}
struct timespec;

BOOST_AUTO_TEST_CASE(verify)
{
	BOOST_CHECK_NO_THROW(MyGrate::verify<std::runtime_error>(true, "no throw"));
	BOOST_CHECK_THROW(MyGrate::verify<std::runtime_error>(false, "throw re"), std::runtime_error);
	BOOST_CHECK_THROW(MyGrate::verify<std::logic_error>(false, "throw le"), std::logic_error);
}

using Ints = std::tuple<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t>;
using Floats = std::tuple<float, double>;
using Times = std::tuple<timespec, MyGrate::Date, MyGrate::Time, MyGrate::DateTime>;
using Str = std::tuple<std::string_view>;
using Others = std::tuple<std::nullptr_t, MyGrate::BitSet, MyGrate::Blob>;
using TinyInts = std::tuple<int8_t, uint8_t>;
using SmallInts = std::tuple<int8_t, uint8_t, int16_t, uint16_t>;

BOOST_AUTO_TEST_CASE_TEMPLATE(DbValueConvIntToInts, I, Ints)
{
	MyGrate::DbValue v {123};
	I out {v};
	BOOST_CHECK_EQUAL(123, out);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(DbValueConvIntToTinyInts, I, TinyInts)
{
	MyGrate::DbValue v {1234};
	BOOST_CHECK_THROW([[maybe_unused]] I out {v}, boost::bad_numeric_cast);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(DbValueConvIntToSmallInts, I, SmallInts)
{
	MyGrate::DbValue v {123400};
	BOOST_CHECK_THROW([[maybe_unused]] I out {v}, boost::bad_numeric_cast);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(DbValueConvIntToFloats, F, Floats)
{
	MyGrate::DbValue v {123400};
	BOOST_CHECK_THROW([[maybe_unused]] F out {v}, std::logic_error);
}

BOOST_AUTO_TEST_CASE(DbValueConvIntToStringView)
{
	MyGrate::DbValue v {123};
	BOOST_CHECK_THROW([[maybe_unused]] std::string_view out {v}, std::bad_variant_access);
}

BOOST_AUTO_TEST_CASE(DbValueConvStrViewToStringView)
{
	using namespace std::literals;
	MyGrate::DbValue v {"str"};
	BOOST_CHECK_EQUAL((std::string_view)v, "str"sv);
	BOOST_CHECK_EQUAL((std::string)v, "str"s);
}

static_assert(MyGrate::detail::HasToString<int>);
static_assert(!MyGrate::detail::HasToString<MyGrate::Date>);

BOOST_AUTO_TEST_CASE_TEMPLATE(DbValueConvIntToString, I, Ints)
{
	using namespace std::literals;
	MyGrate::DbValue v {I {123}};
	BOOST_CHECK_EQUAL((std::string)v, "123"s);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(DbValueConvFloatToString, F, Floats)
{
	using namespace std::literals;
	MyGrate::DbValue v {F {123}};
	BOOST_CHECK_EQUAL((std::string)v, "123.000000"s);
}

BOOST_AUTO_TEST_CASE(create_datetime)
{
	struct tm tm;
	time_t t {1629222289};
	gmtime_r(&t, &tm);
	BOOST_REQUIRE_EQUAL(tm.tm_gmtoff, 0);
	BOOST_REQUIRE_EQUAL(tm.tm_isdst, 0);
	MyGrate::DateTime dt {tm};
	BOOST_CHECK_EQUAL(dt, (MyGrate::DateTime {2021, 8, 17, 17, 44, 49}));
}

BOOST_AUTO_TEST_CASE(mod100_extract)
{
	long unsigned int i {1629222289};
	BOOST_CHECK_EQUAL((int)MyGrate::mod100_extract(i), 89);
	BOOST_CHECK_EQUAL((int)MyGrate::mod100_extract(i), 22);
	BOOST_CHECK_EQUAL((int)MyGrate::mod100_extract(i), 22);
	BOOST_CHECK_EQUAL((int)MyGrate::mod100_extract(i), 29);
	BOOST_CHECK_EQUAL((int)MyGrate::mod100_extract(i), 16);
	BOOST_CHECK_EQUAL((int)MyGrate::mod100_extract(i), 0);
	BOOST_CHECK_EQUAL((int)MyGrate::mod100_extract(i), 0);
	BOOST_CHECK_EQUAL((int)MyGrate::mod100_extract(i), 0);
}

using ConvertTimeData = std::tuple<uint32_t, MyGrate::Time>;
BOOST_DATA_TEST_CASE(convert_time,
		boost::unit_test::data::make<ConvertTimeData>({
				{0x8128cc00, {18, 35, 12}},
				{0x81537500, {21, 13, 53}},
				{0x817d2200, {23, 52, 34}},
				{0x8027cf00, {2, 31, 15}},
				{0x80527800, {5, 9, 56}},
				{0x807c2500, {7, 48, 37}},
				{0x80a6d200, {10, 27, 18}},
				{0x80d17b00, {13, 5, 59}},
				{0x80fb2800, {15, 44, 40}},
				{0x8125d500, {18, 23, 21}},
		}),
		tint, time)
{
	BOOST_CHECK_EQUAL(MyGrate::MySQL::time2From24bit(tint), time);
}
