#ifndef MYGRATE_DBTYPES_H
#define MYGRATE_DBTYPES_H

#include "bitset.h"
#include <cstdint>
#include <span>
#include <string_view>
#include <variant>

struct timespec;

namespace MyGrate {
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

	using DbValue = std::variant<std::nullptr_t, double, float, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t,
			int64_t, uint64_t, timespec, Date, Time, DateTime, std::string_view, BitSet, Blob>;
}

#endif
