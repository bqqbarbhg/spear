#pragma once

#include "Base.h"
#include "String.h"

namespace sf {

typedef void (*ThreadEntry)(void *user);

struct ThreadDesc
{
	ThreadEntry entry;
	void *user;
	sf::String name;
};

struct Thread
{
	static Thread *start(const ThreadDesc &desc);
	static void join(Thread *thread);

	static void sleepMs(uint32_t ms);
};

void setDebugThreadName(sf::String name);

}
