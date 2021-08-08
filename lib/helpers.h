#ifndef MYGRATE_HELPERS_H
#define MYGRATE_HELPERS_H

#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <utility>

namespace MyGrate {
	constexpr inline auto
	bitslice(const std::integral auto i, uint8_t offset, uint8_t size)
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
		const auto r {i % 100};
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
