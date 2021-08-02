#ifndef MYGRATE_ROW_H
#define MYGRATE_ROW_H

#include "dbTypes.h"
#include <utility>
#include <vector>
namespace MyGrate {
	class RawDataReader;
}
struct st_mariadb_rpl_rows_event;
struct st_mariadb_rpl_table_map_event;

namespace MyGrate {
	class Row : public std::vector<DbValue> {
	public:
		using Rows = std::vector<Row>;
		static Rows fromRowsEvent(const st_mariadb_rpl_rows_event &, const st_mariadb_rpl_table_map_event &);

		using std::vector<DbValue>::vector;
		Row(const st_mariadb_rpl_rows_event &, const st_mariadb_rpl_table_map_event &, MyGrate::RawDataReader & md,
				MyGrate::RawDataReader & data);
	};
}

#endif
