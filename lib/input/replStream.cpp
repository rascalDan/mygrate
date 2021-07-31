#include "replStream.h"
#include "mariadb_repl.h"
#include "mysqlConn.h"
#include <eventHandlerBase.h>
#include <eventHandlers.h>
#include <helpers.h>
#include <input/sql/selectBinLogFormat.h>
#include <memory>
#include <stdexcept>
#include <utility>

namespace MyGrate::Input {
	class ConfigError : public std::runtime_error {
		using std::runtime_error::runtime_error;
	};

	ReplicationStream::ReplicationStream(const std::string & host, const std::string & user, const std::string & pass,
			unsigned short port, uint64_t sid, std::string fn, uint64_t pos) :
		MySQLConn {host.c_str(), user.c_str(), pass.c_str(), port},
		serverid {sid}, filename {std::move(fn)}, position {pos}
	{
		query("SET @mariadb_slave_capability = 4");
		query("SET @master_binlog_checksum = @@global.binlog_checksum");
		verify<ConfigError>((**input::sql::selectBinLogFormat::execute(this)).get<std::string_view>() == "ROW",
				"@@binlog_format must be 'ROW'");
	}

	void
	ReplicationStream::readEvents(MyGrate::EventHandlerBase & eh)
	{
		using MariaDB_Rpl_Ptr = std::unique_ptr<MARIADB_RPL, decltype(&mariadb_rpl_close)>;
		auto rpl = MariaDB_Rpl_Ptr {mariadb_rpl_init(this), &mariadb_rpl_close};

		mariadb_rpl_optionsv(rpl.get(), MARIADB_RPL_SERVER_ID, serverid);
		mariadb_rpl_optionsv(rpl.get(), MARIADB_RPL_FILENAME, filename.c_str(), filename.length());
		mariadb_rpl_optionsv(rpl.get(), MARIADB_RPL_START, position);
		mariadb_rpl_optionsv(rpl.get(), MARIADB_RPL_FLAGS, MARIADB_RPL_BINLOG_SEND_ANNOTATE_ROWS);

		verify<MySQLErr>(!mariadb_rpl_open(rpl.get()), "Failed to mariadb_rpl_open", this);

		while (MyGrate::MariaDB_Event_Ptr event {mariadb_rpl_fetch(rpl.get(), nullptr), &mariadb_free_rpl_event}) {
			auto np = event->next_event_pos;
			if (const auto & h = eventHandlers.at(event->event_type); h.func) {
				(eh.*h.func)(std::move(event));
			}
			position = np;
		}
	}

	void
	ReplicationStream::stopEvents()
	{
		mariadb_cancel(this);
	}
}
