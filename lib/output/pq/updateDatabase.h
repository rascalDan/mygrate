#ifndef MYGRATE_OUTPUT_PQ_UPDATEDATABASE_H
#define MYGRATE_OUTPUT_PQ_UPDATEDATABASE_H

#include "pqConn.h"
#include "pqStmt.h"
#include <cstdint>
#include <eventCounter.h>
#include <eventHandlerBase.h>
#include <eventSourceBase.h>
#include <row.h>

namespace MyGrate::Input {
	class MySQLConn;
}
namespace MyGrate::Output::Pq {
	struct ColumnDef {
#ifndef __cpp_aggregate_paren_init
		ColumnDef(std::string, std::size_t, bool);
#endif

		std::string name;
		std::size_t ordinal;
		bool is_pk;
	};
	using ColumnDefPtr = std::unique_ptr<ColumnDef>;

	class TableOutput {
	public:
		TableOutput() = default;
		TableOutput(const RecordSet &, std::string_view name);

		std::vector<ColumnDefPtr> columns;
		std::size_t keys;
		DbPrepStmtPtr update, deleteFrom, insertInto;
	};
	using TableDefPtr = std::unique_ptr<TableOutput>;

	class ConfigError : public std::runtime_error {
		using std::runtime_error::runtime_error;
	};

	class ReplicationError : public std::runtime_error {
		using std::runtime_error::runtime_error;
	};

	class UpdateDatabase : public PqConn, public EventHandlerBase {
	public:
		UpdateDatabase(const char * const str, uint64_t source);

		EventSourceBasePtr getSource();

		static UpdateDatabase createNew(PqConn *, const char * host, const char * un, const char * pw, unsigned short p,
				const char * db, int sid, const char * sc);

		void addTable(Input::MySQLConn *, const char * tableName);

		// Replication events
		void updateRow(MariaDB_Event_Ptr) override;
		void deleteRow(MariaDB_Event_Ptr) override;
		void insertRow(MariaDB_Event_Ptr) override;
		void tableMap(MariaDB_Event_Ptr) override;
		void rotate(MariaDB_Event_Ptr) override;
		void gtid(MariaDB_Event_Ptr) override;
		void xid(MariaDB_Event_Ptr) override;

		const EventCounter & getProcessedCounts() const;

		const uint64_t source;
		const std::string schema;
		const std::string database;

	protected:
		virtual void beforeEvent(const MariaDB_Event_Ptr & e);
		virtual void afterEvent(const MariaDB_Event_Ptr & e);

	private:
		UpdateDatabase(PqConn &&, uint64_t source);
		UpdateDatabase(PqConn &&, uint64_t source, RecordSetPtr cfg);

		static void verifyRow(const MariaDB_Event_Ptr & e, const TableDefPtr &);
		static void copyKeys(const Row & r, const TableDefPtr &, std::back_insert_iterator<Row> &&);

		void copyTableContent(Input::MySQLConn *, const char * tableName, const TableDefPtr &);
		void copyIndexes(Input::MySQLConn *, const char * tableName);

		using Tables = std::map<std::string, TableDefPtr, std::less<>>;
		Tables tables;
		Tables::const_iterator selected;
		MariaDB_Event_Ptr table_map;
		bool intx {false};
		EventCounter processed;
	};
}

#endif
