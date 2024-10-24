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
		uint32_t endingIndex = ~0u;
		uint32_t parentId = ~0u;
		sf::Vec3 attachOffset;
		bool grounded = false;
	};

	sf::Array<Effect> effects;
	sf::Array<uint32_t> freeEffectIds;

	sf::Array<uint32_t> endingEffects;

	// API

	uint32_t spawnOneShotEffect(Systems &systems, const sf::Symbol &prefabName, const sf::Vec3 &position) override
	{
		if (!prefabName) return ~0u;

		Transform transform;
		transform.position = position;

		bool grounded = false;
		float lifeTime = 1.0f;
		if (sv::Prefab *prefab = systems.entities.findPrefab(prefabName)) {
			if (auto c = prefab->findComponent<sv::EffectComponent>()) {
				lifeTime = c->lifeTime;
				if (c->grounded) {
					grounded = true;
					transform.position.y = 0.0f;
				}
			}
		}

		uint32_t entityId = systems.entities.addEntity(systems, 0, transform, prefabName);
		if (entityId == ~0u) return ~0u;

		uint32_t effectId = effects.size;
		if (freeEffectIds.size > 0) {
			effectId = freeEffectIds.popValue();
		} else {
			effects.push();
		}

		Effect &effect = effects[effectId];
		effect.entityId = entityId;
		effect.grounded = grounded;
		effect.lifeTime = lifeTime;

		effect.endingIndex = endingEffects.size;
		endingEffects.push(effectId);

		systems.entities.addComponent(entityId, this, effectId, 0, 0, 0);

		return entityId;
	}

	uint32_t spawnAttachedEffect(Systems &systems, const sf::Symbol &prefabName, uint32_t parentId, const sf::Vec3 &offset) override
	{
		if (!prefabName) return ~0u;
		Entity &parent = systems.entities.entities[parentId];

		Transform transform;
		transform.position = parent.transform.transformPoint(offset);

		bool grounded = false;
		float lifeTime = 1.0f;
		if (sv::Prefab *prefab = systems.entities.findPrefab(prefabName)) {
			if (auto c = prefab->findComponent<sv::EffectComponent>()) {
				lifeTime = c->lifeTime;
				if (c->grounded) {
					grounded = true;
					transform.position.y = 0.0f;
				}
			}
		}

		uint32_t entityId = systems.entities.addEntity(systems, 0, transform, prefabName);
		if (entityId == ~0u) return ~0u;

		uint32_t effectId = effects.size;
		if (freeEffectIds.size > 0) {
			effectId = freeEffectIds.popValue();
		} else {
			effects.push();
		}

		Effect &effect = effects[effectId];
		effect.entityId = entityId;
		effect.attachOffset = offset;
		effect.parentId = parentId;
		effect.lifeTime = lifeTime;
		effect.grounded = grounded;

		systems.entities.addComponent(entityId, this, effectId, 0, 0, 0);
		systems.entities.addComponent(parentId, this, effectId, 1, 0, Entity::UpdateTransform);

		return effectId;
	}

	void removeAttachedEffect(uint32_t effectId) override
	{
		if (effectId == ~0u) return;

		Effect &effect = effects[effectId];
		if (effect.endingIndex == ~0u) {
			effect.endingIndex = endingEffects.size;
			endingEffects.push(effectId);
		}
	}

	void updateTransform(Systems &systems, uint32_t entityId, const EntityComponent &ec, const TransformUpdate &update) override
	{
		if (ec.subsystemIndex == 1) {
			uint32_t effectId = ec.userId;
			Effect &effect = effects[effectId];

			Transform transform;
			transform.position = update.transform.transformPoint(effect.attachOffset);

			if (effect.grounded) {
				transform.position.y = 0.0f;
			}

			systems.entities.updateTransform(systems, effect.entityId, transform);

		} else {
			sf_fail();
		}
	}

	void remove(Systems &systems, uint32_t entityId, const EntityComponent &ec) override
	{
		if (ec.subsystemIndex == 0) {
			uint32_t effectId = ec.userId;
			Effect &effect = effects[effectId];

			if (effect.parentId != ~0u) {
				systems.entities.removeComponent(effect.parentId, this, effectId, 1);
			}

			if (effect.endingIndex != ~0u) {
				effects[endingEffects.back()].endingIndex = effect.endingIndex;
				endingEffects.removeSwap(effect.endingIndex);
			}

			freeEffectIds.push(effectId);
			sf::reset(effect);
		} else if (ec.subsystemIndex == 1) {
			uint32_t effectId = ec.userId;
			Effect &effect = effects[effectId];

			effect.parentId = ~0u;

			if (effect.endingIndex == ~0u) {
				effect.endingIndex = endingEffects.size;
				endingEffects.push(effectId);
			}
		}
	}

	void update(Entities &entities, const FrameArgs &frameArgs) override
	{
		float dt = frameArgs.dt;

		for (uint32_t i = 0; i < endingEffects.size; i++) {
			uint32_t effectId = endingEffects[i];
			Effect &effect = effects[effectId];

			effect.lifeTime -= dt;
			if (effect.lifeTime <= 0.0f) {
				entities.removeEntityQueued(effect.entityId);
				effects[endingEffects.back()].endingIndex = i;
				endingEffects.removeSwap(i--);
				effect.endingIndex = ~0u;
			}
		}
	}
};

sf::Box<EffectSystem> EffectSystem::create() { return sf::box<EffectSystemImp>(); }

}
