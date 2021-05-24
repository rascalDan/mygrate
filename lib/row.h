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
		Row(const st_mariadb_rpl_rows_event &, const st_mariadb_rpl_table_map_event &);

	private:
		friend class RowPair;
		Row(const st_mariadb_rpl_rows_event &, const st_mariadb_rpl_table_map_event &, MyGrate::RawDataReader && md,
				MyGrate::RawDataReader && data);
		Row(const st_mariadb_rpl_rows_event &, const st_mariadb_rpl_table_map_event &, MyGrate::RawDataReader & md,
				MyGrate::RawDataReader & data);
	};

	class RowPair : public std::pair<Row, Row> {
	public:
		RowPair(const st_mariadb_rpl_rows_event &, const st_mariadb_rpl_table_map_event &);

	private:
		RowPair(const st_mariadb_rpl_rows_event &, const st_mariadb_rpl_table_map_event &,
				MyGrate::RawDataReader && md1, MyGrate::RawDataReader && md2, MyGrate::RawDataReader && data);
	};
}

#endif
