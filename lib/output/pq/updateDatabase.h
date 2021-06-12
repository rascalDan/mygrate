#ifndef MYGRATE_OUTPUT_PQ_UPDATEDATABASE_H
#define MYGRATE_OUTPUT_PQ_UPDATEDATABASE_H

#include "pqConn.h"
#include <cstdint>
#include <eventHandlerBase.h>
#include <eventSourceBase.h>

namespace MyGrate::Output::Pq {
	struct ColumnDef {
		std::string name;
		std::size_t ordinal;
		bool is_pk;
	};
	using ColumnDefPtr = std::unique_ptr<ColumnDef>;

	class TableDef {
	public:
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

	private:
		uint64_t source;
		std::map<std::string, TableDefPtr, std::less<>> tables;
	};
}

#endif
