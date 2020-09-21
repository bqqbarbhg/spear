#pragma once

#include "server/ServerState.h"

#include "client/System.h"

namespace sf { struct FastRay; }

namespace cl {

struct HighlightDesc
{
	sf::Vec3 color;
};

struct CharacterModelSystem : EntitySystem
{
	struct AttachDesc
	{
		sf::Symbol boneName;
		sf::Mat34 childToBone;
		sf::Slice<const sf::Symbol> animationTags;
	};

	virtual sf::Box<void> preloadCharacterModel(const sv::CharacterModelComponent &c) = 0;

	static sf::Box<CharacterModelSystem> create(const SystemsDesc &desc);

	virtual void addCharacterModel(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::CharacterModelComponent &c, const Transform &transform) = 0;

	virtual void addAttachedEntity(Systems &systems, uint32_t parentEntityId, uint32_t childEntityId, const AttachDesc &desc) = 0;

	virtual uint32_t addBoneListener(Systems &systems, uint32_t parentEntityId, const sf::Symbol &boneName, BoneUpdates::Group group, uint32_t userId) = 0;
	virtual void freeBoneListener(uint32_t listenerId) = 0;

	virtual void updateAnimations(const VisibleAreas &activeAreas, float dt) = 0;
	virtual void updateBoneListeners(BoneUpdates &boneUpdates, const VisibleAreas &activeAreas) = 0;
	virtual void updateAttachedEntities(Systems &systems) = 0;
	virtual void updateLoadQueue(AreaSystem *areaSystem) = 0;

	virtual void addOneShotTag(const Entities &entities, uint32_t entityId, const sf::Symbol &tag) = 0;
	virtual void addTag(const Entities &entities, uint32_t entityId, const sf::Symbol &tag) = 0;
	virtual void removeTag(const Entities &entities, uint32_t entityId, const sf::Symbol &tag) = 0;
	virtual void queryFrameEvents(const Entities &entities, uint32_t entityId, sf::Array<sf::Symbol> &events) = 0;

	virtual uint32_t getEntityId(uint32_t modelId) const = 0;
	virtual void addFrameHighlightToModel(uint32_t characterModelId, const HighlightDesc &desc, const FrameArgs &frameArgs) = 0;
	virtual void addFrameHighlightToEntity(const Entities &entities, uint32_t entityId, const HighlightDesc &desc, const FrameArgs &frameArgs) = 0;

	virtual void renderShadow(const VisibleAreas &shadowAreas, const RenderArgs &renderArgs) = 0;
	virtual void renderEnvmapGBuffer(const VisibleAreas &envmapAreas, const RenderArgs &renderArgs) = 0;
	virtual void renderMain(const LightSystem *lightSystem, const EnvLightSystem *envLightSystem, const VisibleAreas &visibleAreas, const RenderArgs &renderArgs, const FrameArgs &frameArgs) = 0;
};

}
