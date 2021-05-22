#ifndef MYGRATE_EVENTHANDLERBASE_H
#define MYGRATE_EVENTHANDLERBASE_H

#include "mariadb_repl.h"
#include <map>
#include <memory>

namespace MyGrate {
	using MariaDB_Event_Ptr = std::unique_ptr<MARIADB_RPL_EVENT, decltype(&mariadb_free_rpl_event)>;
	class EventHandlerBase {
	public:
		virtual void tableMap(MariaDB_Event_Ptr);
		virtual void insertRow(MariaDB_Event_Ptr);
		virtual void updateRow(MariaDB_Event_Ptr);
		virtual void deleteRow(MariaDB_Event_Ptr);

	protected:
		using TableId = decltype(st_mariadb_rpl_table_map_event::table_id);
		using TableMaps = std::map<TableId, MyGrate::MariaDB_Event_Ptr>;
		TableMaps tableMaps;
	};
}

#endif