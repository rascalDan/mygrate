#ifndef MYGRATE_EVENTHANDLERS_H
#define MYGRATE_EVENTHANDLERS_H

#include "eventHandlerBase.h"
#include <array>
#include <string_view>

namespace MyGrate {
	struct EventHandler {
		std::string_view name;
		void (MyGrate::EventHandlerBase::*func)(MyGrate::MariaDB_Event_Ptr);
	};
	using EventHandlers = std::array<EventHandler, ENUM_END_EVENT>;

	constexpr EventHandlers eventHandlers {[]() {
		EventHandlers eh {};
		eh[UNKNOWN_EVENT] = {"UNKNOWN_EVENT", nullptr};
		eh[START_EVENT_V3] = {"START_EVENT_V3", nullptr};
		eh[QUERY_EVENT] = {"QUERY_EVENT", nullptr};
		eh[STOP_EVENT] = {"STOP_EVENT", nullptr};
		eh[ROTATE_EVENT] = {"ROTATE_EVENT", &MyGrate::EventHandlerBase::rotate};
		eh[INTVAR_EVENT] = {"INTVAR_EVENT", nullptr};
		eh[LOAD_EVENT] = {"LOAD_EVENT", nullptr};
		eh[SLAVE_EVENT] = {"SLAVE_EVENT", nullptr};
		eh[CREATE_FILE_EVENT] = {"CREATE_FILE_EVENT", nullptr};
		eh[APPEND_BLOCK_EVENT] = {"APPEND_BLOCK_EVENT", nullptr};
		eh[EXEC_LOAD_EVENT] = {"EXEC_LOAD_EVENT", nullptr};
		eh[DELETE_FILE_EVENT] = {"DELETE_FILE_EVENT", nullptr};
		eh[NEW_LOAD_EVENT] = {"NEW_LOAD_EVENT", nullptr};
		eh[RAND_EVENT] = {"RAND_EVENT", nullptr};
		eh[USER_VAR_EVENT] = {"USER_VAR_EVENT", nullptr};
		eh[FORMAT_DESCRIPTION_EVENT] = {"FORMAT_DESCRIPTION_EVENT", nullptr};
		eh[XID_EVENT] = {"XID_EVENT", nullptr};
		eh[BEGIN_LOAD_QUERY_EVENT] = {"BEGIN_LOAD_QUERY_EVENT", nullptr};
		eh[EXECUTE_LOAD_QUERY_EVENT] = {"EXECUTE_LOAD_QUERY_EVENT", nullptr};
		eh[TABLE_MAP_EVENT] = {"TABLE_MAP_EVENT", &MyGrate::EventHandlerBase::tableMap};
		eh[PRE_GA_WRITE_ROWS_EVENT] = {"PRE_GA_WRITE_ROWS_EVENT", nullptr};
		eh[PRE_GA_UPDATE_ROWS_EVENT] = {"PRE_GA_UPDATE_ROWS_EVENT", nullptr};
		eh[PRE_GA_DELETE_ROWS_EVENT] = {"PRE_GA_DELETE_ROWS_EVENT", nullptr};
		eh[WRITE_ROWS_EVENT_V1] = {"WRITE_ROWS_EVENT_V1", &MyGrate::EventHandlerBase::insertRow};
		eh[UPDATE_ROWS_EVENT_V1] = {"UPDATE_ROWS_EVENT_V1", &MyGrate::EventHandlerBase::updateRow};
		eh[DELETE_ROWS_EVENT_V1] = {"DELETE_ROWS_EVENT_V1", &MyGrate::EventHandlerBase::deleteRow};
		eh[INCIDENT_EVENT] = {"INCIDENT_EVENT", nullptr};
		eh[HEARTBEAT_LOG_EVENT] = {"HEARTBEAT_LOG_EVENT", nullptr};
		eh[IGNORABLE_LOG_EVENT] = {"IGNORABLE_LOG_EVENT", nullptr};
		eh[ROWS_QUERY_LOG_EVENT] = {"ROWS_QUERY_LOG_EVENT", nullptr};
		eh[WRITE_ROWS_EVENT] = {"WRITE_ROWS_EVENT", nullptr};
		eh[UPDATE_ROWS_EVENT] = {"UPDATE_ROWS_EVENT", nullptr};
		eh[DELETE_ROWS_EVENT] = {"DELETE_ROWS_EVENT", nullptr};
		eh[GTID_LOG_EVENT] = {"GTID_LOG_EVENT", nullptr};
		eh[ANONYMOUS_GTID_LOG_EVENT] = {"ANONYMOUS_GTID_LOG_EVENT", nullptr};
		eh[PREVIOUS_GTIDS_LOG_EVENT] = {"PREVIOUS_GTIDS_LOG_EVENT", nullptr};
		eh[TRANSACTION_CONTEXT_EVENT] = {"TRANSACTION_CONTEXT_EVENT", nullptr};
		eh[VIEW_CHANGE_EVENT] = {"VIEW_CHANGE_EVENT", nullptr};
		eh[XA_PREPARE_LOG_EVENT] = {"XA_PREPARE_LOG_EVENT", nullptr};
		eh[MARIA_EVENTS_BEGIN] = {"MARIA_EVENTS_BEGIN", nullptr};
		eh[ANNOTATE_ROWS_EVENT] = {"ANNOTATE_ROWS_EVENT", nullptr};
		eh[BINLOG_CHECKPOINT_EVENT] = {"BINLOG_CHECKPOINT_EVENT", nullptr};
		eh[GTID_EVENT] = {"GTID_EVENT", nullptr};
		eh[GTID_LIST_EVENT] = {"GTID_LIST_EVENT", nullptr};
		eh[START_ENCRYPTION_EVENT] = {"START_ENCRYPTION_EVENT", nullptr};
		eh[QUERY_COMPRESSED_EVENT] = {"QUERY_COMPRESSED_EVENT", nullptr};
		eh[WRITE_ROWS_COMPRESSED_EVENT_V1] = {"WRITE_ROWS_COMPRESSED_EVENT_V1", nullptr};
		eh[UPDATE_ROWS_COMPRESSED_EVENT_V1] = {"UPDATE_ROWS_COMPRESSED_EVENT_V1", nullptr};
		eh[DELETE_ROWS_COMPRESSED_EVENT_V1] = {"DELETE_ROWS_COMPRESSED_EVENT_V1", nullptr};
		eh[WRITE_ROWS_COMPRESSED_EVENT] = {"WRITE_ROWS_COMPRESSED_EVENT", nullptr};
		eh[UPDATE_ROWS_COMPRESSED_EVENT] = {"UPDATE_ROWS_COMPRESSED_EVENT", nullptr};
		eh[DELETE_ROWS_COMPRESSED_EVENT] = {"DELETE_ROWS_COMPRESSED_EVENT", nullptr};
		return eh;
	}()};
}

#endif
