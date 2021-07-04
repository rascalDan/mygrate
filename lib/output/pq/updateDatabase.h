#ifndef MYGRATE_OUTPUT_PQ_UPDATEDATABASE_H
#define MYGRATE_OUTPUT_PQ_UPDATEDATABASE_H

#include "pqConn.h"
#include <cstdint>
#include <eventHandlerBase.h>
#include <eventSourceBase.h>

namespace MyGrate::Input {
	class MySQLConn;
}
namespace MyGrate::Output::Pq {
	struct ColumnDef {
		ColumnDef(std::string, std::size_t, bool);

		std::string name;
		std::size_t ordinal;
		bool is_pk;
	};
	using ColumnDefPtr = std::unique_ptr<ColumnDef>;

	class TableDef {
	public:
		TableDef() = default;
		TableDef(const RecordSet &, std::string_view name);

		std::vector<ColumnDefPtr> columns;
	};
	using TableDefPtr = std::unique_ptr<TableDef>;

	class ConfigError : public std::runtime_error {
		using std::runtime_error::runtime_error;
	};

	class UpdateDatabase : public PqConn, public EventHandlerBase {
	public:
		UpdateDatabase(const char * const str, uint64_t source);

		EventSourceBasePtr getSource();

		static UpdateDatabase createNew(PqConn *, const char * host, const char * un, const char * pw, unsigned short p,
				const char * db, int sid, const char * sc);

		void addTable(Input::MySQLConn *, const char * tableName);
		void copyTableContent(Input::MySQLConn *, const char * tableName);

		const uint64_t source;
		const std::string schema;

	private:
		std::map<std::string, TableDefPtr, std::less<>> tables;
	};
}

#endif
