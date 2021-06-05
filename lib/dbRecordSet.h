#ifndef MYGRATE_DBRECORDSET_H
#define MYGRATE_DBRECORDSET_H

#include <dbTypes.h>
#include <memory>

namespace MyGrate {
	class RecordSet;

	class RowView {
	public:
		RowView(const RecordSet * rs, std::size_t rw = 0);

		DbValue operator[](std::size_t col) const;

		template<typename S, std::size_t N, typename Indices = std::make_index_sequence<N>>
		auto
		create() const
		{
			return create<S>(Indices {});
		}

		template<typename S, std::size_t... I> auto create(std::index_sequence<I...>) const
		{
			return std::make_unique<S>((*this)[I]...);
		}

	private:
		const RecordSet * recordSet;
		std::size_t row;
	};

	class RecordSet {
	public:
		virtual ~RecordSet() = default;

		virtual std::size_t rows() const = 0;
		virtual std::size_t columns() const = 0;
		virtual DbValue at(std::size_t, std::size_t) const = 0;
		RowView operator[](std::size_t row) const;
	};
	using RecordSetPtr = std::unique_ptr<RecordSet>;
}

#endif
