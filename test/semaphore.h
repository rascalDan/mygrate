#ifndef MYGRATE_TEST_SEMAPHORE_H
#define MYGRATE_TEST_SEMAPHORE_H

#if __has_include(<semaphore>)
#	include <semaphore>
#endif

#ifdef __cpp_lib_semaphore
using semaphore = std::binary_semaphore;
#else
#	include <mutex>
class semaphore {
public:
	semaphore(unsigned int v_);

	void release(unsigned int n = 1);

	void acquire();

private:
	bool try_dec();
	unsigned int v;
	std::mutex m;
};
#endif

#endif
