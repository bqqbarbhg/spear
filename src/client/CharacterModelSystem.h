#pragma once

#include "server/ServerState.h"

#include "client/System.h"

namespace sf { struct FastRay; }

namespace cl {

struct CharacterModelSystem : EntitySystem
{
	static sf::Box<CharacterModelSystem> create();

	virtual void addCharacterModel(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::CharacterModelComponent &c, const Transform &transform) = 0;
	virtual void updateLoadQueue(AreaSystem *areaSystem) = 0;

	virtual void renderMain(const VisibleAreas &visibleAreas, const RenderArgs &renderArgs) = 0;
};

}
