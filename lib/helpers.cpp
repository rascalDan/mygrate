#include "helpers.h"

namespace MyGrate {
	static_assert(std::is_same_v<SmallestUInt<1>, uint8_t>);
	static_assert(std::is_same_v<SmallestUInt<8>, uint8_t>);
	static_assert(std::is_same_v<SmallestUInt<9>, uint16_t>);
	static_assert(std::is_same_v<SmallestUInt<16>, uint16_t>);
	static_assert(std::is_same_v<SmallestUInt<17>, uint32_t>);
	static_assert(std::is_same_v<SmallestUInt<32>, uint32_t>);
	static_assert(std::is_same_v<SmallestUInt<33>, uint64_t>);
}
