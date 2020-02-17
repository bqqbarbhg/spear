#pragma once

#include "sf/String.h"

namespace sp {

struct ContentFile
{
	typedef void (*Callback)(void *user, const ContentFile &file);
	struct LoadHandle {
		uint32_t id = ~0u;
		bool isValid() const { return id != ~0u; }
		bool operator==(LoadHandle rhs) const { return id == rhs.id; }
		bool operator!=(LoadHandle rhs) const { return id != rhs.id; }
	};

	// Lifecycle
	static void init();
	static void update();

	static LoadHandle load(const sf::CString &name, Callback callback, void *user);
	static LoadHandle load(const sf::String &name, Callback callback, void *user);
	static void cancel(LoadHandle handle);

	sf_forceinline bool isValid() const { return data != nullptr; }

	void *data = nullptr;
	size_t size = 0;
};

}
