#pragma once

#include "sf/String.h"

namespace sp {

struct ContentFile
{
	typedef void (*Callback)(void *user, const ContentFile &file);

	static void load(const sf::String &name, Callback callback, void *user);

	void *data;
	size_t size;
};

}
