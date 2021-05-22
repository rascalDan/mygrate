#include "rawDataReader.h"

namespace MyGrate {
	RawDataReader::RawDataReader(const void * const d, std::size_t l) :
		rawdata {static_cast<const std::byte *>(d)}, len {l}
	{
	}

	RawDataReader::RawDataReader(const MARIADB_STRING & str) : RawDataReader {str.str, str.length} { }

	void
	RawDataReader::offsetSizeCheck(size_t l) const
	{
		if (offset + l > len) {
			throw std::range_error("Read beyond end of data");
		}
	}

	void
	RawDataReader::discard(size_t d)
	{
		offset += d;
	}

	template<>
	uint64_t
	RawDataReader::readValue<PackedInteger>()
	{
		switch (const auto byte1 {readValue<uint8_t>()}) {
			case 0 ... 250: // The value as-is
				return byte1;
			case 252: // 2 bytes
				return readValue<uint16_t, 2>();
			case 253: // 3 bytes
				return readValue<uint32_t, 3>();
			case 254: // 8 bytes
				return readValue<uint64_t, 8>();
			default:;
		}
		throw std::domain_error("Invalid first byte of packed integer");
	}
}
