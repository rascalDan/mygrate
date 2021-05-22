#include "streamSupport.h"
#include "bitset.h"
#include "compileTimeFormatter.h"
#include "mysql_types.h"
#include <cstdint>
#include <string_view>
#include <type_traits>
struct timespec;
struct tm;

namespace std {
	std::ostream &
	operator<<(std::ostream & strm, const std::byte byt)
	{
		return AdHoc::scprintf<"%#02x">(strm, std::to_integer<uint8_t>(byt));
	}

	std::ostream &
	operator<<(std::ostream & s, const MARIADB_STRING & str)
	{
		return s << std::string_view(str.str, str.length);
	}

	std::ostream &
	operator<<(std::ostream & s, const tm & tm)
	{
		return AdHoc::scprintf<"%04d-%02d-%02d %02d:%02d:%02d">(
				s, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	}

	std::ostream &
	operator<<(std::ostream & s, const timespec & ts)
	{
		return AdHoc::scprintf<"%d.%09d">(s, ts.tv_sec, ts.tv_nsec);
	}

	std::ostream &
	operator<<(std::ostream & s, const MyGrate::MySQL::Date & d)
	{
		return AdHoc::scprintf<"%04d-%02d-%02d">(s, d.year, d.month, d.day);
	}

	std::ostream &
	operator<<(std::ostream & s, const MyGrate::MySQL::Time & t)
	{
		return AdHoc::scprintf<"%02d:%02d:%02d">(s, t.hour, t.minute, t.second);
	}

	std::ostream &
	operator<<(std::ostream & s, const MyGrate::MySQL::DateTime & dt)
	{
		return AdHoc::scprintf<"%? %?">(s, (const MyGrate::MySQL::Date)dt, (const MyGrate::MySQL::Time)dt);
	}

	std::ostream &
	operator<<(std::ostream & s, const MyGrate::BitSet & bs)
	{
		return s << std::make_pair(bs.begin(), bs.end());
	}
}
