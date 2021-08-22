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

EventCounterTarget &
EventCounterTarget::add(mariadb_rpl_event e, unsigned long n)
{
	counters[e] += n;
	return *this;
}
