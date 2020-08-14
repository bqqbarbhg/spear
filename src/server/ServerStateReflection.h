#pragma once

#include "sf/Base.h"
#include "sf/Symbol.h"

namespace sv {

struct ReflectionInfo
{
	const char *description = nullptr;
	bool prefab = false;
	bool asset = false;
	bool multiline = false;
	bool color = false;
	uint8_t fixedBits = 0;
};

ReflectionInfo getTypeReflectionInfo(sf::Type *type, const sf::Symbol &fieldName);
ReflectionInfo getTypeReflectionInfo(sf::Type *type, const sf::String &fieldName);

}
