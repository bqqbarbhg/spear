#pragma once

#include "sf/Base.h"
#include "sf/HashMap.h"
#include "sf/Array.h"
#include "sf/Box.h"

#include "ext/json_input.h"
#include "ext/json_output.h"

namespace sp {

struct WeakHandles
{
	sf::HashMap<uint64_t, sf::WeakBox<void>> handles;
	uint32_t gcLimit = 128;

	void collectGarbage(sf::Array<uint64_t> &deleted);
};

struct StrongHandles
{
	sf::HashMap<uint64_t, sf::Box<void>> handles;

	void collectGarbage(const sf::Slice<uint64_t> &deleted);
};

void writeInstJson(jso_stream &dst, void *inst, sf::Type *type, WeakHandles *handles, sf::Type *parentType=NULL);
bool readInstJson(jsi_value *src, void *inst, sf::Type *type, StrongHandles *handles);

template <typename T> sf_inline void writeJson(jso_stream &dst, T &t) { writeInstJson(dst, (void*)&t, sf::typeOf<T>()); }
template <typename T> sf_inline bool readJson(jsi_value *src, T &t) { return readInstJson(src, (void*)&t, sf::typeOf<T>()); }

}
