#ifndef MYGRATE_RAW_DATA_READER_H
#define MYGRATE_RAW_DATA_READER_H

#include "helpers.h"
#include "mariadb_repl.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>

namespace MyGrate {
	struct PackedInteger {
	};
	template<typename T> struct type_map {
		using target = T;
	};
	template<> struct type_map<PackedInteger> {
		using target = uint64_t;
	};

	class RawDataReader {
	public:
		explicit RawDataReader(const void * const d, std::size_t len);
		explicit RawDataReader(const MARIADB_STRING & str);

		template<typename T, size_t L = sizeof(T)>
		typename type_map<T>::target
		readValue()
		{
			static_assert(L > 0 && L <= sizeof(T), "Read exceeds target size");
			return readValue<T>(L);
		}

		template<typename T>
		typename type_map<T>::target
		readValue(size_t L)
		{
			static_assert(std::is_trivial_v<T>, "Do not read non-trivial types");
			verify<std::logic_error>(L > 0 && L <= sizeof(T), "Read exceeds target size");
			offsetSizeCheck(L);
			T v {};
			memcpy(&v, rawdata + offset, L);
			offset += L;
			return v;
		}

		template<typename T>
		T
		viewValue(size_t L)
		{
			static_assert(sizeof(typename T::value_type) == sizeof(std::byte));
			offsetSizeCheck(L);
			T v {reinterpret_cast<const typename T::value_type *>(rawdata + offset), L};
			offset += L;
			return v;
		}

		void discard(size_t);

		bool
		more() const
		{
			return offset < len;
		}

		std::span<const std::byte>
		raw() const
		{
			return {rawdata, len};
		}

	private:
		void offsetSizeCheck(size_t) const;

		const std::byte * const rawdata;
		const std::size_t len;
		std::size_t offset {0};
	};

	template<> uint64_t RawDataReader::readValue<PackedInteger>();
}

#endif
