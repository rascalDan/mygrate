#include "mysqlRecordSet.h"
#include "mysqlBindings.h"
#include "mysqlConn.h"
#include "mysqlStmt.h"
#include <cstdint>
#include <dbTypes.h>
#include <helpers.h>
#include <stdexcept>
#include <string_view>
#include <utility>
// IWYU pragma: no_include <ext/alloc_traits.h>

namespace MyGrate::Input {
	MySQLData::MySQLData(StmtPtr s) :
		stmt {std::move(s)}, fields(mysql_stmt_field_count(stmt.get())), extras(fields.size())
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
				case MYSQL_TYPE_TINY_BLOB:
				case MYSQL_TYPE_MEDIUM_BLOB:
				case MYSQL_TYPE_LONG_BLOB:
				case MYSQL_TYPE_BLOB:
					if (f.charsetnr == 63)
						return std::make_unique<ResultDataT<Blob>>(b, f);
					else
						return std::make_unique<ResultDataT<std::string_view>>(b, f);
				case MAX_NO_FIELD_TYPES:
				case MYSQL_TYPE_BIT:
				case MYSQL_TYPE_SET:
				case MYSQL_TYPE_GEOMETRY:;
			}
			throw std::logic_error("Unsupported column type");
		};
		ResPtr meta {mysql_stmt_result_metadata(stmt.get()), mysql_free_result};
		const auto fieldDefs = verify<MySQLErr>(mysql_fetch_fields(meta.get()), "Fetch fields", stmt->mysql);
		for (std::size_t i = 0; i < fields.size(); i += 1) {
			extras[i] = getBind(fieldDefs[i], fields[i]);
		}
		verify<MySQLErr>(!mysql_stmt_bind_result(stmt.get(), fields.data()), "Store result error", stmt->mysql);
	}

	std::size_t
	MySQLData::columns() const
	{
		return fields.size();
	}

	MySQLRecordSet::MySQLRecordSet(StmtPtr s) : MySQLData(std::move(s)), stmtres {nullptr, nullptr}
	{
		verify<MySQLErr>(!mysql_stmt_store_result(stmt.get()), "Store result error", stmt->mysql);
		stmtres = {stmt.get(), mysql_stmt_free_result};
	}

	std::size_t
	MySQLRecordSet::rows() const
	{
		return mysql_stmt_num_rows(stmt.get());
	}

	std::size_t
	MySQLRecordSet::columns() const
	{
		return MySQLData::columns();
	}

	DbValue
	MySQLRecordSet::at(std::size_t row, std::size_t col) const
	{
		if (currentRow != row) {
			mysql_stmt_data_seek(stmt.get(), row);
			verify<MySQLErr>(!mysql_stmt_fetch(stmt.get()), "Fetch", stmt->mysql);
			currentRow = row;
		}
		if (extras[col]->null) {
			return nullptr;
		}
		return extras[col]->getValue();
	}
}
