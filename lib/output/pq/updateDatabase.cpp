#include "updateDatabase.h"
#include "pqConn.h"
#include "typeMapper.h"
#include <compileTimeFormatter.h>
#include <cstdint>
#include <dbRecordSet.h>
#include <eventSourceBase.h>
#include <helpers.h>
#include <input/mysqlConn.h>
#include <input/replStream.h>
#include <input/sql/selectColumns.h>
#include <input/sql/showMasterStatus.h>
#include <memory>
#include <output/pq/sql/insertColumn.h>
#include <output/pq/sql/insertSource.h>
#include <output/pq/sql/insertTable.h>
#include <output/pq/sql/selectColumns.h>
#include <output/pq/sql/selectSource.h>
#include <output/pq/sql/selectSourceSchema.h>
#include <output/pq/sql/selectTables.h>
#include <stdexcept>
#include <streamSupport.h>

namespace MyGrate::Output::Pq {
	UpdateDatabase::UpdateDatabase(const char * const str, uint64_t s) :
		PqConn {str}, source {s}, schema(**output::pq::sql::selectSourceSchema::execute(this, s))
	{
		auto trecs = output::pq::sql::selectTables::execute(this, source);
		auto crecs = output::pq::sql::selectColumns::execute(this, source);
		for (auto t {0U}; t < trecs->rows(); t++) {
			tables.emplace(trecs->at(t, 0), std::make_unique<TableDef>(*crecs, trecs->at(t, 0)));
		}
	}

	EventSourceBasePtr
	UpdateDatabase::getSource()
	{
		auto srcrec = output::pq::sql::selectSource::execute(this, source);
		verify<ConfigError>(srcrec->rows() == 1, "Wrong number of source config rows");
		return (*srcrec)[0].create<Input::ReplicationStream, 7>();
	}

	TableDef::TableDef(const RecordSet & crecs, std::string_view name)
	{
		for (auto c {0U}; c < crecs.rows(); c++) {
			if (crecs.at(c, 0) == name) {
				columns.emplace_back(crecs[c].create<ColumnDef, 3, 1>());
			}
		}
	}

	UpdateDatabase
	UpdateDatabase::createNew(PqConn * pq, const char * host, const char * username, const char * password,
			unsigned short port, const char * db, int sid, const char * schema)
	{
		Input::MySQLConn my {host, username, password, port};
		auto ms = input::sql::showMasterStatus::execute(&my);
		auto source_id = Tx {pq}([&]() {
			pq->query(scprintf<"CREATE SCHEMA IF NOT EXISTS %?">(schema).c_str());
			return **output::pq::sql::insertSource::execute(
					pq, host, username, password, port, db, ms->at(0, 0), ms->at(0, 1), sid, schema);
		});
		return UpdateDatabase(pq->connstr.c_str(), source_id);
	}

	void
	UpdateDatabase::addTable(Input::MySQLConn * conn, const char * tableName)
	{
		auto cols = input::sql::selectColumns::execute(conn, tableName);
		verify<std::logic_error>(cols->rows() > 0, "Table has no rows");
		Tx {this}([&] {
			const auto table_id = **output::pq::sql::insertTable::execute(this, tableName, source);
			std::stringstream ct;
			scprintf<"CREATE TABLE %?.%?(">(ct, schema, tableName);
			TypeMapper tm;
			for (auto col : *cols) {
				output::pq::sql::insertColumn::execute(this, col[0], col.currentRow(), table_id);
				if (col.currentRow())
					ct << ',';
				scprintf<"%? %?">(ct, col[0], tm.map(col[2], scprintf<"%?.%?">(tableName, col[0])));
				if (!col[1])
					ct << " not null";
				if (col[3])
					ct << " primary key";
			}
			ct << ")";
			this->query(ct.str().c_str());
		});
	}
}
