#ifndef MYGRATE_OUTPUT_PQ_PQRECORDSET_H
#define MYGRATE_OUTPUT_PQ_PQRECORDSET_H

#include "dbRecordSet.h"
#include "dbTypes.h"
#include "pqStmt.h"
#include <cstddef>

namespace MyGrate::Output::Pq {
	class PqRecordSet : public RecordSet {
	public:
		explicit PqRecordSet(ResPtr r);

		std::size_t rows() const override;

		std::size_t columns() const override;

		DbValue at(std::size_t row, std::size_t col) const override;

	private:
		ResPtr res;
	};
}

#endif
