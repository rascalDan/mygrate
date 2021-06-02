#ifndef MYGRATE_DBSTMT_H
#define MYGRATE_DBSTMT_H

#include "fixedString.h"
#include <dbConn.h>
#include <dbRecordSet.h>
#include <memory>
#include <string_view>
#include <type_traits>

namespace MyGrate {
	template<Support::basic_fixed_string S> class DbStmt {
	public:
		// This don't account for common table expressions, hopefully won't need those :)
		static constexpr auto isSelect {S.v().starts_with("SELECT") || S.v().starts_with("SHOW")
				|| S.v().find("RETURNING") != std::string_view::npos};

		// These don't account for string literals, which we'd prefer to avoid anyway :)
		static constexpr std::size_t
		paramCount(ParamMode pm)
		{
			switch (pm) {
				case ParamMode::None:
					return 0LU;
				case ParamMode::DollarNum: {
					const auto pn = [](const char * c, const char * const e) {
						std::size_t n {0};
						while (++c != e && *c >= '0' && *c <= '9') {
							n = (n * 10) + (*c - '0');
						}
						return n;
					};
					return pn(std::max_element(S.v().begin(), S.v().end(),
									  [pn, e = S.v().end()](const char & a, const char & b) {
										  return (a == '$' ? pn(&a, e) : 0) < (b == '$' ? pn(&b, e) : 0);
									  }),
							S.v().end());
				}
				case ParamMode::QMark:
					return std::count_if(S.v().begin(), S.v().end(), [](char c) {
						return c == '?';
					});
			}
		}

		using Return = std::conditional_t<isSelect, RecordSetPtr, std::size_t>;

		template<typename ConnType, typename... P>
		static Return
		execute(ConnType * c, P &&... p)
		{
			static_assert(sizeof...(P) == paramCount(ConnType::paramMode));
			auto stmt {c->prepare(S, sizeof...(P))};
			stmt->execute({std::forward<P...>(p)...});
			if constexpr (isSelect) {
				return stmt->recordSet();
			}
			else {
				return stmt->rows();
			}
		}
	};
}

#endif
