#ifndef MYGRATE_SUPPORT_FIXEDSTRING_H
#define MYGRATE_SUPPORT_FIXEDSTRING_H

#include <array>
#include <cstddef>
#include <string_view>

namespace MyGrate::Support {
	template<typename CharT, std::size_t N> class basic_fixed_string {
	public:
		// cppcheck-suppress noExplicitConstructor
		constexpr basic_fixed_string(const CharT (&str)[N + 1])
		{
			for (decltype(N) x = 0; x < N; x++) {
				arr.at(x) = str[x];
			}
			arr.at(N) = '\0';
		}
		constexpr basic_fixed_string(const CharT * str, decltype(N) len)
		{
			for (decltype(N) x = 0; x < len; x++) {
				arr.at(x) = str[x];
			}
			arr.at(N) = '\0';
		}
		constexpr const char *
		s() const
		{
			return arr.data();
		}
		constexpr operator const char *() const
		{
			return s();
		}
		constexpr std::string_view
		v() const
		{
			return {arr.data(), arr.size() - 1};
		}
		constexpr auto &
		operator[](std::size_t n) const
		{
			return arr[n];
		}
		constexpr auto
		size() const
		{
			return arr.size() - 1;
		}

		std::array<CharT, N + 1> arr;
	};

	template<typename CharT, std::size_t N>
	basic_fixed_string(const CharT (&str)[N]) -> basic_fixed_string<CharT, N - 1>;

}

#endif
