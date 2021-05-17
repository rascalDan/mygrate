#include "bitset.h"

namespace MyGrate {
	static_assert(std::forward_iterator<BitSet::Iterator>);

	BitSet::Iterator::Iterator(const std::byte * bb) : b {bb} { }

	bool
	BitSet::Iterator::operator*() const
	{
		return (*b & m) != std::byte {};
	}

	bool
	BitSet::Iterator::operator==(const BitSet::Iterator & o) const
	{
		return b == o.b && m == o.m;
	}

	bool
	BitSet::Iterator::operator!=(const BitSet::Iterator & o) const
	{
		return b != o.b || m != o.m;
	}

	BitSet::Iterator &
	BitSet::Iterator::operator++()
	{
		if (m == std::byte {128}) {
			b++;
			m = std::byte {1};
		}
		else {
			m <<= 1;
		}
		return *this;
	}

	BitSet::Iterator
	BitSet::Iterator::operator++(int)
	{
		auto pre {*this};
		++*this;
		return pre;
	}

	BitSet::BitSet(const std::span<const std::byte> bs) : bytes {bs} { }

	std::size_t
	BitSet::size() const
	{
		return bytes.size() * 8;
	}

	BitSet::Iterator
	BitSet::begin() const
	{
		return BitSet::Iterator {bytes.data()};
	}

	BitSet::Iterator
	BitSet::end() const
	{
		return BitSet::Iterator {bytes.data() + bytes.size()};
	}
}
