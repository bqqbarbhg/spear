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

struct StaticRecursiveMutex
{
	mx_recursive_mutex mutex;

	void lock() { mx_recursive_mutex_lock(&mutex); }
	bool tryLock() { return mx_recursive_mutex_try_lock(&mutex); }
	void unlock() { mx_recursive_mutex_unlock(&mutex); }
	bool isLocked() { return mx_recursive_mutex_get_depth(&mutex) > 0; }
};

struct Mutex : StaticMutex
{
	Mutex() { mutex.state = 0; }
};

struct RecursiveMutex : StaticRecursiveMutex
{
	RecursiveMutex() { mutex.mutex.state = 0; mutex.thread_id = 0; mutex.recursion_depth = 0; }
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

struct RecursiveMutexGuard
{
	StaticRecursiveMutex &mutex;

	RecursiveMutexGuard(StaticRecursiveMutex &m)
		: mutex(m)
	{
		m.lock();
	}

	~RecursiveMutexGuard()
	{
		mutex.unlock();
	}

	RecursiveMutexGuard(const MutexGuard &) = delete;
	RecursiveMutexGuard(MutexGuard &&) = delete;
};


}
