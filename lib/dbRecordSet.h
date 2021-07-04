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

		template<typename S, std::size_t N, std::size_t O = 0>
		auto
		create() const
		{
			return create<S, O>(std::make_index_sequence<N> {});
		}

		template<typename S, std::size_t O, std::size_t... I> auto create(std::index_sequence<I...>) const
		{
			return std::make_unique<S>((*this)[I + O]...);
		}

		bool operator==(const RowView &) const;
		RowView & operator++();
		const RowView & operator*() const;
		std::size_t currentRow() const;

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
		DbValue operator*() const;

		RowView begin() const;
		RowView end() const;
	};
	using RecordSetPtr = std::unique_ptr<RecordSet>;

	class Cursor {
	public:
		virtual ~Cursor() = default;

		virtual bool fetch() = 0;
		virtual std::size_t columns() const = 0;
		virtual DbValue at(std::size_t) const = 0;
	};
	using CursorPtr = std::unique_ptr<Cursor>;
}

#endif
