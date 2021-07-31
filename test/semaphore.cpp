#include "semaphore.h"

#ifndef __cpp_lib_semaphore
#	include <thread>

semaphore::semaphore(unsigned int v_) : v {v_} { }

void
semaphore::release(unsigned int n)
{
	std::lock_guard lk {m};
	v += n;
}

void
semaphore::acquire()
{
	while (!try_dec()) { }
}

bool
semaphore::try_dec()
{
	std::lock_guard lk {m};
	if (v) {
		v--;
		m.unlock();
		return true;
	}
	else {
		m.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		return false;
	}
}

#endif
