#ifndef MYGRATE_TEST_HELPERS_H
#define MYGRATE_TEST_HELPERS_H

#include <concepts>
#include <cstddef>
#include <ctime>
#include <iostream>
#include <optional>

inline constexpr std::byte operator""_b(const unsigned long long hex)
{
	return std::byte(hex);
}

inline constexpr bool
operator==(const struct tm & a, const struct tm & b)
{
	return (a.tm_year == b.tm_year) && (a.tm_mon == b.tm_mon) && (a.tm_mday == b.tm_mday) && (a.tm_hour == b.tm_hour)
			&& (a.tm_min == b.tm_min) && (a.tm_sec == b.tm_sec);
}

namespace std {
	template<integral T>
	inline constexpr bool
	operator!=(const byte b, const T i)
	{
		return to_integer<T>(b) != i;
	}

	template<typename T>
	ostream &
	operator<<(ostream & s, const std::optional<T> & o)
	{
		if (o) {
			return s << *o;
		}
		else {
			return s << "-";
		}
	}
}

inline struct tm
make_tm(int year, int mon, int day, int hr, int min, int sec)
{
	struct tm tm {
	};
	tm.tm_year = year - 1900;
	tm.tm_mon = mon - 1;
	tm.tm_mday = day;
	tm.tm_hour = hr;
	tm.tm_min = min;
	tm.tm_sec = sec;
	mktime(&tm);
	return tm;
}

#endif
