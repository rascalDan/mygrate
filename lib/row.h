#ifndef MYGRATE_ROW_H
#define MYGRATE_ROW_H

#include "mysql_types.h"
#include "rawDataReader.h"
#include <functional>

namespace MyGrate {
	class Row {
	public:
		Row(const st_mariadb_rpl_rows_event &, const st_mariadb_rpl_table_map_event &);

		void forEachField(const std::function<void(MySQL::FieldValue)> & callback);

	private:
		const st_mariadb_rpl_rows_event & row;
		const st_mariadb_rpl_table_map_event & tm;
	};
}

#endif
