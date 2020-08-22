#pragma once

#include "server/ServerState.h"

#include "client/System.h"

namespace sf { struct FastRay; }

namespace cl {

struct TapAreaSystem : EntitySystem
{
	static sf::Box<TapAreaSystem> create();

	virtual void addTapArea(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::TapAreaComponent &c, const Transform &transform) = 0;

	virtual uint32_t getClosestTapAreaEntity(const AreaSystem *areaSystem, const sf::Ray &ray) const = 0;

	virtual void editorPick(sf::Array<EntityHit> &hits, const sf::FastRay &ray, uint32_t userId) const = 0;
};

}
