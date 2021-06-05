#include "dbRecordSet.h"

namespace MyGrate {
	RowView::RowView(const RecordSet * rs, std::size_t rw) : recordSet {rs}, row {rw} { }

	DbValue
	RowView::operator[](std::size_t col) const
	{
		return recordSet->at(row, col);
	}

	RowView
	RecordSet::operator[](std::size_t row) const
	{
		return {this, row};
	}
}
