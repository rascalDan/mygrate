#include "helpers.h"

MemStream::MemStream() : out {nullptr}, len {}, s {open_memstream(&out, &len)} { }

MemStream::~MemStream()
{
	fclose(s);
	free(out);
}

void
MemStream::flush()
{
	fflush(s);
}
