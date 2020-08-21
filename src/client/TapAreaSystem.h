#pragma once

#include "server/ServerState.h"

#include "client/System.h"

namespace cl {

struct TapAreaSystem : EntitySystem
{
	static sf::Box<TapAreaSystem> create();

	virtual void addTapArea(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::TapAreaComponent &c, const Transform &transform) = 0;
};

}
