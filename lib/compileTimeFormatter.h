#ifndef MYGRATE_COMPILE_TIME_FORMATTER_H
#define MYGRATE_COMPILE_TIME_FORMATTER_H

#include "fixedString.h"
#include <array>
#include <boost/preprocessor/variadic/size.hpp>
#include <cstring>
#include <iostream>
#include <optional>
#include <sstream> // IWYU pragma: export
#include <string_view>

namespace MyGrate {
	// Template char utils
	template<typename char_type>
	constexpr bool
	isdigit(const char_type & ch)
	{
		return (ch >= '0' && ch <= '9');
	}

	template<typename char_type>
	constexpr bool
	ispositivedigit(const char_type & ch)
	{
		return (ch >= '1' && ch <= '9');
	}

	// Template string utils
	template<const auto S>
	static constexpr auto
	strlen()
	{
		auto off = 0U;
		while (S[off]) {
			++off;
		}
		return off;
	}

	template<typename char_type>
	static constexpr auto
	strlen(const char_type * S)
	{
		auto off = 0U;
		while (S[off]) {
			++off;
		}
		return off;
	}

	template<const auto S, auto n, auto start = 0U, auto L = strlen<S>()>
	static constexpr std::optional<decltype(start)>
	strchr()
	{
		static_assert(start <= L);
		decltype(start) off = start;
		while (off < L && S[off] != n) {
			++off;
		}
		if (off == L) {
			return {};
		}
		return off;
	}

	template<const auto S, auto n, auto start = 0U, auto L = strlen<S>()>
	static constexpr decltype(L)
	strchrnul()
	{
		decltype(start) off = start;
		while (off < L && S[off] != n) {
			++off;
		}
		return off;
	}

	template<const auto S, const auto L> class FormatterDetail;

	/// Template used to apply parameters to a stream.
	template<const auto S, auto L, auto pos, typename stream, typename, auto...> struct StreamWriter {
		/// Write parameters to stream.
		template<typename... Pn>
		static void
		write(stream &, const Pn &...)
		{
			static_assert(!L, "invalid format string/arguments");
		}
	};

	/// Helper to simplify implementations of StreamWriter.
	template<const auto S, auto L, auto pos, typename stream> struct StreamWriterBase {
		/// Continue processing parameters.
		template<typename... Pn>
		static inline void
		next(stream & s, const Pn &... pn)
		{
			FormatterDetail<S, L>::template Parser<stream, pos + 1, Pn...>::run(s, pn...);
		}
	};

#define StreamWriterT(C...) \
	template<const auto S, auto L, auto pos, typename stream, auto... sn> \
	struct StreamWriter<S, L, pos, stream, void, '%', C, sn...> : \
		public StreamWriterBase<S, L, BOOST_PP_VARIADIC_SIZE(C) + pos, stream>

#define StreamWriterTP(P, C...) \
	template<const auto S, auto L, auto pos, typename stream, auto P, auto... sn> \
	struct StreamWriter<S, L, pos, stream, void, '%', C, sn...> : \
		public StreamWriterBase<S, L, BOOST_PP_VARIADIC_SIZE(C) + pos, stream>

	// Default stream writer formatter
	StreamWriterT('?') {
		template<typename P, typename... Pn>
		static inline void
		write(stream & s, const P & p, const Pn &... pn)
		{
			s << p;
			StreamWriter::next(s, pn...);
		}
	};

	// Escaped % stream writer formatter
	StreamWriterT('%') {
		template<typename... Pn>
		static inline void
		write(stream & s, const Pn &... pn)
		{
			s << '%';
			StreamWriter::next(s, pn...);
		}
	};

	template<typename stream, typename char_type>
	static inline void
	appendStream(stream & s, const char_type * p, size_t n)
	{
		s.write(p, n);
	}

	template<typename stream>
	static inline auto
	streamLength(stream & s)
	{
		return s.tellp();
	}

	/**
	 * Compile time string formatter.
	 * @param S the format string.
	 */
	template<const auto S, const auto L> class FormatterDetail {
	private:
		using strlen_t = decltype(strlen<S>());
		template<const auto, auto, auto, typename> friend struct StreamWriterBase;

	public:
		/// The derived charater type of the format string.
		using char_type = typename std::decay<decltype(S[0])>::type;
		/**
		 * Get a string containing the result of formatting.
		 * @param pn the format arguments.
		 * @return the formatted string.
		 */
		template<typename... Pn>
		static inline auto
		get(const Pn &... pn)
		{
			std::basic_stringstream<char_type> s;
			return write(s, pn...).str();
		}
		/**
		 * Get a string containing the result of formatting.
		 * @param pn the format arguments.
		 * @return the formatted string.
		 */
		template<typename... Pn>
		inline auto
		operator()(const Pn &... pn) const
		{
			return get(pn...);
		}

		/**
		 * Write the result of formatting to the given stream.
		 * @param s the stream to write to.
		 * @param pn the format arguments.
		 * @return the stream.
		 */
		template<typename stream, typename... Pn>
		static inline stream &
		write(stream & s, const Pn &... pn)
		{
			return Parser<stream, 0U, Pn...>::run(s, pn...);
		}
		/**
		 * Write the result of formatting to the given stream.
		 * @param s the stream to write to.
		 * @param pn the format arguments.
		 * @return the stream.
		 */
		template<typename stream, typename... Pn>
		inline typename std::enable_if<std::is_base_of_v<std::basic_ostream<char_type>, stream>, stream>::type &
		operator()(stream & s, const Pn &... pn) const
		{
			return write(s, pn...);
		}

	private:
		template<typename stream, auto pos, typename... Pn> struct Parser {
			static inline stream &
			run(stream & s, const Pn &... pn)
			{
				if (pos != L) {
					constexpr auto ph = strchrnul<S, '%', pos, L>();
					if constexpr (ph != pos) {
						appendStream(s, &S[pos], ph - pos);
					}
					if constexpr (ph != L) {
						packAndWrite<ph>(s, pn...);
					}
				}
				return s;
			}
			template<strlen_t ph, strlen_t off = 0U, auto... Pck>
			static inline void
			packAndWrite(stream & s, const Pn &... pn)
			{
				if constexpr (ph + off == L || sizeof...(Pck) == 32) {
					StreamWriter<S, L, ph, stream, void, Pck...>::write(s, pn...);
				}
				else if constexpr (ph + off < L) {
					packAndWrite<ph, off + 1, Pck..., S[ph + off]>(s, pn...);
				}
			}
		};
	};

	template<const Support::basic_fixed_string Str> class LiteralFormatter : public FormatterDetail<Str, Str.size()> {
	};

	template<const auto & S, decltype(strlen(S)) L = strlen(S)>
	class Formatter :
		public FormatterDetail<Support::basic_fixed_string<typename std::decay<decltype(S[0])>::type, L>(S, L), L> {
	};

	template<const Support::basic_fixed_string Str, typename... Pn>
	inline auto
	scprintf(const Pn &... pn)
	{
		return FormatterDetail<Str, Str.size()>::get(pn...);
	}

	template<typename T>
	concept Writable = requires(T s)
	{
		{s << 0};
		{s << ""};
		{s.write("", 0U)};
	};
	template<const Support::basic_fixed_string Str, Writable stream, typename... Pn>
	inline auto &
	scprintf(stream & strm, const Pn &... pn)
	{
		return FormatterDetail<Str, Str.size()>::write(strm, pn...);
	}

	template<const Support::basic_fixed_string Str, typename... Pn>
	inline auto &
	cprintf(const Pn &... pn)
	{
		return scprintf<Str>(std::cout, pn...);
	}
}

#include <boost/assert.hpp>
#include <boost/preprocessor/arithmetic/add.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/comma_if.hpp>
#include <boost/preprocessor/if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <iomanip>
#include <type_traits>

namespace MyGrate {
#define BASICCONV(PARAMTYPE, OP, ...) \
	StreamWriterT(__VA_ARGS__) { \
		template<typename... Pn> \
		static inline void \
		write(stream & s, const PARAMTYPE & p, const Pn &... pn) \
		{ \
			OP; \
			s.copyfmt(std::ios(nullptr)); \
			StreamWriter::next(s, pn...); \
		} \
	}

	// Integers (d, i, o, u, x, X)
#define INTCONV(BASE, OP, CONV) \
	BASICCONV(BASE, OP, CONV); \
	BASICCONV(short BASE, OP, 'h', CONV); \
	BASICCONV(long BASE, OP, 'l', CONV); \
	BASICCONV(long long BASE, OP, 'l', 'l', CONV);
	INTCONV(int, s << std::dec << p, 'i');
	INTCONV(int, s << std::dec << p, 'd');
	INTCONV(unsigned int, s << std::oct << p, 'o');
	INTCONV(unsigned int, s << std::dec << p, 'u');
	INTCONV(unsigned int, s << std::nouppercase << std::hex << p, 'x');
	INTCONV(unsigned int, s << std::uppercase << std::hex << p, 'X');
#undef INTCONV

	BASICCONV(intmax_t, s << std::dec << p, 'j', 'd');
	BASICCONV(uintmax_t, s << std::dec << p, 'j', 'u');
	BASICCONV(ssize_t, s << std::dec << p, 'z', 'd');
	BASICCONV(size_t, s << std::dec << p, 'z', 'u');
	BASICCONV(short int, s << std::dec << p, 'h', 'h', 'i'); // char
	BASICCONV(short int, s << std::dec << p, 'h', 'h', 'd'); // char
	BASICCONV(unsigned char, s << std::dec << p, 'h', 'h', 'u');
	BASICCONV(unsigned char, s << std::oct << p, 'h', 'h', 'o');
	BASICCONV(unsigned char, s << std::nouppercase << std::hex << p, 'h', 'h', 'x');
	BASICCONV(unsigned char, s << std::uppercase << std::hex << p, 'h', 'h', 'X');

	// Floating point (a, A, e, E, f, F, g, G)
#define FPCONV(BASE, OP, CONV) \
	BASICCONV(BASE, OP, CONV); \
	BASICCONV(long BASE, OP, 'L', CONV);
	FPCONV(double, s << std::nouppercase << std::hexfloat << p, 'a');
	FPCONV(double, s << std::uppercase << std::hexfloat << p, 'A');
	FPCONV(double, s << std::nouppercase << std::scientific << p, 'e');
	FPCONV(double, s << std::uppercase << std::scientific << p, 'E');
	FPCONV(double, s << std::nouppercase << std::fixed << p, 'f');
	FPCONV(double, s << std::uppercase << std::fixed << p, 'F');
	FPCONV(double, s << std::nouppercase << std::defaultfloat << p, 'g');
	FPCONV(double, s << std::uppercase << std::defaultfloat << p, 'G');
#undef FPCONV

	BASICCONV(std::string_view, s << p, 's');
	BASICCONV(std::wstring_view, s << p, 'l', 's');
	BASICCONV(char, s << p, 'c');
	BASICCONV(wchar_t, s << p, 'l', 'c');
#undef BASICCONV
	StreamWriterT('p') {
		template<typename Obj, typename... Pn>
		static inline void
		write(stream & s, Obj * const ptr, const Pn &... pn)
		{
			s << std::showbase << std::hex << (long unsigned int)ptr;
			s.copyfmt(std::ios(nullptr));
			StreamWriter::next(s, pn...);
		}
		template<typename Ptr, typename... Pn>
		static inline void
		write(stream & s, const Ptr & ptr, const Pn &... pn)
		{
			write(s, ptr.get(), pn...);
		}
	};

	StreamWriterT('m') {
		template<typename... Pn>
		static inline void
		write(stream & s, const Pn &... pn)
		{
			s << strerror(errno);
			s.copyfmt(std::ios(nullptr));
			StreamWriter::next(s, pn...);
		}
	};
	StreamWriterT('n') {
		template<typename... Pn>
		static inline void
		write(stream & s, int * n, const Pn &... pn)
		{
			BOOST_ASSERT_MSG(n, "%n conversion requires non-null parameter");
			*n = streamLength(s);
			s.copyfmt(std::ios(nullptr));
			StreamWriter::next(s, pn...);
		}
	};

	////
	// Width/precision embedded in format string
	template<auto... chs>
	constexpr auto
	decdigits()
	{
		static_assert((isdigit(chs) && ... && true));
		int n = 0;
		(
				[&n](auto ch) {
					n = (n * 10) + (ch - '0');
				}(chs),
				...);
		return n;
	}
#define AUTON(z, n, data) \
	BOOST_PP_COMMA_IF(n) \
	auto BOOST_PP_CAT(data, n)
#define NS(z, n, data) \
	BOOST_PP_COMMA_IF(n) \
	BOOST_PP_CAT(data, n)
#define ISDIGIT(z, n, data) &&isdigit(BOOST_PP_CAT(data, BOOST_PP_ADD(n, 1)))
#define FMTWIDTH(unused, d, data) \
	template<const auto S, auto L, auto pos, typename stream, BOOST_PP_REPEAT(BOOST_PP_ADD(d, 1), AUTON, n), auto nn, \
			auto... sn> \
	struct StreamWriter<S, L, pos, stream, \
			typename std::enable_if<ispositivedigit(n0) BOOST_PP_REPEAT(d, ISDIGIT, n) && !isdigit(nn)>::type, '%', \
			BOOST_PP_REPEAT(BOOST_PP_ADD(d, 1), NS, n), nn, sn...> { \
		template<typename... Pn> \
		static inline void \
		write(stream & s, const Pn &... pn) \
		{ \
			constexpr auto p = decdigits<BOOST_PP_REPEAT(BOOST_PP_ADD(d, 1), NS, n)>(); \
			s << std::setw(p); \
			StreamWriter<S, L, pos + BOOST_PP_ADD(d, 1), stream, void, '%', nn, sn...>::write(s, pn...); \
		} \
	};
	BOOST_PP_REPEAT(6, FMTWIDTH, void);
#define FMTPRECISION(unused, d, data) \
	template<const auto S, auto L, auto pos, typename stream, BOOST_PP_REPEAT(BOOST_PP_ADD(d, 1), AUTON, n), auto nn, \
			auto... sn> \
	struct StreamWriter<S, L, pos, stream, \
			typename std::enable_if<isdigit(n0) BOOST_PP_REPEAT(d, ISDIGIT, n) && !isdigit(nn)>::type, '%', '.', \
			BOOST_PP_REPEAT(BOOST_PP_ADD(d, 1), NS, n), nn, sn...> { \
		template<typename... Pn> \
		static inline void \
		write(stream & s, const Pn &... pn) \
		{ \
			constexpr auto p = decdigits<BOOST_PP_REPEAT(BOOST_PP_ADD(d, 1), NS, n)>(); \
			s << std::setprecision(p); \
			StreamWriter<S, L, pos + BOOST_PP_ADD(d, 2), stream, void, '%', nn, sn...>::write(s, pn...); \
		} \
	};
	BOOST_PP_REPEAT(6, FMTPRECISION, void);
#undef AUTON
#undef NS
#undef ISDIGIT
#undef FMTWIDTH

	StreamWriterT('.', '*') {
		template<typename... Pn>
		static inline void
		write(stream & s, int l, const Pn &... pn)
		{
			s << std::setw(l);
			StreamWriter<S, L, pos + 2, stream, void, '%', sn...>::write(s, pn...);
		}
	};
	StreamWriterT('.', '*', 's') {
		template<typename... Pn>
		static inline void
		write(stream & s, int l, const std::string_view & p, const Pn &... pn)
		{
			s << p.substr(0, l);
			s.copyfmt(std::ios(nullptr));
			StreamWriter::next(s, pn...);
		}
	};

		// Flags
#define FLAGCONV(OP, ...) \
	StreamWriterT(__VA_ARGS__) { \
		template<typename... Pn> \
		static inline void \
		write(stream & s, const Pn &... pn) \
		{ \
			OP; \
			StreamWriter<S, L, pos + 1, stream, void, '%', sn...>::write(s, pn...); \
		} \
	};
	FLAGCONV(s << std::showbase, '#');
	FLAGCONV(s << std::setfill('0'), '0');
	FLAGCONV(s << std::left, '-');
	FLAGCONV(s << std::showpos, '+');
	FLAGCONV(s << std::setfill(' '), ' ');
#undef FLAGCONV
}

#endif
