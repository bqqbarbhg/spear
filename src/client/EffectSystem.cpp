#include "EffectSystem.h"

#include "client/AreaSystem.h"

#include "game/DebugDraw.h"

#include "sf/Geometry.h"

namespace cl {

struct EffectSystemImp final : EffectSystem
{
	struct Effect
	{
		float lifeTime = 1.0f;
		uint32_t entityId;
		uint32_t activeIndex = ~0u;
	};

	sf::Array<Effect> effects;
	sf::Array<uint32_t> freeEffectIds;

	sf::Array<uint32_t> activeEffects;

	// API

	void spawnOneShotEffect(Systems &systems, const sf::Symbol &prefabName, const sf::Vec3 &position) override
	{
		uint32_t effectId = effects.size;
		if (freeEffectIds.size > 0) {
			effectId = freeEffectIds.popValue();
		} else {
			effects.push();
		}

		Transform transform;
		transform.position = position;

		Effect &effect = effects[effectId];
		effect.entityId = systems.entities.addEntity(systems, 0, transform, prefabName);

		effect.activeIndex = activeEffects.size;
		activeEffects.push(effectId);

		systems.entities.addComponent(effect.entityId, this, effectId, 0, 0, 0);
	}

	void updateTransform(Systems &systems, uint32_t entityId, const EntityComponent &ec, const TransformUpdate &update) override
	{
	}

	void remove(Systems &systems, uint32_t entityId, const EntityComponent &ec) override
	{
		uint32_t effectId = ec.userId;
		Effect &effect = effects[effectId];

		if (effect.activeIndex != ~0u) {
			effects[activeEffects.back()].activeIndex = effect.activeIndex;
			activeEffects.removeSwap(effect.activeIndex);
		}

		freeEffectIds.push(effectId);
		sf::reset(effect);
	}

	void update(Entities &entities, const FrameArgs &frameArgs) override
	{
		float dt = frameArgs.dt;

		for (uint32_t i = 0; i < activeEffects.size; i++) {
			uint32_t effectId = activeEffects[i];
			Effect &effect = effects[effectId];

			effect.lifeTime -= dt;
			if (effect.lifeTime <= 0.0f) {
				entities.removeEntityQueued(effect.entityId);
				effects[activeEffects.back()].activeIndex = i;
				activeEffects.removeSwap(i--);
				effect.activeIndex = ~0u;
			}
		}
	}
};

sf::Box<EffectSystem> EffectSystem::create() { return sf::box<EffectSystemImp>(); }

}
