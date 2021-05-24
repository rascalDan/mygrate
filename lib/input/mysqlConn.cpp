#include "mysqlConn.h"
#include "helpers.h"
#include <cstddef>
#include <cstring>
#include <dbTypes.h>
#include <memory>
#include <mysql.h>
#include <mysql_types.h>
#include <stdexcept>
#include <variant>
#include <vector>

namespace MyGrate::Input {
	using StmtPtr = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>;

	MySQLConn::MySQLConn(
			const char * const host, const char * const user, const char * const pass, unsigned short port) :
		st_mysql {}
	{
		mysql_init(this);
		if (!mysql_real_connect(this, host, user, pass, "", port, nullptr, 0)) {
			mysql_close(this);
			throw std::runtime_error("ConnectionError");
		}
		verify<std::runtime_error>(!mysql_set_character_set(this, "utf8"), "Set character set");
	}

	MySQLConn::~MySQLConn()
	{
		mysql_close(this);
	}

	void
	MySQLConn::query(const char * const q)
	{
		verify<std::runtime_error>(!mysql_query(this, q), q);
	}

	struct Bindings {
		// NOLINTNEXTLINE(hicpp-explicit-conversions)
		explicit Bindings(const std::initializer_list<DbValue> & vs)
		{
			binds.reserve(vs.size());
			extras.reserve(vs.size());
			for (const auto & v : vs) {
				std::visit(*this, v);
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
			b.length = &extras.emplace_back(v.size(), 0).len;
		}
		void
		operator()(const std::nullptr_t &)
		{
			auto & b = binds.emplace_back();
			b.buffer = nullptr;
			b.is_null = &extras.emplace_back(0, 1).null;
		}
		template<typename T>
		void
		operator()(const T &)
		{
			throw std::runtime_error("Not implemented");
		}
		struct extra {
			explicit extra(unsigned long l, my_bool n = 0) : len {l}, null {n} { }
			unsigned long len;
			my_bool null;
		};
		std::vector<MYSQL_BIND> binds;
		std::vector<extra> extras;
	};

	void
	MySQLConn::query(const char * const q, const std::initializer_list<DbValue> & vs)
	{
		StmtPtr stmt {mysql_stmt_init(this), &mysql_stmt_close};
		verify<std::runtime_error>(!mysql_stmt_prepare(stmt.get(), q, strlen(q)), q);
		verify<std::logic_error>(mysql_stmt_param_count(stmt.get()) == vs.size(), "Param count mismatch");
		Bindings b {vs};
		verify<std::runtime_error>(!mysql_stmt_bind_param(stmt.get(), b.binds.data()), "Param count mismatch");
		verify<std::runtime_error>(!mysql_stmt_execute(stmt.get()), q);
	}
}
