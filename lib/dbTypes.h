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
		inline Date() { }
		inline Date(uint16_t y, uint8_t m, uint8_t d) : year {y}, month {m}, day {d} { }
		explicit inline Date(const tm & tm) : Date(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday) { }
		uint16_t year;
		uint8_t month;
		uint8_t day;
	};
	struct Time {
		inline Time() { }
		inline Time(uint8_t h, uint8_t m, uint8_t s) : hour {h}, minute {m}, second {s} { }
		explicit inline Time(const tm & tm) : Time(tm.tm_hour, tm.tm_min, tm.tm_sec) { }
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
	};
	struct DateTime : public Date, public Time {
		inline DateTime() { }
		inline DateTime(const Date & d, const Time & t) : Date {d}, Time {t} { }
		explicit inline DateTime(const tm & tm) : Date {tm}, Time {tm} { }
	};
	using Blob = std::span<const std::byte>;

	using DbValue = std::variant<std::nullptr_t, double, float, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t,
			int64_t, uint64_t, timespec, Date, Time, DateTime, std::string_view, BitSet, Blob>;
}

#endif
