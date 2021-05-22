#ifndef MYGRATE_OUTPUT_DUMPTOCONSOLE_H
#define MYGRATE_OUTPUT_DUMPTOCONSOLE_H

#include <eventHandlerBase.h>

namespace MyGrate::Output {
	class DumpToConsole : public MyGrate::EventHandlerBase {
	protected:
		void tableMap(MyGrate::MariaDB_Event_Ptr event) override;

		void insertRow(MyGrate::MariaDB_Event_Ptr event) override;

		void updateRow(MyGrate::MariaDB_Event_Ptr event) override;

		void deleteRow(MyGrate::MariaDB_Event_Ptr event) override;

	private:
		void dumpRowData(const st_mariadb_rpl_rows_event & row) const;
		void dumpRowPairData(const st_mariadb_rpl_rows_event & row) const;
	};
}

#endif
