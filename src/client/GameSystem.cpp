#include "GameSystem.h"

#include "server/Pathfinding.h"

#include "client/CharacterModelSystem.h"

#include "game/DebugDraw.h"

#include "sf/UintMap.h"

namespace cl {

static Transform getCharacterTransform(const sv::Character &chr)
{
	Transform ret;
	ret.position = sf::Vec3((float)chr.tile.x, 0.0f, (float)chr.tile.y);
	return ret;
}

struct GameSystemImp final : GameSystem
{
	uint32_t selectedCharacterId = 102;

	struct Character
	{
		sf::Box<sv::Prefab> svPrefab;
		uint32_t svId;
		uint32_t entityId;
	};

	struct Card
	{
		sf::Box<sv::Prefab> svPrefab;
		uint32_t prefabId;
		uint32_t svId;

		sf::Array<uint32_t> entityIds;
	};

	sf::Array<Character> characters;
	sf::Array<uint32_t> freeCharacterIds;

	sf::Array<Card> cards;
	sf::Array<uint32_t> freeCardIds;

	sf::UintMap svToCharacter;
	sf::UintMap entityToCharacter;

	sf::UintMap svToCard;


	void equipCardImp(Systems &systems, Character &character, Card &card)
	{
		for (const sv::Component *comp : card.svPrefab->components) {
			if (const auto *c = comp->as<sv::CardAttachComponent>()) {
				cl::Transform transform;
				transform.scale = 0.0f;
				uint32_t entityId = systems.entities.addEntity(systems, card.svId, transform, c->prefabName);
				if (entityId == ~0u) continue;

				card.entityIds.push(entityId);

				CharacterModelSystem::AttachDesc desc;
				desc.boneName = c->boneName;
				desc.childToBone = sf::mat::translate(c->offset) * sf::mat::scale(c->scale);
				desc.animationTags = c->animationTags.slice();
				systems.characterModel->addAttachedEntity(systems, character.entityId, entityId, desc);
			}
		}
	}

	void unequipCardImp(Systems &systems, Character &character, Card &card)
	{
		for (uint32_t entityId : card.entityIds) {
			systems.entities.removeEntityQueued(entityId);
		}
	}


	// -- API

	void update(const sv::ServerState &svState, const FrameArgs &frameArgs) override
	{
		{
			const sv::Character *chr = svState.characters.find(selectedCharacterId);

#if 0
			if (chr) {
				sv::PathfindOpts opts;
				opts.isBlockedFn = &sv::isBlockedByPropOrCharacter;
				opts.maxDistance = 10;
				sv::ReachableSet reachable = sv::findReachableSet(svState, opts, chr->tile);

				for (const auto &pair : reachable.distanceToTile) {
					sf::Bounds3 bounds;
					bounds.origin = sf::Vec3((float)pair.key.x, 0.0f, (float)pair.key.y);
					bounds.extent = sf::Vec3(0.95f, 0.2f, 0.95f) / 2.0f;
					debugDrawBox(bounds, sf::Vec3(1.0f, 1.0f, 0.0f));
				}
			}
#endif

		}
	}

	void updateTransform(Systems &systems, uint32_t entityId, const EntityComponent &ec, const TransformUpdate &update) override
	{
	}

	void remove(Systems &systems, uint32_t entityId, const EntityComponent &ec) override
	{
	}

	void applyEvent(Systems &systems, const sv::Event &event) override
	{
		if (const auto *e = event.as<sv::AddCharacterEvent>()) {

			Transform transform = getCharacterTransform(e->character);
			uint32_t entityId = systems.entities.addEntity(systems, e->character.id, transform, e->character.prefabName);
			uint32_t svId = e->character.id;

			uint32_t characterId = characters.size;
			if (freeCharacterIds.size > 0) {
				characterId = freeCharacterIds.popValue();
			} else {
				characters.push();
			}

			Character &character = characters[characterId];
			character.svPrefab = systems.entities.prefabs[systems.entities.entities[entityId].prefabId].svPrefab;
			character.entityId = entityId;
			character.svId = svId;

			entityToCharacter.insertDuplicate(entityId, characterId);
			svToCharacter.insertDuplicate(svId, characterId);

		} else if (const auto *e = event.as<sv::AddCardEvent>()) {

			uint32_t svId = e->card.id;

			uint32_t cardId = cards.size;
			if (freeCardIds.size > 0) {
				cardId = freeCardIds.popValue();
			} else {
				cards.push();
			}

			Card &card = cards[cardId];
			card.svId = svId;
			card.svPrefab = systems.entities.findPrefab(e->card.prefabName);

			svToCard.insertDuplicate(svId, cardId);

		} else if (const auto *e = event.as<sv::SelectCardEvent>()) {
			uint32_t characterId = svToCharacter.findOne(e->ownerId, ~0u);
			uint32_t cardId = svToCard.findOne(e->cardId, ~0u);

			if (characterId != ~0u && cardId != ~0u) {
				Character &chr = characters[characterId];
				Card &card = cards[cardId];
				equipCardImp(systems, chr, card);
			}

		}
	}
};

sf::Box<GameSystem> GameSystem::create() { return sf::box<GameSystemImp>(); }

}
