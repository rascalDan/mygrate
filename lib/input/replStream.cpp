#include "replStream.h"
#include "mariadb_repl.h"
#include "mysqlConn.h"
#include <eventHandlerBase.h>
#include <eventHandlers.h>
#include <helpers.h>
#include <memory>
#include <stdexcept>
#include <utility>

namespace MyGrate::Input {
	ReplicationStream::ReplicationStream(const char * const host, const char * const user, const char * const pass,
			unsigned short port, uint64_t sid, std::string fn, uint64_t pos) :
		MySQLConn {host, user, pass, port},
		serverid {sid}, filename {std::move(fn)}, position {pos}
	{
	}

	void
	ReplicationStream::readEvents(MyGrate::EventHandlerBase & eh)
	{
		using MariaDB_Rpl_Ptr = std::unique_ptr<MARIADB_RPL, decltype(&mariadb_rpl_close)>;
		auto rpl = MariaDB_Rpl_Ptr {mariadb_rpl_init(this), &mariadb_rpl_close};

		query("SET @mariadb_slave_capability = 4");
		query("SET @master_binlog_checksum = @@global.binlog_checksum");

		mariadb_rpl_optionsv(rpl.get(), MARIADB_RPL_SERVER_ID, serverid);
		mariadb_rpl_optionsv(rpl.get(), MARIADB_RPL_FILENAME, filename.c_str());
		mariadb_rpl_optionsv(rpl.get(), MARIADB_RPL_START, position);
		mariadb_rpl_optionsv(rpl.get(), MARIADB_RPL_FLAGS, MARIADB_RPL_BINLOG_SEND_ANNOTATE_ROWS);

		verify<std::runtime_error>(!mariadb_rpl_open(rpl.get()), "Failed to mariadb_rpl_open");

		while (MyGrate::MariaDB_Event_Ptr event {mariadb_rpl_fetch(rpl.get(), nullptr), &mariadb_free_rpl_event}) {
			position = event->next_event_pos;
			if (const auto & h = eventHandlers.at(event->event_type); h.func) {
				(eh.*h.func)(std::move(event));
			}
		}
	}
}
