#ifndef MYGRATE_DBTYPES_H
#define MYGRATE_DBTYPES_H

#include "bitset.h"
#include <boost/numeric/conversion/cast.hpp>
#include <cstdint>
#include <span>
#include <string_view>
#include <variant>

struct timespec;

namespace MyGrate {
	template<typename T> struct printer;
	template<> struct printer<double> {
		constexpr static const char * const fmt {"%g"};
	};
	template<> struct printer<float> {
		constexpr static const char * const fmt {"%g"};
	};
	template<> struct printer<int8_t> {
		constexpr static const char * const fmt {"%hhd"};
	};
	template<> struct printer<uint8_t> {
		constexpr static const char * const fmt {"%hhu"};
	};
	template<> struct printer<int16_t> {
		constexpr static const char * const fmt {"%hd"};
	};
	template<> struct printer<uint16_t> {
		constexpr static const char * const fmt {"%hu"};
	};
	template<> struct printer<int32_t> {
		constexpr static const char * const fmt {"%d"};
	};
	template<> struct printer<uint32_t> {
		constexpr static const char * const fmt {"%u"};
	};
	template<> struct printer<int64_t> {
		constexpr static const char * const fmt {"%ld"};
	};
	template<> struct printer<uint64_t> {
		constexpr static const char * const fmt {"%lu"};
	};

	struct Date {
		inline Date() { }
		inline Date(uint16_t y, uint8_t m, uint8_t d) : year {y}, month {m}, day {d} { }
		explicit inline Date(const tm & tm) :
			Date(boost::numeric_cast<uint16_t>(tm.tm_year + 1900), boost::numeric_cast<uint8_t>(tm.tm_mon + 1),
					boost::numeric_cast<uint8_t>(tm.tm_mday))
		{
		}
		bool operator<=>(const Date &) const = default;
		uint16_t year;
		uint8_t month;
		uint8_t day;
	};
	struct Time {
		inline Time() { }
		inline Time(uint8_t h, uint8_t m, uint8_t s) : hour {h}, minute {m}, second {s} { }
		explicit inline Time(const tm & tm) :
			Time(boost::numeric_cast<uint8_t>(tm.tm_hour), boost::numeric_cast<uint8_t>(tm.tm_min),
					boost::numeric_cast<uint8_t>(tm.tm_sec))
		{
		}
		bool operator<=>(const Time &) const = default;
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
	};
	struct DateTime : public Date, public Time {
		inline DateTime() { }
		inline DateTime(const Date & d, const Time & t) : Date {d}, Time {t} { }
		inline DateTime(uint16_t y, uint8_t m, uint8_t d, uint8_t H, uint8_t M, uint8_t S) :
			DateTime {{y, m, d}, {H, M, S}}
		{
		}
		explicit inline DateTime(const tm & tm) : Date {tm}, Time {tm} { }
		bool operator<=>(const DateTime &) const = default;
	};
	using Blob = std::span<const std::byte>;

	using DbValueV = std::variant<std::nullptr_t, double, float, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t,
			int64_t, uint64_t, timespec, Date, Time, DateTime, std::string_view, BitSet, Blob>;

	namespace detail {
		template<typename I>
		concept HasToString = requires
		{
			std::to_string(I {});
		};

		template<typename T> struct is_false {
			static constexpr bool value {false};
		};

		template<typename R, template<typename> typename ConceptT> struct SafeExtract {
			R
			operator()(const R & i) const
			{
				return i;
			}

			template<typename I>
			R
			operator()(const I & i) const
			{
				if constexpr (ConceptT<I>::value) {
					return boost::numeric_cast<R>(i);
				}
				else {
					throw std::logic_error("Unreasonable conversion requested");
				}
			}
		};

		struct ToString {
			std::string
			operator()(const std::string_view & i) const
			{
				return std::string {i};
			}

			std::string
			operator()(const HasToString auto & i) const
			{
				return std::to_string(i);
			}

			template<typename I>
			std::string
			operator()(const I &) const
			{
				throw std::logic_error("Unreasonable to_string requested");
			}
		};
	}

	class DbValue : public DbValueV {
	public:
		using DbValueV::DbValueV;
		using DbValueV::operator=;

		template<typename V>
		inline auto
		visit(V && v) const
		{
			return std::visit(std::forward<V>(v), static_cast<const DbValueV &>(*this));
		}

		template<typename T>
		inline const auto &
		get() const
		{
			return std::get<T>(static_cast<const DbValueV &>(*this));
		}

		template<typename R> operator R() const
		{
			if constexpr (std::is_integral_v<R>) {
				return visit(detail::SafeExtract<R, std::is_integral> {});
			}
			else if constexpr (std::is_floating_point_v<R>) {
				return visit(detail::SafeExtract<R, std::is_floating_point> {});
			}
			else if constexpr (std::is_same_v<std::string, R>) {
				return visit(detail::ToString {});
			}
			else {
				return get<R>();
			}
		}
	};
}

namespace std {
	std::ostream & operator<<(std::ostream & strm, const MyGrate::DbValueV & v);
}

#endif
