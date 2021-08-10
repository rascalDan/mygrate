#include "writePqCopyStrm.h"
#include <cstdint>

namespace MyGrate::Output::Pq {

	WritePqCopyStream::WritePqCopyStream(FILE * o) : out {o} { }

	void
	WritePqCopyStream::nextRecord()
	{
		fputc('\n', out);
	}

	void
	WritePqCopyStream::nextField()
	{
		fputc('\t', out);
	}

	void WritePqCopyStream::operator()(std::nullptr_t) const
	{
		fputs("\\N", out);
	}

	void WritePqCopyStream::operator()(timespec) const
	{
		throw std::logic_error("timespec not implemented");
	}

	void
	WritePqCopyStream::operator()(Date v) const
	{
		fprintf(out, "%d-%d-%d", v.year, v.month, v.day);
	}

	void
	WritePqCopyStream::operator()(Time v) const
	{
		fprintf(out, "%d:%d:%d", v.hour, v.minute, v.second);
	}

	void
	WritePqCopyStream::operator()(DateTime v) const
	{
		operator()(static_cast<Date &>(v));
		fputc('T', out);
		operator()(static_cast<Time &>(v));
	}

	void
	WritePqCopyStream::operator()(std::string_view v) const
	{
		auto pos {v.begin()};
		while (pos != v.end()) {
			auto esc = std::find_if(pos, v.end(), [](unsigned char c) {
				return std::iscntrl(c);
			});
			if (esc != pos) {
				fwrite(pos, boost::numeric_cast<size_t>(esc - pos), 1, out);
				pos = esc;
			}
			while (pos != v.end() && std::iscntrl(*pos)) {
				fprintf(out, "\\%03o", *pos);
				pos++;
			}
		}
	}

	void WritePqCopyStream::operator()(BitSet) const
	{
		throw std::logic_error("bitset not implemented");
	}

	void
	WritePqCopyStream::operator()(Blob v) const
	{
		static constexpr const auto hex {[] {
			std::array<std::array<char, 2>, 256> h {};
			std::array<char, 16> hc {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
			for (decltype(h)::size_type x {}; x < 256; x += 1) {
				h[x] = {hc[x >> 4], hc[x & 0xF]};
			}
			return h;
		}()};
		fputs("\\\\x", out);
		std::for_each(v.begin(), v.end(), [this](auto b) {
			fwrite(hex[(uint8_t)b].data(), 2, 1, out);
		});
	}
}
