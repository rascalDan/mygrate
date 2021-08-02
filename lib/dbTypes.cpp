#include "dbTypes.h"
#include <iostream> // Required else this write::operator() is always recursive

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

static_assert(std::is_copy_assignable_v<MyGrate::Date>);
static_assert(std::is_copy_assignable_v<MyGrate::Time>);
static_assert(std::is_copy_assignable_v<MyGrate::DateTime>);
static_assert(std::is_copy_assignable_v<MyGrate::Blob>);
static_assert(std::is_copy_assignable_v<timespec>);
static_assert(std::is_copy_assignable_v<MyGrate::BitSet>);
static_assert(std::is_copy_assignable_v<MyGrate::DbValueV>);
static_assert(std::is_copy_assignable_v<MyGrate::DbValue>);

static_assert(std::is_move_assignable_v<MyGrate::Date>);
static_assert(std::is_move_assignable_v<MyGrate::Time>);
static_assert(std::is_move_assignable_v<MyGrate::DateTime>);
static_assert(std::is_move_assignable_v<MyGrate::Blob>);
static_assert(std::is_move_assignable_v<timespec>);
static_assert(std::is_move_assignable_v<MyGrate::BitSet>);
static_assert(std::is_move_assignable_v<MyGrate::DbValueV>);
static_assert(std::is_move_assignable_v<MyGrate::DbValue>);
