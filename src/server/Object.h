#pragma once

#include "sf/Base.h"

namespace sv2 {

struct Object
{
	enum Type {
		Error,
		CardType,
		Player,

		Type_Count,
		Type_Force32 = 0x7fffffff,
	};

	Type objectType = Error;
	uint64_t objectId = 0;
	uint64_t objectRevision = 0;

	Object() { }
	Object(Type type) : objectType(type) { }
};

}
