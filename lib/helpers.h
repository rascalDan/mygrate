#ifndef MYGRATE_HELPERS_H
#define MYGRATE_HELPERS_H

#include <boost/numeric/conversion/cast.hpp>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <utility>

namespace MyGrate {
	template<uint8_t size>
	using SmallestUInt = std::conditional_t<size <= 8, uint8_t,
			std::conditional_t<size <= 16, uint16_t, std::conditional_t<size <= 32, uint32_t, uint64_t>>>;

	template<uint8_t size>
	constexpr inline auto
	bitslice(const std::integral auto i, uint8_t offset) -> SmallestUInt<size>
	{
		return (i >> offset) & ((1U << size) - 1U);
	}

	template<typename X, typename R, typename... P>
	constexpr inline auto
	verify(R expr, P &&... p)
	{
		if (!expr) [[unlikely]] {
			throw X(std::forward<P>(p)...);
		}
		return expr;
	}

	constexpr inline auto
	mod100_extract(std::integral auto & i)
	{
		using R = std::conditional_t<std::is_signed_v<decltype(i)>, int8_t, uint8_t>;
		const auto r {boost::numeric_cast<R>(i % 100)};
		i /= 100;
		return r;
	}

	template<typename T>
	concept Stringable = requires(T a)
	{
		{
			std::to_string(a)
			} -> std::same_as<std::string>;
	};

	template<typename T>
	concept Viewable = requires(T a)
	{
		{
			a.data()
			} -> std::convertible_to<const char *>;
		{
			a.size()
			} -> std::integral;
	};

	inline const char *
	getenv(const char * env, const char * dfl)
	{
		if (const auto e {::getenv(env)}) {
			return e;
		}
		return dfl;
	}
}

#endif
