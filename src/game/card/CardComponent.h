#pragma once

#include "game/Entity.h"

struct CardComponent
{
	Entity owner;

	virtual void render();
};
