#ifndef MYGRATE_MYSQL_TYPES_H
#define MYGRATE_MYSQL_TYPES_H

#include "bitset.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <mysql.h> // IWYU pragma: keep
#include <span>
#include <string_view>
#include <variant>

#include <mariadb_rpl.h>

namespace MyGrate {
	class RawDataReader;
	namespace MySQL {
		struct Date {
			uint16_t year;
			uint8_t month;
			uint8_t day;
		};
		struct Time {
			uint8_t hour;
			uint8_t minute;
			uint8_t second;
		};
		struct DateTime : public Date, public Time {
		};
		using Blob = std::span<const std::byte>;

		template<enum_field_types, bool /*unsigned*/ = false> struct Type;

#define DEFINE_BASE_TYPE(ET, CT, MDS, SIGN) \
	template<> struct Type<ET, SIGN> { \
		using C = CT; \
		constexpr static size_t md_bytes {MDS}; \
		static C read(RawDataReader & md, RawDataReader & data); \
	}
#define DEFINE_TYPE(ET, CT, MDS) DEFINE_BASE_TYPE(ET, CT, MDS, false)
#define DEFINE_ITYPE(ET, BCT, MDS) \
	DEFINE_BASE_TYPE(ET, BCT, MDS, false); \
	DEFINE_BASE_TYPE(ET, u##BCT, MDS, true)

		DEFINE_TYPE(MYSQL_TYPE_DECIMAL, double, 2);
		DEFINE_ITYPE(MYSQL_TYPE_TINY, int8_t, 0);
		DEFINE_ITYPE(MYSQL_TYPE_SHORT, int16_t, 0);
		DEFINE_ITYPE(MYSQL_TYPE_LONG, int32_t, 0);
		DEFINE_TYPE(MYSQL_TYPE_FLOAT, float, 1);
		DEFINE_TYPE(MYSQL_TYPE_DOUBLE, double, 1);
		DEFINE_TYPE(MYSQL_TYPE_NULL, std::nullptr_t, 0);
		DEFINE_TYPE(MYSQL_TYPE_TIMESTAMP, timespec, 0);
		DEFINE_TYPE(MYSQL_TYPE_TIMESTAMP2, timespec, 1);
		DEFINE_ITYPE(MYSQL_TYPE_LONGLONG, int64_t, 0);
		DEFINE_ITYPE(MYSQL_TYPE_INT24, int32_t, 0);
		DEFINE_TYPE(MYSQL_TYPE_DATE, Date, 0);
		DEFINE_TYPE(MYSQL_TYPE_TIME, Time, 0);
		DEFINE_TYPE(MYSQL_TYPE_TIME2, Time, 0);
		DEFINE_TYPE(MYSQL_TYPE_DATETIME, DateTime, 0);
		DEFINE_TYPE(MYSQL_TYPE_DATETIME2, DateTime, 1);
		DEFINE_TYPE(MYSQL_TYPE_YEAR, int16_t, 0);
		DEFINE_TYPE(MYSQL_TYPE_NEWDATE, Date, 0);
		DEFINE_TYPE(MYSQL_TYPE_VARCHAR, std::string_view, 2);
		DEFINE_TYPE(MYSQL_TYPE_BIT, BitSet, 2);
		DEFINE_TYPE(MYSQL_TYPE_NEWDECIMAL, double, 2);
		DEFINE_TYPE(MYSQL_TYPE_ENUM, std::string_view, 0);
		DEFINE_TYPE(MYSQL_TYPE_SET, std::string_view, 0);
		DEFINE_TYPE(MYSQL_TYPE_TINY_BLOB, Blob, 0);
		DEFINE_TYPE(MYSQL_TYPE_MEDIUM_BLOB, Blob, 0);
		DEFINE_TYPE(MYSQL_TYPE_LONG_BLOB, Blob, 0);
		DEFINE_TYPE(MYSQL_TYPE_BLOB, std::string_view, 1);
		DEFINE_TYPE(MYSQL_TYPE_VAR_STRING, std::string_view, 2);
		DEFINE_TYPE(MYSQL_TYPE_STRING, std::string_view, 2);
		DEFINE_TYPE(MYSQL_TYPE_JSON, std::string_view, 2);
		DEFINE_TYPE(MYSQL_TYPE_GEOMETRY, Blob, 1);

#undef DEFINE_ITYPE
#undef DEFINE_USTYPE
#undef DEFINE_TYPE

		using FieldValue = std::variant<std::nullptr_t, double, float, int8_t, uint8_t, int16_t, uint16_t, int32_t,
				uint32_t, int64_t, uint64_t, timespec, Date, Time, DateTime, std::string_view, BitSet, Blob>;
	}
}

#endif
