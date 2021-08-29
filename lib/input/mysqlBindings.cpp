#include "mysqlBindings.h"

namespace MyGrate::Input {
	MYSQL_TIME &
	operator<<(MYSQL_TIME & t, const Date & dt)
	{
		t.year = dt.year;
		t.month = dt.month;
		t.day = dt.day;
		return t;
	}

	MYSQL_TIME &
	operator<<(MYSQL_TIME & t, const Time & dt)
	{
		t.hour = dt.hour;
		t.minute = dt.minute;
		t.second = dt.second;
		return t;
	}

	MYSQL_TIME &
	operator<<(MYSQL_TIME & t, const DateTime & dt)
	{
		return t << static_cast<Date>(dt) << static_cast<Time>(dt);
	}
}
