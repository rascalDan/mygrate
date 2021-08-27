#include "mysql_types.h"
#include "helpers.h"
#include "rawDataReader.h"
#include <algorithm>
#include <array>
#include <byteswap.h>
#include <stdexcept>
#include <string>

namespace MyGrate::MySQL {
#define NTOH(B) \
	inline auto ntoh(uint##B##_t x) \
	{ \
		if constexpr (__BYTE_ORDER == __LITTLE_ENDIAN) \
			x = bswap_##B(x); \
		return x; \
	}
	NTOH(32);
	NTOH(64);

	typename Type<MYSQL_TYPE_NULL, false>::C
	Type<MYSQL_TYPE_NULL, false>::read(RawDataReader &, RawDataReader &)
	{
		return nullptr;
	}

#define INTEGER_TYPE(ET) \
	typename Type<ET, false>::C Type<ET, false>::read(RawDataReader &, RawDataReader & data) \
	{ \
		return data.readValue<typename Type<ET, false>::C>(); \
	} \
	typename Type<ET, true>::C Type<ET, true>::read(RawDataReader &, RawDataReader & data) \
	{ \
		return data.readValue<typename Type<ET, true>::C>(); \
	}
	INTEGER_TYPE(MYSQL_TYPE_TINY);
	INTEGER_TYPE(MYSQL_TYPE_SHORT);
	INTEGER_TYPE(MYSQL_TYPE_LONG);
	INTEGER_TYPE(MYSQL_TYPE_LONGLONG);
#undef INTEGER_TYPE

#define FLOAT_TYPE(ET) \
	typename Type<ET, false>::C Type<ET, false>::read(RawDataReader & md, RawDataReader & data) \
	{ \
		verify<std::length_error>(sizeof(typename Type<ET>::C) == md.readValue<uint8_t>(), "Invalid " #ET " size"); \
		return data.readValue<typename Type<ET>::C>(); \
	}
	FLOAT_TYPE(MYSQL_TYPE_FLOAT);
	FLOAT_TYPE(MYSQL_TYPE_DOUBLE);
#undef FLOAT_TYPE

	typename Type<MYSQL_TYPE_DECIMAL>::C
	Type<MYSQL_TYPE_DECIMAL>::read(RawDataReader &, RawDataReader &)
	{
		throw std::logic_error("Not implemented");
	}

	typename Type<MYSQL_TYPE_NEWDECIMAL>::C
	Type<MYSQL_TYPE_NEWDECIMAL>::read(RawDataReader &, RawDataReader &)
	{
		throw std::logic_error("Not implemented");
	}

#define INTEGER_TYPE(ET, s, l) \
	typename Type<ET, s>::C Type<ET, s>::read(RawDataReader &, RawDataReader & data) \
	{ \
		return data.readValue<typename Type<ET, s>::C, l>(); \
	}
	INTEGER_TYPE(MYSQL_TYPE_INT24, false, 3);
	INTEGER_TYPE(MYSQL_TYPE_INT24, true, 3);
#undef INTEGER_TYPE

	typename Type<MYSQL_TYPE_YEAR, false>::C
	Type<MYSQL_TYPE_YEAR, false>::read(RawDataReader &, RawDataReader & data)
	{
		auto y {data.readValue<typename Type<MYSQL_TYPE_YEAR, false>::C, 1>()};
		if (y) {
			y += 1900;
		};
		return y;
	}

	static Blob
	readBlob(RawDataReader & md, RawDataReader & data)
	{
		const auto lenlen {md.readValue<uint8_t>()};
		const auto len {data.readValue<uint64_t>(lenlen)};
		return data.viewValue<Blob>(len);
	}
#define BLOB_TYPE(ET) \
	typename Type<ET, false>::C Type<ET, false>::read(RawDataReader & md, RawDataReader & data) \
	{ \
		return readBlob(md, data); \
	}
	BLOB_TYPE(MYSQL_TYPE_TINY_BLOB);
	BLOB_TYPE(MYSQL_TYPE_MEDIUM_BLOB);
	BLOB_TYPE(MYSQL_TYPE_LONG_BLOB);
	BLOB_TYPE(MYSQL_TYPE_GEOMETRY); // Ummm, pass this to the target to handle?
#undef BLOB_TYPE

	typename Type<MYSQL_TYPE_BLOB, false>::C
	Type<MYSQL_TYPE_BLOB, false>::read(RawDataReader & md, RawDataReader & data)
	{
		const auto lenlen {md.readValue<uint8_t>()};
		const auto len {data.readValue<uint64_t>(lenlen)};
		return data.viewValue<std::string_view>(len);
	}

	typename Type<MYSQL_TYPE_BIT>::C
	Type<MYSQL_TYPE_BIT>::read(RawDataReader &, RawDataReader &)
	{
		throw std::logic_error("Not implemented");
	}

#define STRING_TYPE(ET) \
	typename Type<ET, false>::C Type<ET, false>::read(RawDataReader & md, RawDataReader & data) \
	{ \
		const auto realtype {md.readValue<enum_field_types, 1>()}; \
		const auto len {md.readValue<uint8_t>()}; \
		switch (realtype) { \
			case MYSQL_TYPE_ENUM: \
			case MYSQL_TYPE_VAR_STRING: \
				return data.viewValue<std::string_view>(len); \
			default: \
				throw std::logic_error("Not implemented: sub-type: " + std::to_string(realtype)); \
		} \
		throw std::logic_error("Didn't return a value: " + std::to_string(realtype)); \
	}
	STRING_TYPE(MYSQL_TYPE_STRING);
	STRING_TYPE(MYSQL_TYPE_JSON);
	STRING_TYPE(MYSQL_TYPE_SET);
	STRING_TYPE(MYSQL_TYPE_ENUM);
#undef STRING_TYPE

	typename Type<MYSQL_TYPE_VARCHAR>::C
	Type<MYSQL_TYPE_VARCHAR>::read(RawDataReader & md, RawDataReader & data)
	{
		const auto fieldlen {md.readValue<uint16_t>()};
		const auto len {data.readValue<uint16_t>(fieldlen > 255 ? 2 : 1)};
		return data.viewValue<std::string_view>(len);
	}

	typename Type<MYSQL_TYPE_VAR_STRING>::C
	Type<MYSQL_TYPE_VAR_STRING>::read(RawDataReader & md, RawDataReader & data)
	{
		const auto fieldlen {md.readValue<uint16_t>()};
		const auto len {md.readValue<uint16_t>(fieldlen > 255 ? 2 : 1)};
		return data.viewValue<std::string_view>(len);
	}

	typename Type<MYSQL_TYPE_DATETIME>::C
	Type<MYSQL_TYPE_DATETIME>::read(RawDataReader &, RawDataReader & data)
	{
		auto dtint {data.readValue<uint64_t>()};
		DateTime dt {};
		dt.second = mod100_extract(dtint);
		dt.minute = mod100_extract(dtint);
		dt.hour = mod100_extract(dtint);
		dt.day = mod100_extract(dtint);
		dt.month = mod100_extract(dtint);
		dt.year = boost::numeric_cast<decltype(dt.year)>(dtint);
		return dt;
	}

	typename Type<MYSQL_TYPE_TIME>::C
	Type<MYSQL_TYPE_TIME>::read(RawDataReader &, RawDataReader & data)
	{
		auto tint {data.readValue<uint32_t, 3>()};
		Time t {};
		t.second = mod100_extract(tint);
		t.minute = mod100_extract(tint);
		t.hour = boost::numeric_cast<decltype(t.hour)>(tint);
		return t;
	}

	Time
	time2From24bit(uint32_t tint)
	{
		Time t {};
		t.second = bitslice<6>(tint, 8);
		t.minute = bitslice<6>(tint, 14);
		t.hour = bitslice<5>(tint, 20);
		return t;
	}

	typename Type<MYSQL_TYPE_TIME2>::C
	Type<MYSQL_TYPE_TIME2>::read(RawDataReader & md, RawDataReader & data)
	{
		md.discard(1);
		return time2From24bit(ntoh(data.readValue<uint32_t>(3)));
	}

	typename Type<MYSQL_TYPE_DATE>::C
	Type<MYSQL_TYPE_DATE>::read(RawDataReader &, RawDataReader & data)
	{
		const auto dint {data.readValue<uint32_t, 3>()};
		Date d {};
		d.day = bitslice<5>(dint, 0);
		d.month = bitslice<4>(dint, 5);
		d.year = bitslice<14>(dint, 9);
		return d;
	}

	typename Type<MYSQL_TYPE_NEWDATE>::C
	Type<MYSQL_TYPE_NEWDATE>::read(RawDataReader &, RawDataReader &)
	{
		throw std::logic_error("Not implemented");
	}

	typename Type<MYSQL_TYPE_TIMESTAMP>::C
	Type<MYSQL_TYPE_TIMESTAMP>::read(RawDataReader &, RawDataReader & data)
	{
		return {data.readValue<int32_t>(), 0};
	}

	typename Type<MYSQL_TYPE_TIMESTAMP2>::C
	Type<MYSQL_TYPE_TIMESTAMP2>::read(RawDataReader & md, RawDataReader & data)
	{
		const auto decimals {md.readValue<uint8_t>()};
		return {data.readValue<int32_t>(), data.readValue<long>(decimals)};
	}

	typename Type<MYSQL_TYPE_DATETIME2>::C
	Type<MYSQL_TYPE_DATETIME2>::read(RawDataReader & md, RawDataReader & data)
	{
		md.discard(1);
		return datetime2From40bit(ntoh(data.readValue<uint64_t>(5)));
	}

	DateTime
	datetime2From40bit(uint64_t dtint)
	{
		DateTime dt;
		dt.year = boost::numeric_cast<decltype(dt.year)>(bitslice<17>(dtint, 46) / 13);
		dt.month = boost::numeric_cast<decltype(dt.month)>(bitslice<17>(dtint, 46) % 13);
		dt.day = bitslice<5>(dtint, 41);
		dt.hour = bitslice<5>(dtint, 36);
		dt.minute = bitslice<6>(dtint, 30);
		dt.second = bitslice<6>(dtint, 24);
		return dt;
	}
}
