#include "streamSupport.h"
#include "bitset.h"
#include "compileTimeFormatter.h"
#include "dbTypes.h"
#include <cstdint>
#include <string_view>
#include <type_traits>
struct timespec;
struct tm;

namespace std {
	std::ostream &
	operator<<(std::ostream & strm, const std::byte byt)
	{
		return MyGrate::scprintf<"%#02x">(strm, std::to_integer<uint8_t>(byt));
	}

	std::ostream &
	operator<<(std::ostream & s, const MARIADB_STRING & str)
	{
		return s << *str;
	}

	std::ostream &
	operator<<(std::ostream & s, const tm & tm)
	{
		return MyGrate::scprintf<"%04d-%02d-%02d %02d:%02d:%02d">(
				s, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	}

	std::ostream &
	operator<<(std::ostream & s, const timespec & ts)
	{
		return MyGrate::scprintf<"%ld.%09ld">(s, ts.tv_sec, ts.tv_nsec);
	}

	std::ostream &
	operator<<(std::ostream & s, const MyGrate::Date & d)
	{
		return MyGrate::scprintf<"%04d-%02d-%02d">(s, d.year, d.month, d.day);
	}

	std::ostream &
	operator<<(std::ostream & s, const MyGrate::Time & t)
	{
		return MyGrate::scprintf<"%02d:%02d:%02d">(s, t.hour, t.minute, t.second);
	}

	std::ostream &
	operator<<(std::ostream & s, const MyGrate::DateTime & dt)
	{
		return MyGrate::scprintf<"%? %?">(
				s, static_cast<const MyGrate::Date>(dt), static_cast<const MyGrate::Time>(dt));
	}

	std::ostream &
	operator<<(std::ostream & s, const MyGrate::BitSet & bs)
	{
		return s << std::make_pair(bs.begin(), bs.end());
	}
}
