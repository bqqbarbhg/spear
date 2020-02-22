#pragma once

#include "ext/mx/mx_sync.h"

namespace sf {

struct StaticSemaphore
{
	mx_semaphore sem;

	void wait() { mx_semaphore_wait(&sem); }
	void wait(uint32_t num) { mx_semaphore_wait_n(&sem, num); }
	void signal() { mx_semaphore_signal(&sem); }
	void signal(uint32_t num) { mx_semaphore_signal_n(&sem, num); }
	bool tryWait() { return mx_semaphore_try_wait(&sem); }
	bool tryWait(uint32_t num) { return mx_semaphore_try_wait_n(&sem, num); }
	int32_t getCount() const { return mx_semaphore_get_count(&sem); }
};

struct Semaphore : StaticSemaphore
{
	Semaphore(uint32_t count=0) {
		sem.count = count;
		sem.sema.state = 0;
	}
};

}
