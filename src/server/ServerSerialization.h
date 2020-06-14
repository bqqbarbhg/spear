#pragma once

#include "sf/Reflection.h"
#include "sf/HashMap.h"
#include "sf/Box.h"
#include "server/Object.h"

namespace sv2 {

using SerializationState = sf::HashMap<uint64_t, uint64_t>;
using DeserializationState = sf::HashMap<uint64_t, sf::Box<sv2::Object>>;

void serializeInstBinary(sf::Array<char> &dst, SerializationState &state, void *inst, sf::Type *type);
bool deserializeInstBinary(sf::Slice<char> &src, DeserializationState &state, void *inst, sf::Type *type);

template <typename T>
sf_inline void serializeBinary(sf::Array<char> &dst, SerializationState &state, T &t) {
	return serializeInstBinary(dst, state, (void*)&t, sf::typeOf<T>());
}

template <typename T>
sf_inline bool deserializeBinary(sf::Slice<char> &src, DeserializationState &state, T &t) {
	return deserializeInstBinary(src, state, (void*)&t, sf::typeOf<T>());
}

}
