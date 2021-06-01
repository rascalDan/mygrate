#include "mysqlRecordSet.h"
#include "mysqlBindings.h"
#include "mysqlStmt.h"
#include <cstdint>
#include <dbTypes.h>
#include <helpers.h>
#include <stdexcept>
#include <string_view>
#include <utility>
// IWYU pragma: no_include <ext/alloc_traits.h>

namespace MyGrate::Input {
	MySQLRecordSet::MySQLRecordSet(StmtPtr s) :
		stmt {std::move(s)}, stmtres {nullptr, nullptr}, fields(mysql_stmt_field_count(stmt.get())),
		extras(fields.size())
	{
		auto getBind = [](const MYSQL_FIELD & f, MYSQL_BIND & b) -> std::unique_ptr<ResultData> {
			switch (f.type) {
				case MYSQL_TYPE_DECIMAL:
				case MYSQL_TYPE_NEWDECIMAL:
				case MYSQL_TYPE_DOUBLE:
					return std::make_unique<ResultDataT<double>>(b, f);
				case MYSQL_TYPE_FLOAT:
					return std::make_unique<ResultDataT<float>>(b, f);
				case MYSQL_TYPE_TINY:
					return std::make_unique<ResultDataT<int8_t>>(b, f);
				case MYSQL_TYPE_SHORT:
				case MYSQL_TYPE_YEAR:
					return std::make_unique<ResultDataT<int16_t>>(b, f);
				case MYSQL_TYPE_LONG:
				case MYSQL_TYPE_INT24:
					return std::make_unique<ResultDataT<int32_t>>(b, f);
				case MYSQL_TYPE_LONGLONG:
					return std::make_unique<ResultDataT<int64_t>>(b, f);
				case MYSQL_TYPE_NULL:
					return std::make_unique<ResultDataT<std::nullptr_t>>(b, f);
				case MYSQL_TYPE_TIMESTAMP:
				case MYSQL_TYPE_TIMESTAMP2:
				case MYSQL_TYPE_DATETIME:
				case MYSQL_TYPE_DATETIME2:
					return std::make_unique<ResultDataTime<DateTime>>(b, f);
				case MYSQL_TYPE_TIME:
				case MYSQL_TYPE_TIME2:
					return std::make_unique<ResultDataTime<Time>>(b, f);
				case MYSQL_TYPE_DATE:
				case MYSQL_TYPE_NEWDATE:
					return std::make_unique<ResultDataTime<Date>>(b, f);
				case MYSQL_TYPE_VARCHAR:
				case MYSQL_TYPE_VAR_STRING:
				case MYSQL_TYPE_STRING:
				case MYSQL_TYPE_JSON:
				case MYSQL_TYPE_ENUM:
					return std::make_unique<ResultDataT<std::string_view>>(b, f);
				case MYSQL_TYPE_TINY_BLOB:
				case MYSQL_TYPE_MEDIUM_BLOB:
				case MYSQL_TYPE_LONG_BLOB:
				case MYSQL_TYPE_BLOB:
					return std::make_unique<ResultDataT<Blob>>(b, f);
				case MAX_NO_FIELD_TYPES:
				case MYSQL_TYPE_BIT:
				case MYSQL_TYPE_SET:
				case MYSQL_TYPE_GEOMETRY:;
			}
			throw std::runtime_error("Unsupported column type");
		};
		ResPtr meta {mysql_stmt_result_metadata(stmt.get()), mysql_free_result};
		const auto fieldDefs = mysql_fetch_fields(meta.get());
		verify<std::runtime_error>(fieldDefs, "Fetch fields");
		for (std::size_t i = 0; i < fields.size(); i += 1) {
			extras[i] = getBind(fieldDefs[i], fields[i]);
		}
		verify<std::runtime_error>(!mysql_stmt_bind_result(stmt.get(), fields.data()), "Store result error");
		verify<std::runtime_error>(!mysql_stmt_store_result(stmt.get()), "Store result error");
		stmtres = {stmt.get(), mysql_stmt_free_result};
		verify<std::runtime_error>(!mysql_stmt_fetch(stmt.get()), "Fetch");
	}

	std::size_t
	MySQLRecordSet::rows() const
	{
		return mysql_stmt_num_rows(stmt.get());
	}

	std::size_t
	MySQLRecordSet::columns() const
	{
		return fields.size();
	}

	DbValue
	MySQLRecordSet::at(std::size_t row, std::size_t col) const
	{
		mysql_stmt_data_seek(stmt.get(), row);
		if (extras[col]->null) {
			return nullptr;
		}
		return extras[col]->getValue();
	}
}