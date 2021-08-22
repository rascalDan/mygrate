#ifndef MYGRATE_STREAM_SUPPORT_H
#define MYGRATE_STREAM_SUPPORT_H

#include "eventCounter.h"
#include "mariadb_repl.h"
#include <array>
#include <cstddef>
#include <ctime>
#include <ostream>
#include <span>
#include <utility>
#include <vector>
namespace MyGrate {
	class BitSet;
	struct Date;
	struct DateTime;
	struct Time;
}
struct timespec;
struct tm;

namespace std {
	std::ostream & operator<<(std::ostream & strm, const std::byte byt);

	std::ostream & operator<<(std::ostream & s, const MARIADB_STRING & str);

	std::ostream & operator<<(std::ostream & s, const tm & tm);

	std::ostream & operator<<(std::ostream & s, const timespec & ts);

	std::ostream & operator<<(std::ostream & s, const MyGrate::Date & d);

	std::ostream & operator<<(std::ostream & s, const MyGrate::Time & t);

	std::ostream & operator<<(std::ostream & s, const MyGrate::DateTime & dt);

	std::ostream & operator<<(std::ostream & s, const MyGrate::BitSet & bs);

	template<std::forward_iterator Iter>
	inline std::ostream &
	operator<<(std::ostream & strm, const std::pair<Iter, Iter> range)
	{
		strm << '[';
		for (auto i {range.first}; i != range.second; i++) {
			if (i != range.first) {
				strm << ',';
			}
			strm << *i;
		}
		return strm << ']';
	}

	template<typename T>
	std::ostream &
	operator<<(std::ostream & strm, const std::span<const T> spn)
	{
		return strm << std::make_pair(spn.begin(), spn.end());
	}

	template<typename T>
	std::ostream &
	operator<<(std::ostream & strm, const std::vector<T> & v)
	{
		return strm << std::span<const T>(v.data(), v.size());
	}

	template<typename T, std::size_t L>
	std::ostream &
	operator<<(std::ostream & strm, const std::array<T, L> & a)
	{
		return strm << std::span<const T>(a.begin(), a.end());
	}
}

#endif
