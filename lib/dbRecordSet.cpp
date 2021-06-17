#include "dbRecordSet.h"
#include "helpers.h"

namespace MyGrate {
	RowView::RowView(const RecordSet * rs, std::size_t rw) : recordSet {rs}, row {rw} { }

	DbValue
	RowView::operator[](std::size_t col) const
	{
		return recordSet->at(row, col);
	}

	bool
	RowView::operator==(const RowView & that) const
	{
		verify<std::logic_error>(this->recordSet == that.recordSet, "Cannot iterator between RecordSets");
		return this->row == that.row;
	}

	RowView &
	RowView::operator++()
	{
		row++;
		return *this;
	}

	const RowView &
	RowView::operator*() const
	{
		return *this;
	}

	std::size_t
	RowView::currentRow() const
	{
		return row;
	}

	RowView
	RecordSet::operator[](std::size_t row) const
	{
		return {this, row};
	}

	DbValue
	RecordSet::operator*() const
	{
		return at(0, 0);
	}

	RowView
	RecordSet::begin() const
	{
		return {this, 0};
	}

	RowView
	RecordSet::end() const
	{
		return {this, rows()};
	}
}
