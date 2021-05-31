#include "dumpToConsole.h"
#include "eventHandlerBase.h"
#include "mariadb_repl.h"
#include <algorithm>
#include <compileTimeFormatter.h>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <row.h>
#include <streamSupport.h>
#include <utility>
#include <variant>

namespace MyGrate::Output {
	void
	DumpToConsole::tableMap(MyGrate::MariaDB_Event_Ptr event)
	{
		const auto & tm = event->event.table_map;
		scprintf<"Table map %?.%? -> %?\n">(std::cout, tm.database, tm.table, tm.table_id);
		EventHandlerBase::tableMap(std::move(event));
	}

	void
	DumpToConsole::insertRow(MyGrate::MariaDB_Event_Ptr event)
	{
		const auto & rs = event->event.rows;
		scprintf<"Insert into %?\n">(std::cout, rs.table_id);
		dumpRowData(event->event.rows);
	}

	void
	DumpToConsole::updateRow(MyGrate::MariaDB_Event_Ptr event)
	{
		const auto & rs = event->event.rows;
		scprintf<"Update %?\n">(std::cout, rs.table_id);
		dumpRowPairData(event->event.rows);
	}

	void
	DumpToConsole::deleteRow(MyGrate::MariaDB_Event_Ptr event)
	{
		const auto & rs = event->event.rows;
		scprintf<"Delete from %?\n">(std::cout, rs.table_id);
		dumpRowData(event->event.rows);
	}

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

	void
	DumpToConsole::dumpRowData(const st_mariadb_rpl_rows_event & row) const
	{
		Row r {row, tableMaps.at(row.table_id)->event.table_map};
		std::for_each(r.begin(), r.end(), [](auto && fv) {
			std::visit(write {}, fv);
		});
	}

	void
	DumpToConsole::dumpRowPairData(const st_mariadb_rpl_rows_event & row) const
	{
		RowPair rp {row, tableMaps.at(row.table_id)->event.table_map};
		std::for_each(rp.first.begin(), rp.first.end(), [](auto && fv) {
			std::visit(write {}, fv);
		});
		std::for_each(rp.second.begin(), rp.second.end(), [](auto && fv) {
			std::visit(write {}, fv);
		});
	}

}
