#ifndef MYGRATE_DBRECORDSET_H
#define MYGRATE_DBRECORDSET_H

#include <dbTypes.h>
#include <memory>

namespace MyGrate {
	class RecordSet {
	public:
		virtual ~RecordSet() = default;

		virtual std::size_t rows() const = 0;
		virtual std::size_t columns() const = 0;
		virtual DbValue at(std::size_t, std::size_t) const = 0;
	};
	using RecordSetPtr = std::unique_ptr<RecordSet>;
}

#endif
