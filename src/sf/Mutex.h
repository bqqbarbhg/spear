#pragma once

#include "ext/mx/mx_sync.h"

namespace sf {

struct StaticMutex
{
	mx_mutex mutex;

	void lock() { mx_mutex_lock(&mutex); }
	bool tryLock() { return mx_mutex_try_lock(&mutex); }
	void unlock() { mx_mutex_unlock(&mutex); }
	bool isLocked() { return (bool)mx_mutex_is_locked(&mutex); }
};

struct Mutex : StaticMutex
{
	Mutex() { mutex.state = 0; }
};

struct MutexGuard
{
	StaticMutex &mutex;

	MutexGuard(StaticMutex &m)
		: mutex(m)
	{
		m.lock();
	}

	~MutexGuard()
	{
		mutex.unlock();
	}

	MutexGuard(const MutexGuard &) = delete;
	MutexGuard(MutexGuard &&) = delete;
};

}
