#include <cstddef>
#include <mysql.h>

#include <array>
#include <bitset>
#include <cmath>
#include <compileTimeFormatter.h>
#include <iomanip>
#include <map>
#include <mariadb_rpl.h>
#include <memory>
#include <vector>

#include <bitset.h>
#include <rawDataReader.h>
#include <row.h>
#include <streamSupport.h>

using namespace AdHoc;
using MariaDB_Rpl_Ptr = std::unique_ptr<MARIADB_RPL, decltype(&mariadb_rpl_close)>;
using MariaDB_Event_Ptr = std::unique_ptr<MARIADB_RPL_EVENT, decltype(&mariadb_free_rpl_event)>;

struct EventHandler {
	std::string_view name;
	void (*func)(MariaDB_Event_Ptr);
};
using EventHandlers = std::array<EventHandler, ENUM_END_EVENT>;

using TableId = decltype(st_mariadb_rpl_table_map_event::table_id);
using TableMaps = std::map<TableId, MariaDB_Event_Ptr>;
TableMaps tableMaps;

struct write {
	template<typename T>
	void
	operator()(const T & v) const
	{
		scprintf<"\t\t%?\n">(std::cout, v);
	}
	void
	operator()(const uint8_t & v) const
	{
		scprintf<"\t\t%d\n">(std::cout, v);
	}
	void
	operator()(const int8_t & v) const
	{
		scprintf<"\t\t%d\n">(std::cout, v);
	}
};
static void
dumpRowData(const st_mariadb_rpl_rows_event & row)
{
	MyGrate::Row {row, tableMaps.at(row.table_id)->event.table_map}.forEachField([](auto && fv) {
		std::visit(write {}, fv);
	});
}

static void
doTableMap(MariaDB_Event_Ptr event)
{
	const auto & tm = event->event.table_map;
	AdHoc::scprintf<"Table map %?.%? -> %?\n">(std::cout, tm.database, tm.table, tm.table_id);
	// for (auto c = 0U; c < tm.column_types.length; c++) {
	// AdHoc::scprintf<"\t%#02hx\n">(std::cout, tm.column_types.str[c]);
	//}
	tableMaps.insert_or_assign(tm.table_id, std::move(event));
}

static void
doWrite(MariaDB_Event_Ptr event)
{
	const auto & rs = event->event.rows;
	AdHoc::scprintf<"Insert into %?\n">(std::cout, rs.table_id);
	dumpRowData(event->event.rows);
}

static void
doUpdate(MariaDB_Event_Ptr event)
{
	const auto & rs = event->event.rows;
	AdHoc::scprintf<"Update %?\n">(std::cout, rs.table_id);
	dumpRowData(event->event.rows);
}

static void
doDelete(MariaDB_Event_Ptr event)
{
	const auto & rs = event->event.rows;
	AdHoc::scprintf<"Delete from %?\n">(std::cout, rs.table_id);
	dumpRowData(event->event.rows);
}

constexpr EventHandlers eventHandlers {[]() {
	EventHandlers eh {};
	eh[UNKNOWN_EVENT] = {"UNKNOWN_EVENT", nullptr};
	eh[START_EVENT_V3] = {"START_EVENT_V3", nullptr};
	eh[QUERY_EVENT] = {"QUERY_EVENT", nullptr};
	eh[STOP_EVENT] = {"STOP_EVENT", nullptr};
	eh[ROTATE_EVENT] = {"ROTATE_EVENT", nullptr};
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
	eh[TABLE_MAP_EVENT] = {"TABLE_MAP_EVENT", doTableMap};
	eh[PRE_GA_WRITE_ROWS_EVENT] = {"PRE_GA_WRITE_ROWS_EVENT", nullptr};
	eh[PRE_GA_UPDATE_ROWS_EVENT] = {"PRE_GA_UPDATE_ROWS_EVENT", nullptr};
	eh[PRE_GA_DELETE_ROWS_EVENT] = {"PRE_GA_DELETE_ROWS_EVENT", nullptr};
	eh[WRITE_ROWS_EVENT_V1] = {"WRITE_ROWS_EVENT_V1", doWrite};
	eh[UPDATE_ROWS_EVENT_V1] = {"UPDATE_ROWS_EVENT_V1", doUpdate};
	eh[DELETE_ROWS_EVENT_V1] = {"DELETE_ROWS_EVENT_V1", doDelete};
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

static void
read_events(MYSQL * mysql)
{
	auto rpl = MariaDB_Rpl_Ptr {mariadb_rpl_init(mysql), &mariadb_rpl_close};

	mysql_query(mysql, "SET @mariadb_slave_capability=4");
	// mysql_query(mysql, "SET @slave_gtid_strict_mode=1");
	// mysql_query(mysql, "SET @slave_gtid_ignore_duplicates=1");
	mysql_query(mysql, "SET NAMES utf8");
	mysql_query(mysql, "SET @master_binlog_checksum= @@global.binlog_checksum");

	mariadb_rpl_optionsv(rpl.get(), MARIADB_RPL_SERVER_ID, 12);
	mariadb_rpl_optionsv(rpl.get(), MARIADB_RPL_FILENAME, "mariadb-bin.000242");
	mariadb_rpl_optionsv(rpl.get(), MARIADB_RPL_START, 4);
	mariadb_rpl_optionsv(rpl.get(), MARIADB_RPL_FLAGS, MARIADB_RPL_BINLOG_SEND_ANNOTATE_ROWS);

	if (mariadb_rpl_open(rpl.get())) {
		throw std::runtime_error("Failed to mariadb_rpl_open");
	}

	while (MariaDB_Event_Ptr event {mariadb_rpl_fetch(rpl.get(), nullptr), &mariadb_free_rpl_event}) {
		const auto & h = eventHandlers.at(event->event_type);
		if (h.func) {
			h.func(std::move(event));
		}
		else {
			std::cout << h.name << "\n";
		}
	}
}

int
main(int, char **)
{
	std::cout << std::boolalpha;
	MYSQL conn;
	mysql_init(&conn);
	if (!mysql_real_connect(&conn, "192.168.1.38", "repl", "r3pl", "",
				// NOLINTNEXTLINE(hicpp-signed-bitwise)
				3306, nullptr, CLIENT_LOCAL_FILES | CLIENT_MULTI_STATEMENTS)) {
		throw std::runtime_error("ConnectionError");
	}
	read_events(&conn);
	mysql_close(&conn);
}
