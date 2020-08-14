#pragma once

#include "server/ServerState.h"

#include "client/System.h"

namespace sf { struct FastRay; }

namespace cl {

struct CharacterModelSystem : EntitySystem
{
	struct AttachDesc
	{
		sf::Symbol boneName;
		sf::Mat34 childToBone;
		sf::Slice<const sf::Symbol> animationTags;
	};

	static sf::Box<CharacterModelSystem> create(const SystemsDesc &desc);

	virtual void addCharacterModel(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::CharacterModelComponent &c, const Transform &transform) = 0;

	virtual void addAttachedEntity(Systems &systems, uint32_t parentEntityId, uint32_t childEntityId, const AttachDesc &desc) = 0;

	virtual void updateAnimations(const VisibleAreas &visibleAreas, float dt) = 0;
	virtual void updateAttachedEntities(Systems &systems) = 0;
	virtual void updateLoadQueue(AreaSystem *areaSystem) = 0;

	virtual void renderMain(LightSystem *lightSystem, const VisibleAreas &visibleAreas, const RenderArgs &renderArgs) = 0;
};

}
