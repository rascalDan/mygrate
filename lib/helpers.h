#ifndef MYGRATE_HELPERS_H
#define MYGRATE_HELPERS_H

#include <concepts>
#include <cstdint>
#include <utility>

namespace MyGrate {
	template<std::integral I>
	constexpr inline auto
	bitslice(const I i, uint8_t offset, uint8_t size)
	{
		return (i >> offset) & ((1U << size) - 1U);
	}

	template<typename X, typename... P>
	constexpr inline void
	verify(bool expr, P &&... p)
	{
		if (!expr) {
			throw X(std::forward<P...>(p)...);
		}
	}

	template<typename T>
	constexpr inline auto
	mod100_extract(T & i)
	{
		const auto r {i % 100};
		i /= 100;
		return r;
	}
}

#endif
