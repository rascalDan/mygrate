#define BOOST_TEST_MODULE RawDataReader
#include <boost/mpl/list.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "bitset.h"
#include "dbTypes.h"
#include "helpers.h"
#include "mariadb_repl.h"
#include <cstddef>
#include <cstdint>
#include <mysql.h>
#include <rawDataReader.h>
#include <stdexcept>
#include <streamSupport.h>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

using namespace MyGrate;

using Bytes = std::vector<uint8_t>;
template<typename T> using BytesTo = std::tuple<Bytes, T>;

BOOST_DATA_TEST_CASE(read_packedinteger,
		boost::unit_test::data::make<BytesTo<uint64_t>>({
				{{0x00, 0xb2, 0x2, 0}, 0},
				{{0x01, 0xb2, 0x2, 0}, 1},
				{{0xfa, 0xb2, 0x2, 0}, 0xfa},
				{{0xfc, 0xb2, 0x2, 0}, 0x02b2},
				{{0xfd, 0xb2, 0x2, 0}, 690},
				{{0xfe, 0xb2, 0x2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 690},
		}),
		bytes, out)
{
	RawDataReader rdr {bytes.data(), bytes.size()};
	BOOST_CHECK_EQUAL((rdr.readValue<PackedInteger>()), out);
}

BOOST_DATA_TEST_CASE(invalid_packed_ints,
		boost::unit_test::data::make<Bytes>({
				{0xFF, 0x78, 0x38, 0x1a, 0x0b},
				{0xFB, 0x78, 0x38, 0x1a, 0x0b},
		}),
		bytes)
{
	RawDataReader rdr {bytes.data(), bytes.size()};
	BOOST_CHECK_THROW(rdr.readValue<PackedInteger>(), std::domain_error);
}

BOOST_DATA_TEST_CASE(read_overflow,
		boost::unit_test::data::make<Bytes>({
				{0x00, 0xFB, 0x12, 0x00},
		}),
		bytes)
{
	RawDataReader rdr {bytes.data(), bytes.size()};
	rdr.discard(1);
	BOOST_CHECK_EQUAL(rdr.readValue<uint16_t>(), 0x12FB);
	BOOST_CHECK_THROW(rdr.readValue<uint16_t>(), std::range_error);
}

/*
BOOST_DATA_TEST_CASE(read_overflow_string,
		boost::unit_test::data::make<Bytes>({
				{0x04, 'a', 'b', 'c'},
		}),
		bytes)
{
	RawDataReader rdr {bytes.data(), bytes.size()};
	// BOOST_CHECK_THROW(rdr.readValue<VarChar<1>>(), std::range_error);
}

BOOST_DATA_TEST_CASE(read_datetime2,
		boost::unit_test::data::make<BytesTo<struct tm>>({
				{{0x99, 0x78, 0x38, 0x1a, 0x0b}, make_tm(2006, 2, 28, 1, 40, 11)},
		}),
		bytes, exp)
{
	RawDataReader rdr {bytes.data(), bytes.size()};
	(void)exp;
	// BOOST_CHECK_EQUAL((rdr.readValue<DateTime2>()), out);
}

BOOST_DATA_TEST_CASE(read_varchars1,
		boost::unit_test::data::make<BytesTo<std::string_view>>({
				{{0x00}, ""},
				{{0x01, 'A'}, "A"},
				{{0x03, 'a', 'b', 'c'}, "abc"},
				{{0x10, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'},
						"0123456789abcdef"},
		}),
		bytes, exp)
{
	RawDataReader rdr {bytes.data(), bytes.size()};
	(void)exp;
	// BOOST_CHECK_EQUAL((rdr.readValue<VarChar<1>>()), exp);
}

BOOST_DATA_TEST_CASE(read_varchars2,
		boost::unit_test::data::make<BytesTo<std::string_view>>({
				{{0x00, 0x00}, ""},
				{{0x01, 0x00, 'A'}, "A"},
				{{0x03, 0x00, 'a', 'b', 'c'}, "abc"},
				{{0x10, 0x00, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'},
						"0123456789abcdef"},
		}),
		bytes, exp)
{
	RawDataReader rdr {bytes.data(), bytes.size()};
	(void)exp;
	// BOOST_CHECK_EQUAL((rdr.readValue<VarChar<2>>()), exp);
}
*/

BOOST_DATA_TEST_CASE(read_bytes,
		boost::unit_test::data::make<Bytes>({
				{},
				{0x01},
				{0x00, 0x01, 0x02},
				{0xFF, 0xFE, 0xFD},
				{0xFF, 0xFE, 0xFD, 0x00, 0x01, 0x02},
		}),
		bytes)
{
	RawDataReader rdr {bytes.data(), bytes.size()};
	const auto out {rdr.viewValue<MyGrate::Blob>(bytes.size())};
	BOOST_CHECK_EQUAL_COLLECTIONS(out.begin(), out.end(), bytes.begin(), bytes.end());
}

BOOST_DATA_TEST_CASE(read_bytes_overflow,
		boost::unit_test::data::make<Bytes>({
				{},
				{0x01},
				{0x00, 0x01, 0x02},
				{0xFF, 0xFE, 0xFD},
				{0xFF, 0xFE, 0xFD, 0x00, 0x01, 0x02},
		}),
		bytes)
{
	RawDataReader rdr {bytes.data(), bytes.size()};
	BOOST_CHECK_THROW(rdr.viewValue<MyGrate::Blob>(bytes.size() + 1), std::range_error);
}

BOOST_DATA_TEST_CASE(read_field_type,
		boost::unit_test::data::make<BytesTo<enum_field_types>>({
				{{0x00}, MYSQL_TYPE_DECIMAL},
				{{0x04}, MYSQL_TYPE_FLOAT},
				{{0xFF}, MYSQL_TYPE_GEOMETRY},
		}),
		bytes, exp)
{
	RawDataReader rdr {bytes.data(), bytes.size()};
	BOOST_CHECK_EQUAL(rdr.readValue<enum_field_types>(1), exp);
}

BOOST_AUTO_TEST_CASE(rdr_from_MARIADB_STRING)
{
	std::string buf {"test"};
	MARIADB_STRING str {buf.data(), buf.length()};
	RawDataReader rdr {str};
	BOOST_CHECK_EQUAL(rdr.viewValue<std::string_view>(buf.length()), buf);
}

using SimpleTypes = boost::mpl::list<std::byte, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t,
		uint64_t, float, double>;

BOOST_AUTO_TEST_CASE_TEMPLATE(rdr_read_simple, T, SimpleTypes)
{
	RawDataReader rdr {"don't care, some bytes", 20};
	rdr.readValue<T>();
}
