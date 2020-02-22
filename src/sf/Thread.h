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
};

void setDebugThreadName(sf::String name);

}
