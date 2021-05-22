#include "eventHandlerBase.h"
#include <type_traits>
#include <utility>

namespace MyGrate {
	void
	EventHandlerBase::tableMap(MyGrate::MariaDB_Event_Ptr event)
	{
		tableMaps.insert_or_assign(event->event.table_map.table_id, std::move(event));
	}

	void EventHandlerBase::insertRow(MariaDB_Event_Ptr) { }
	void EventHandlerBase::updateRow(MariaDB_Event_Ptr) { }
	void EventHandlerBase::deleteRow(MariaDB_Event_Ptr) { }
}
