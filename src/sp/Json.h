#pragma once

#include "sf/Base.h"

#include "ext/json_input.h"
#include "ext/json_output.h"

namespace sp {

void writeInstJson(jso_stream &dst, void *inst, sf::Type *type, sf::Type *parentType=NULL);
bool readInstJson(jsi_value *src, void *inst, sf::Type *type);

template <typename T> sf_inline void writeJson(jso_stream &dst, T &t) { writeInstJson(dst, (void*)&t, sf::typeOf<T>()); }
template <typename T> sf_inline bool readJson(jsi_value *src, T &t) { return readInstJson(src, (void*)&t, sf::typeOf<T>()); }

}
