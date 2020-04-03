#pragma once

#include <stdint.h>
#include "sf/Base.h"

struct Entity
{
	uint32_t handle;

	sf_forceinline uint32_t getIndex() const { return handle & 0xffffff; }
	sf_forceinline uint32_t getGeneration() const { return handle >> 24; }
	sf_forceinline bool getIsLocal() const { return (bool)((handle >> 23) & 1); }
};

struct EntityInfo
{
	const char *name;
	uint32_t systemMask[1];
};
