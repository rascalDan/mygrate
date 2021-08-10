#ifndef MYGRATE_INPUT_MYSQLBINDINGS_H
#define MYGRATE_INPUT_MYSQLBINDINGS_H

#include <dbTypes.h>
#include <helpers.h>
#include <initializer_list>
#include <mysql.h>
#include <mysql_types.h>
#include <variant>
#include <vector>

namespace MyGrate::Input {
	struct BingingData {
		explicit BingingData(unsigned long l, my_bool n = 0) : len {l}, null {n} { }
		unsigned long len;
		my_bool null;
	};

	struct Bindings {
		explicit Bindings(const std::span<const DbValue> vs)
		{
			binds.reserve(vs.size());
			data.reserve(vs.size());
			for (const auto & v : vs) {
				v.visit(*this);
			}
		}
		template<std::integral T>
		void
		operator()(const T & v)
		{
			auto & b = binds.emplace_back();
			b.buffer_type = MySQL::CType<T>::type;
			b.buffer = const_cast<T *>(&v);
			b.is_unsigned = std::unsigned_integral<T>;
		}
		template<std::floating_point T>
		void
		operator()(const T & v)
		{
			auto & b = binds.emplace_back();
			b.buffer_type = MySQL::CType<T>::type;
			b.buffer = const_cast<T *>(&v);
		}
		template<Viewable T>
		void
		operator()(const T & v)
		{
			auto & b = binds.emplace_back();
			b.buffer_type = MySQL::CType<T>::type;
			b.buffer = const_cast<typename T::value_type *>(v.data());
			b.length = &data.emplace_back(v.size(), 0).len;
		}
		void
		operator()(const std::nullptr_t &)
		{
			auto & b = binds.emplace_back();
			b.buffer = nullptr;
			b.is_null = &data.emplace_back(0, 1).null;
		}
		void
		operator()(const auto &)
		{
			throw std::logic_error("Not implemented");
		}
		std::vector<MYSQL_BIND> binds;
		std::vector<BingingData> data;
	};

	class ResultData : public BingingData {
	public:
		ResultData() : BingingData {0} { }
		virtual ~ResultData() = default;

		[[nodiscard]] virtual DbValue getValue() const = 0;
	};

	template<typename T> class ResultDataT : public ResultData {
	public:
		ResultDataT(MYSQL_BIND & b, const MYSQL_FIELD & f)
		{
			b.buffer = &buf;
			b.buffer_length = sizeof(T);
			b.is_null = &this->null;
			b.length = &this->len;
			b.is_unsigned = std::is_unsigned_v<T>;
			b.buffer_type = f.type;
		}

		[[nodiscard]] DbValue
		getValue() const override
		{
			return buf;
		}

	private:
		T buf {};
	};

	template<> class ResultDataT<std::string_view> : public ResultData {
	public:
		ResultDataT(MYSQL_BIND & b, const MYSQL_FIELD & f) : buf(f.length)
		{
			b.buffer_length = buf.size();
			b.buffer = buf.data();
			b.is_null = &this->null;
			b.length = &this->len;
			b.buffer_type = f.type;
		}

		[[nodiscard]] DbValue
		getValue() const override
		{
			return std::string_view {buf.data(), this->len};
		}

	private:
		std::vector<char> buf;
	};

	template<> class ResultDataT<Blob> : public ResultData {
	public:
		ResultDataT(MYSQL_BIND & b, const MYSQL_FIELD & f) : buf(f.length)
		{
			b.buffer_length = buf.size();
			b.buffer = buf.data();
			b.is_null = &this->null;
			b.length = &this->len;
			b.buffer_type = f.type;
		}

		[[nodiscard]] DbValue
		getValue() const override
		{
			return Blob {buf.data(), this->len};
		}

	private:
		std::vector<std::byte> buf;
	};

	template<typename Out> class ResultDataTime : public ResultData {
	public:
		ResultDataTime(MYSQL_BIND & b, const MYSQL_FIELD & f)
		{
			b.buffer_length = sizeof(MYSQL_TIME);
			b.buffer = &buf;
			b.is_null = &this->null;
			b.length = &this->len;
			b.buffer_type = f.type;
		}

		[[nodiscard]] DbValue
		getValue() const override
		{
			return Out {*this};
		}

	private:
		operator Date() const
		{
			return Date(boost::numeric_cast<decltype(Date::year)>(buf.year),
					boost::numeric_cast<decltype(Date::month)>(buf.month),
					boost::numeric_cast<decltype(Date::day)>(buf.day));
		}
		operator Time() const
		{
			return Time(boost::numeric_cast<decltype(Time::hour)>(buf.hour),
					boost::numeric_cast<decltype(Time::minute)>(buf.minute),
					boost::numeric_cast<decltype(Time::second)>(buf.second));
		}
		operator DateTime() const
		{
			return DateTime(*this, *this);
		}
		MYSQL_TIME buf;
	};
}

#endif
