#include "updateDatabase.h"
#include "pqConn.h"
#include <cstdint>
#include <dbRecordSet.h>
#include <eventSourceBase.h>
#include <helpers.h>
#include <input/replStream.h>
#include <memory>
#include <output/pq/sql/selectColumns.h>
#include <output/pq/sql/selectSource.h>
#include <output/pq/sql/selectTables.h>
#include <stdexcept>

namespace MyGrate::Output::Pq {
	UpdateDatabase::UpdateDatabase(const char * const str, uint64_t s) : PqConn {str}, source {s}
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
		verify<std::runtime_error>(srcrec->rows() == 1, "Wrong number of source config rows");
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
}
