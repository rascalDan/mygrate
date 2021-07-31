#ifndef MYGRATE_MYSQL_TYPES_H
#define MYGRATE_MYSQL_TYPES_H

#include "bitset.h"
#include "dbTypes.h"
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <mysql.h>
#include <string_view>

namespace MyGrate {
	class RawDataReader;
	namespace MySQL {

		template<enum_field_types, bool /*unsigned*/ = false> struct Type;
		template<typename T> struct CType;

#define DEFINE_BASE_TYPE(ET, CT, MDS, SIGN) \
	template<> struct Type<ET, SIGN> { \
		using C = CT; \
		constexpr static size_t md_bytes {MDS}; \
		static C read(RawDataReader & md, RawDataReader & data); \
	}
#define DEFINE_CTYPE(ET, CT) \
	template<> struct CType<CT> { \
		using C = CT; \
		static constexpr enum_field_types type {ET}; \
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

		DEFINE_CTYPE(MYSQL_TYPE_DOUBLE, double);
		DEFINE_CTYPE(MYSQL_TYPE_FLOAT, float);
		DEFINE_CTYPE(MYSQL_TYPE_SHORT, int16_t);
		DEFINE_CTYPE(MYSQL_TYPE_LONG, int32_t);
		DEFINE_CTYPE(MYSQL_TYPE_LONGLONG, int64_t);
		DEFINE_CTYPE(MYSQL_TYPE_TINY, int8_t);
		DEFINE_CTYPE(MYSQL_TYPE_SHORT, uint16_t);
		DEFINE_CTYPE(MYSQL_TYPE_LONG, uint32_t);
		DEFINE_CTYPE(MYSQL_TYPE_LONGLONG, uint64_t);
		DEFINE_CTYPE(MYSQL_TYPE_TINY, uint8_t);
		DEFINE_CTYPE(MYSQL_TYPE_STRING, std::string_view);
		DEFINE_CTYPE(MYSQL_TYPE_BLOB, Blob);

#undef DEFINE_ITYPE
#undef DEFINE_USTYPE
#undef DEFINE_TYPE

		struct ReplicationPosition {
			std::string filename;
			uint64_t position;
		};
	}
}

#endif
