#ifndef MYGRATE_OUTPUT_PQ_WRITEPQSTRM_H
#define MYGRATE_OUTPUT_PQ_WRITEPQSTRM_H

#include <concepts>
#include <cstdio>
#include <dbTypes.h>

namespace MyGrate::Output::Pq {
	struct WritePqCopyStream {
		explicit WritePqCopyStream(FILE * o);

		void nextRecord();
		void nextField();

		void operator()(std::nullptr_t) const;
		void
		operator()(std::integral auto v) const
		{
			fprintf(out, printer<decltype(v)>::fmt, v);
		}
		void
		operator()(std::floating_point auto v) const
		{
			fprintf(out, printer<decltype(v)>::fmt, v);
		}
		void operator()(timespec) const;
		void operator()(Date v) const;
		void operator()(Time v) const;
		void operator()(DateTime v) const;
		void operator()(std::string_view v) const;
		void operator()(BitSet) const;
		void operator()(Blob v) const;

		FILE * out;
	};
}

#endif
