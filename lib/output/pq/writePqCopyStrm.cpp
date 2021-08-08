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
				fwrite(pos, esc - pos, 1, out);
				pos = esc;
			}
			while (pos != v.end()) {
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
		fputs("\\\\x", out);
		std::for_each(v.begin(), v.end(), [this](auto b) {
			fprintf(out, "%02hhx", (uint8_t)b);
		});
	}
}
