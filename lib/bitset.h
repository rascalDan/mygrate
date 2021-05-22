#ifndef MYGRATE_BITSET_H
#define MYGRATE_BITSET_H

#include <cstddef>
#include <iterator>
#include <span>

namespace MyGrate {
	class BitSet {
	public:
		class Iterator {
		public:
			using iterator_category = std::forward_iterator_tag;
			using difference_type = int;
			using value_type = bool;
			using pointer = bool *;
			using reference = bool;

			explicit Iterator();
			explicit Iterator(const std::byte *);

			bool operator*() const;
			bool operator!=(const Iterator &) const;
			bool operator==(const Iterator &) const;

			Iterator & operator++();
			Iterator operator++(int);

		private:
			const std::byte * b;
			std::byte m {1};
		};

		explicit BitSet(const std::span<const std::byte> b);

		std::size_t size() const;
		Iterator begin() const;
		Iterator end() const;

	private:
		const std::span<const std::byte> bytes;
	};
}

#endif
