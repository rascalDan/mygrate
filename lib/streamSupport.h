#ifndef MYGRATE_STREAM_SUPPORT_H
#define MYGRATE_STREAM_SUPPORT_H

#include <array>
#include <cstddef>
#include <iomanip>
#include <mysql.h>
#include <ostream>
#include <span>
#include <vector>

#include "mysql_types.h"
#include <mariadb_rpl.h>

namespace std {
	std::ostream & operator<<(std::ostream & strm, const std::byte byt);

	std::ostream & operator<<(std::ostream & s, const MARIADB_STRING & str);

	std::ostream & operator<<(std::ostream & s, const tm & tm);

	std::ostream & operator<<(std::ostream & s, const timespec & ts);

	std::ostream & operator<<(std::ostream & s, const MyGrate::MySQL::Date & d);

	std::ostream & operator<<(std::ostream & s, const MyGrate::MySQL::Time & t);

	std::ostream & operator<<(std::ostream & s, const MyGrate::MySQL::DateTime & dt);

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
