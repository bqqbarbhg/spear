#include "GameSystem.h"

#include "server/Pathfinding.h"

#include "client/CharacterModelSystem.h"

#include "game/DebugDraw.h"

#include "sf/UintMap.h"

#include "client/GuiCard.h"

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

	struct SelectedCard
	{
		uint32_t currentSvId = 0;
		uint32_t prevSvId = 0;
	};

	struct Character
	{
		sf::Box<sv::Prefab> svPrefab;
		uint32_t svId;
		uint32_t entityId;

		SelectedCard selectedCards[sv::NumSelectedCards];
	};

	struct Card
	{
		sf::Box<sv::Prefab> svPrefab;
		uint32_t prefabId;
		uint32_t svId;

		GuiCard gui;

		sf::Array<uint32_t> entityIds;
	};

	sf::Array<Character> characters;
	sf::Array<uint32_t> freeCharacterIds;

	sf::Array<Card> cards;
	sf::Array<uint32_t> freeCardIds;

	sf::UintMap svToCharacter;
	sf::UintMap entityToCharacter;

	sf::UintMap svToCard;

	void equipCardImp(Systems &systems, uint32_t characterId, uint32_t cardId, uint32_t slot)
	{
		Character &chr = characters[characterId];
		Card &card = cards[cardId];

		SelectedCard &selected = chr.selectedCards[slot];
		selected.prevSvId = selected.currentSvId;
		selected.currentSvId = card.svId;

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
				systems.characterModel->addAttachedEntity(systems, chr.entityId, entityId, desc);
			}
		}
	}

	void unequipCardImp(Systems &systems, Character &character, Card &card)
	{
		for (uint32_t entityId : card.entityIds) {
			systems.entities.removeEntityQueued(entityId);
		}
	}

	Character *findCharacter(uint32_t svId)
	{
		if (!svId) return nullptr;
		uint32_t id = svToCharacter.findOne(svId, ~0u);
		if (id == ~0u) return nullptr;
		return &characters[id];
	}

	Card *findCard(uint32_t svId)
	{
		if (!svId) return nullptr;
		uint32_t id = svToCard.findOne(svId, ~0u);
		if (id == ~0u) return nullptr;
		return &cards[id];
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
			card.gui.init(*card.svPrefab);

			svToCard.insertDuplicate(svId, cardId);

		} else if (const auto *e = event.as<sv::SelectCardEvent>()) {
			uint32_t characterId = svToCharacter.findOne(e->ownerId, ~0u);
			uint32_t cardId = svToCard.findOne(e->cardId, ~0u);

			if (characterId != ~0u && cardId != ~0u) {
				equipCardImp(systems, characterId, cardId, e->slot);
			}

		}
	}

	void handleGui(Systems &systems, const GuiArgs &guiArgs) override
	{
		sp::Canvas &canvas = *guiArgs.canvas;

		if (Character *chr = findCharacter(selectedCharacterId)) {
			float cardHeight = 140.0f;
			float cardWidth = cardHeight * GuiCard::canvasXByY;
			float cardPad = 10.0f;

			float x = 20.0f;
			float y = guiArgs.resolution.y - 20.0f - cardHeight;

			for (SelectedCard &selected : chr->selectedCards) {
				if (Card *card = findCard(selected.currentSvId)) {
					canvas.pushTransform(sf::mat2D::translate(x, y) * sf::mat2D::scale(cardHeight / GuiCard::canvasHeight));
					cl::renderCard(canvas, card->gui);
					canvas.popTransform();
				}

				x += cardWidth + cardPad;
			}
		}
	}

};

sf::Box<GameSystem> GameSystem::create() { return sf::box<GameSystemImp>(); }

}
