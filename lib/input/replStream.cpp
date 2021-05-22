#include "replStream.h"
#include "../eventHandlers.h"
#include "../mariadb_repl.h"

namespace MyGrate::Input {
	void
	ReplicationStream::readEvents(MyGrate::EventHandlerBase & eh)
	{
		using MariaDB_Rpl_Ptr = std::unique_ptr<MARIADB_RPL, decltype(&mariadb_rpl_close)>;
		auto rpl = MariaDB_Rpl_Ptr {mariadb_rpl_init(this), &mariadb_rpl_close};

		mysql_query(this, "SET @mariadb_slave_capability = 4");
		mysql_query(this, "SET @master_binlog_checksum = @@global.binlog_checksum");

		mariadb_rpl_optionsv(rpl.get(), MARIADB_RPL_SERVER_ID, 12);
		mariadb_rpl_optionsv(rpl.get(), MARIADB_RPL_FILENAME, "mariadb-bin.000242");
		mariadb_rpl_optionsv(rpl.get(), MARIADB_RPL_START, 4);
		mariadb_rpl_optionsv(rpl.get(), MARIADB_RPL_FLAGS, MARIADB_RPL_BINLOG_SEND_ANNOTATE_ROWS);

		if (mariadb_rpl_open(rpl.get())) {
			throw std::runtime_error("Failed to mariadb_rpl_open");
		}

		while (MyGrate::MariaDB_Event_Ptr event {mariadb_rpl_fetch(rpl.get(), nullptr), &mariadb_free_rpl_event}) {
			const auto & h = eventHandlers.at(event->event_type);
			if (h.func) {
				(eh.*h.func)(std::move(event));
			}
		}
	}

}
