#include "eventHandlerBase.h"
#include <utility>

namespace MyGrate {
	// LCOV_EXCL_START not testing these :)
	void EventHandlerBase::tableMap(MyGrate::MariaDB_Event_Ptr) { }
	void EventHandlerBase::insertRow(MariaDB_Event_Ptr) { }
	void EventHandlerBase::updateRow(MariaDB_Event_Ptr) { }
	void EventHandlerBase::deleteRow(MariaDB_Event_Ptr) { }
	void EventHandlerBase::rotate(MariaDB_Event_Ptr) { }
	void EventHandlerBase::gtid(MariaDB_Event_Ptr) { }
	void EventHandlerBase::xid(MariaDB_Event_Ptr) { }
	// LCOV_EXCL_STOP
}
