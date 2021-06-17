#include "dbTypes.h"

namespace std {
	struct write {
		template<typename T>
		std::ostream &
		operator()(const T & v) const
		{
			return strm << v;
		}
		std::ostream & strm;
	};

	std::ostream &
	operator<<(std::ostream & strm, const MyGrate::DbValueV & v)
	{
		return std::visit(write {strm}, v);
	}
}
