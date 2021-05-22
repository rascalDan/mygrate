#include <input/replStream.h>
#include <output/dumpToConsole.h>

int
main(int, char **)
{
	MyGrate::Input::ReplicationStream rs {"192.168.1.38", "repl", "r3pl", 3306};
	MyGrate::Output::DumpToConsole dtc;
	rs.readEvents(dtc);
}
