#include "updateDatabase.h"
#include "pqConn.h"
#include <cstdint>
#include <dbRecordSet.h>
#include <eventSourceBase.h>
#include <helpers.h>
#include <memory>
#include <output/pq/sql/selectSource.h>
#include <stdexcept>

namespace MyGrate::Output::Pq {
	UpdateDatabase::UpdateDatabase(const char * const str, uint64_t s) : PqConn {str}, source {s} { }

	EventSourceBasePtr
	UpdateDatabase::getSource()
	{
		auto srcrec = output::pq::sql::selectSource::execute(this, source);
		verify<std::runtime_error>(srcrec->rows() == 1, "Wrong number of source config rows");
		return {};
	}
}
