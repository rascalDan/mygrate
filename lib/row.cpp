#include "row.h"
#include "bitset.h"
#include "mariadb_repl.h"
#include "mysql_types.h"
#include "rawDataReader.h"
#include <cstddef>
#include <mysql.h>
#include <span>
#include <stdexcept>
#include <string>

namespace MyGrate {
	Row::Row(const st_mariadb_rpl_rows_event & row, const st_mariadb_rpl_table_map_event & tm,
			MyGrate::RawDataReader & md, MyGrate::RawDataReader & data)
	{
		reserve(tm.column_count);
		const size_t flagBytes {(tm.column_count + 7) / 8};
		MyGrate::BitSet nullFlags {data.viewValue<std::span<const std::byte>>(flagBytes)};
		MyGrate::BitSet columnFlags {{reinterpret_cast<const std::byte *>(row.column_bitmap), flagBytes}};
		auto nullIter {nullFlags.begin()};
		auto colIter {columnFlags.begin()};
		for (auto c {0U}; c < tm.column_count; c++) {
			if (*colIter) {
				const enum_field_types type {(enum_field_types)(unsigned char)tm.column_types.str[c]};
				const auto null {*nullIter};
				if (null) {
					switch (type) {
						case MYSQL_TYPE_BIT:
						case MYSQL_TYPE_ENUM:
						case MYSQL_TYPE_SET:
						case MYSQL_TYPE_NEWDECIMAL:
						case MYSQL_TYPE_DECIMAL:
						case MYSQL_TYPE_VARCHAR:
						case MYSQL_TYPE_VAR_STRING:
						case MYSQL_TYPE_STRING:
							md.discard(2);
							break;
						case MYSQL_TYPE_TINY_BLOB:
						case MYSQL_TYPE_MEDIUM_BLOB:
						case MYSQL_TYPE_LONG_BLOB:
						case MYSQL_TYPE_BLOB:
						case MYSQL_TYPE_FLOAT:
						case MYSQL_TYPE_DOUBLE:
						case MYSQL_TYPE_TIMESTAMP2:
						case MYSQL_TYPE_DATETIME2:
						case MYSQL_TYPE_TIME2:
							md.discard(1);
							break;
						default:;
					}
					emplace_back(nullptr);
				}
				else {
					switch (type) {
#define TYPE_CALLBACK(T) \
	case T: \
		emplace_back(MySQL::Type<T>::read(md, data)); \
		break
						TYPE_CALLBACK(MYSQL_TYPE_DECIMAL);
						TYPE_CALLBACK(MYSQL_TYPE_TINY);
						TYPE_CALLBACK(MYSQL_TYPE_SHORT);
						TYPE_CALLBACK(MYSQL_TYPE_LONG);
						TYPE_CALLBACK(MYSQL_TYPE_FLOAT);
						TYPE_CALLBACK(MYSQL_TYPE_DOUBLE);
						TYPE_CALLBACK(MYSQL_TYPE_NULL);
						TYPE_CALLBACK(MYSQL_TYPE_TIMESTAMP);
						TYPE_CALLBACK(MYSQL_TYPE_LONGLONG);
						TYPE_CALLBACK(MYSQL_TYPE_INT24);
						TYPE_CALLBACK(MYSQL_TYPE_DATE);
						TYPE_CALLBACK(MYSQL_TYPE_TIME);
						TYPE_CALLBACK(MYSQL_TYPE_DATETIME);
						TYPE_CALLBACK(MYSQL_TYPE_YEAR);
						TYPE_CALLBACK(MYSQL_TYPE_NEWDATE);
						TYPE_CALLBACK(MYSQL_TYPE_BIT);
						TYPE_CALLBACK(MYSQL_TYPE_TIMESTAMP2);
						TYPE_CALLBACK(MYSQL_TYPE_TIME2);
						TYPE_CALLBACK(MYSQL_TYPE_JSON);
						TYPE_CALLBACK(MYSQL_TYPE_NEWDECIMAL);
						TYPE_CALLBACK(MYSQL_TYPE_ENUM);
						TYPE_CALLBACK(MYSQL_TYPE_SET);
						TYPE_CALLBACK(MYSQL_TYPE_TINY_BLOB);
						TYPE_CALLBACK(MYSQL_TYPE_MEDIUM_BLOB);
						TYPE_CALLBACK(MYSQL_TYPE_LONG_BLOB);
						TYPE_CALLBACK(MYSQL_TYPE_BLOB);
						TYPE_CALLBACK(MYSQL_TYPE_GEOMETRY);
						TYPE_CALLBACK(MYSQL_TYPE_VAR_STRING);
						TYPE_CALLBACK(MYSQL_TYPE_VARCHAR);
						TYPE_CALLBACK(MYSQL_TYPE_DATETIME2);
						TYPE_CALLBACK(MYSQL_TYPE_STRING);
#undef TYPE_CALLBACK
						case MAX_NO_FIELD_TYPES:
							// This is why you don't the end of the enum as member!
#ifdef NDEBUG
							[[fallthrough]];
						default:
#endif
							throw std::logic_error("Unknown type: " + std::to_string(type));
					}
				}
				++nullIter;
			}
			++colIter;
		}
	}

	Row::Rows
	Row::fromRowsEvent(const st_mariadb_rpl_rows_event & row, const st_mariadb_rpl_table_map_event & tm)
	{
		Rows rtn;
		RawDataReader data {row.row_data, row.row_data_size};
		while (data.more()) {
			RawDataReader md {tm.metadata};
			rtn.emplace_back(row, tm, md, data);
		}
		return rtn;
	}
}
