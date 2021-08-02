#ifndef MYGRATE_OUTPUT_DUMPTOCONSOLE_H
#define MYGRATE_OUTPUT_DUMPTOCONSOLE_H

#include <eventHandlerBase.h>
struct st_mariadb_rpl_rows_event;

namespace MyGrate::Output {
	class DumpToConsole : public MyGrate::EventHandlerBase {
	protected:
		void tableMap(MyGrate::MariaDB_Event_Ptr event) override;

		void insertRow(MyGrate::MariaDB_Event_Ptr event) override;

		void updateRow(MyGrate::MariaDB_Event_Ptr event) override;

		void deleteRow(MyGrate::MariaDB_Event_Ptr event) override;

	private:
		using TableId = decltype(st_mariadb_rpl_table_map_event::table_id);
		using TableMaps = std::map<TableId, MyGrate::MariaDB_Event_Ptr>;

		void dumpRowData(const st_mariadb_rpl_rows_event & row) const;

		TableMaps tableMaps;
	};
}

#endif
