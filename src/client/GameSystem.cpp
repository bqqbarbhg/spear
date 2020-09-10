#include "GameSystem.h"

#include "server/Pathfinding.h"

#include "client/CharacterModelSystem.h"
#include "client/BillboardSystem.h"
#include "client/AreaSystem.h"
#include "client/TapAreaSystem.h"
#include "client/EffectSystem.h"

#include "game/DebugDraw.h"

#include "sf/UintMap.h"
#include "sf/Reflection.h"

#include "client/GuiCard.h"

#include "sp/Renderer.h"
#include "sp/Srgb.h"

#include "ext/sokol/sokol_app.h"
#include "ext/imgui/imgui.h"

#include "client/InputState.h"

#include "client/gui/Gui.h"
#include "client/gui/GlueWidgets.h"
#include "client/gui/WidgetLinearLayout.h"
#include "client/gui/WidgetGridLayout.h"
#include "client/gui/WidgetScroll.h"
#include "client/gui/WidgetCardSlot.h"
#include "client/gui/WidgetCard.h"
#include "client/gui/GuiBuilder.h"
#include "client/gui/GuiResources.h"

namespace cl {

static const sf::Symbol guiCardSym { "GuiCard" };
static const sf::Symbol symEffect { "Effect" };
static const sf::Symbol symMelee { "Melee" };
static const sf::Symbol symStagger { "Stagger" };
static const sf::Symbol symCast { "Cast" };
static const sf::Symbol symHit { "Hit" };
static const sf::Symbol symRun { "Run" };

static const constexpr float TapCancelDistance = 0.03f;
static const constexpr float TapCancelDistanceSq = TapCancelDistance * TapCancelDistance;

struct WidgetTest : gui::WidgetBase<'t','e','s','t'>
{
	sp::SpriteRef sprite;
	sp::FontRef font;
	bool tapped = false;
	bool pressed = false;

	uint32_t index;

	virtual void layout(gui::GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max) override
	{
		if (sprite->isLoaded()) {
			float aspect = sprite->aspect;
			sf::Vec2 size = sf::clamp(boxExtent, min, max);
			float scale = sf::min(size.x / aspect, size.y);
			layoutSize = sf::Vec2(scale * aspect, scale);
		} else {
			layoutSize = sf::Vec2();
		}
	}

	virtual void paint(gui::GuiPaint &paint) override
	{
		paint.canvas->draw(sprite, layoutOffset, layoutSize, pressed ? sf::Vec4(1.0f, 0.0f, 0.0f, 1.0f) : sf::Vec4(1.0f));

		sf::SmallStringBuf<16> number;
		number.format("%u", index);
		sp::TextDraw td;
		td.string = number;
		td.font = font;
		td.depth = 1.0f;
		td.height = 30.0f;
		td.color = tapped ? sf::Vec4(0.0f, 0.0f, 0.0f, 1.0f) : sf::Vec4(1.0f);
		sf::Vec2 measure = font->measureText(number, td.height);
		td.transform = sf::mat2D::translate(layoutOffset + layoutSize * 0.5f - sf::Vec2(measure.x, -measure.y*0.5f) * 0.5f);
		paint.canvas->drawText(td);
	}

	virtual bool onPointer(gui::GuiPointer &pointer) override
	{
		if (pointer.button == gui::GuiPointer::MouseHover && pointer.action == gui::GuiPointer::Down) {
			return true;
		}

		if (pointer.button == gui::GuiPointer::Touch && pointer.action == gui::GuiPointer::Tap) {
			tapped = !tapped;
			return true;
		}

		if (pointer.button == gui::GuiPointer::Touch && pointer.action == gui::GuiPointer::LongPress) {
			if (!pointer.trackWidget) {
				pressed = !pressed;
			}
			pointer.trackWidget = sf::boxFromPointer(this);
			return true;
		}

		return false;
	}
};

struct Camera
{
	struct State
	{
		sf::Vec3 origin;
        float zoom = 0.0f;

		void asMatrices(sf::Vec3 &eye, sf::Mat34 &worldToView, sf::Mat44 &viewToClip, float aspect)
		{
            eye = origin + sf::Vec3(0.0f, 5.0f, 1.0f) * exp2f(zoom);
            worldToView = sf::mat::look(eye, sf::Vec3(0.0f, -1.0f, -0.6f + 0.2f * zoom));
			viewToClip = sf::mat::perspectiveD3D(1.0f, aspect, 1.0f, 100.0f);
		}
	};

	State previous;
	State current;

	float touchMove = 0.0f;
	sf::Vec3 targetDelta;
    float zoomDelta = 0.0f;
	sf::Vec3 velocity;

	float timeDelta = 0.0f;

	static State lerp(const State &a, const State &b, float t)
	{
		State s;
		s.origin = sf::lerp(a.origin, b.origin, t);
        s.zoom = sf::lerp(a.zoom, b.zoom, t);
		return s;
	}
};

static void faceTowardsPosition(Transform &transform, const sf::Vec3 &position, float dt, float speed)
{
	sf::Vec3 z = position - transform.position;
	float zLen = sf::length(z);
	if (zLen < 0.01f) return;
	z /= zLen;
	sf::Vec3 y = sf::Vec3(0.0f, 1.0f, 0.0f);
	sf::Vec3 x = sf::cross(y, z);
	sf::Quat target = sf::axesToQuat(x, y, z);
	if (sf::dot(target, transform.rotation) < 0.0f) {
		target = -target;
	}

	float alpha = exp2f(dt*-speed);
	transform.rotation = sf::normalize(sf::lerp(target, transform.rotation, alpha));
}

struct GameSystemImp final : GameSystem
{
	uint32_t selectedCharacterId = 0;
	float selectedCharacterTime = 0.0f;
	float moveSelectTime = 0.0f;

	struct SelectedCard
	{
		uint32_t currentSvId = 0;
		uint32_t prevSvId = 0;
	};

	struct Character
	{
		sf::Box<sv::Prefab> svPrefab;
		sf::Vec2i tile;
		uint32_t svId;
		uint32_t entityId;

		sf::Vec3 centerOffset;

		sf::Array<uint32_t> cardIds;
		SelectedCard selectedCards[sv::NumSelectedCards];
	};

	struct Card
	{
		sf::Box<sv::Prefab> svPrefab;
		uint32_t prefabId;
		uint32_t svId;

		sf::Box<GuiCard> gui;

		sf::Array<uint32_t> entityIds;
	};

	struct Status
	{
		sf::Box<sv::Prefab> svPrefab;
		uint32_t svId;

		sf::Symbol tickEffect;
		sf::Symbol endEffect;

		uint32_t characterSvId = 0;
		uint32_t effectId = ~0u;
	};

	struct PointerState
	{
		uint64_t id;
		bool active = true;
		bool hitGui = false;
		bool hitGuiThisFrame = false;
		bool hitBackground = false;
		uint32_t startSvId = 0;
		uint32_t currentSvId = 0;
		sf::Box<gui::Widget> trackWidget;
	};

	static sf::Vec3 intersectHorizontalPlane(float height, const sf::Ray &ray)
	{
		float t = (ray.origin.y - height) / - ray.direction.y;
		sf::Vec3 v = ray.origin + ray.direction * t;
		v.y = height;
		return v;
	}

	struct DragPointer
	{
		gui::GuiPointer guiPointer;
		sf::Vec2 smoothPosition;
		sf::Vec2 smoothVelocity;
		float time = 0.0f;
		bool active = true;
		bool hitGuiThisFrame = false;
	};

	struct CardTrade
	{
		uint32_t srcSvId;
		uint32_t dstSvId;
		uint32_t cardSvId;
		float time = 0.0f;
	};

	struct EventContext
	{
		float timer = 0.0f;
		bool begin = true;
		bool immediate = false;
	};

	struct Projectile
	{
		uint32_t entityId;
		sf::Symbol hitEffect;

		uint32_t targetSvId;
		float flightSpeed = 1.0f;
	};

	struct DamageNumber
	{
		static const constexpr float BaseHeight = 20.0f;

		sf::SmallStringBuf<16> text;
		sf::Vec3 position;
		sf::Vec2 origin;
		float time = 0.0f;
	};

	sf::Array<Character> characters;
	sf::Array<uint32_t> freeCharacterIds;

	sf::Array<Card> cards;
	sf::Array<uint32_t> freeCardIds;

	sf::Array<Status> statuses;
	sf::Array<uint32_t> freeStatusIds;

	sf::UintMap svToCharacter;
	sf::UintMap entityToCharacter;

	sf::UintMap svToCard;

	sf::UintMap svToStatus;

	InputState input;

	sf::Array<sf::Box<sv::Action>> requestedActions;

	Camera camera;

	sv::ReachableSet moveSet;

	gui::GuiResources guiResources;

	sf::ImplicitHashMap<PointerState, sv::KeyId> pointerStates;

	sf::Array<gui::Widget*> guiWorkArray;
	gui::GuiBuilder guiBuilder;
	sf::Box<gui::Widget> guiRoot;

	sf::Array<DragPointer> dragPointers;

	sf::Array<CardTrade> cardTrades;

	bool didHoverTile = false;
	sf::Vec2i hoveredTile;
	sf::HashMap<sf::Vec2i, float> tileHoverAmount;
	sf::HashMap<sf::Vec2i, float> tileTrailAmount;

	EventContext queuedEventContext;
	sf::Array<sf::Box<sv::Event>> queuedEvents;

	sf::Array<Projectile> projectiles;

	sv::TurnInfo turnInfo;

	uint32_t selectedCardSlot = ~0u;
	float selectedCardTime = 0.0f;

	bool showDebugMenu = false;
	bool showDebugPointers = false;
	bool simulateTouch = false;

	bool castAnimDone = false;
	bool castDone = false;

	uint32_t moveWaypointIndex = ~0u;
	sf::Vec3 moveVelocity;
	float moveEndTime = 0.0f;

	sf::Array<DamageNumber> damageNumbers;

	void equipCardImp(Systems &systems, uint32_t characterId, uint32_t cardId, uint32_t slot)
	{
		Character &chr = characters[characterId];
		Card &card = cards[cardId];

		if (Card *prev = findCard(chr.selectedCards[slot].currentSvId)) {
			unequipCardImp(systems, chr, *prev);
		}

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

	Status *findStatus(uint32_t svId)
	{
		if (!svId) return nullptr;
		uint32_t id = svToStatus.findOne(svId, ~0u);
		if (id == ~0u) return nullptr;
		return &statuses[id];
	}

	bool applyEventImp(Systems &systems, const sv::Event &event, EventContext &ctx)
	{
		float dt = systems.frameArgs.dt;
		if (const auto *e = event.as<sv::AddCharacterEvent>()) {
			Transform transform;
			transform.position = sf::Vec3((float)e->character.tile.x, 0.0f, (float)e->character.tile.y);
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
			character.tile = e->character.tile;

			if (character.svPrefab) {
				if (auto *c = character.svPrefab->findComponent<sv::CharacterComponent>()) {
					character.centerOffset = c->centerOffset;
				}
			}

			entityToCharacter.insertDuplicate(entityId, characterId);
			svToCharacter.insertDuplicate(svId, characterId);

		} else if (const auto *e = event.as<sv::RemoveCharacterEvent>()) {

			if (Character *chr = findCharacter(e->characterId)) {
				Entity &entity = systems.entities.entities[chr->entityId];
				Prefab &prefab = systems.entities.prefabs[entity.prefabId];

				if (!ctx.immediate && ctx.begin) {
					if (sv::CharacterComponent *c = prefab.svPrefab->findComponent<sv::CharacterComponent>()) {
						sf::Vec3 pos = entity.transform.transformPoint(chr->centerOffset);
						systems.effect->spawnOneShotEffect(systems, c->defeatEffect, pos);
					}
				}

				systems.entities.removeEntityQueued(chr->entityId);

				if (selectedCharacterId == e->characterId) {
					selectedCharacterId = 0;
				}
				
				sf::reset(*chr);
				freeCharacterIds.push((uint32_t)(chr - characters.data));
			}

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
			card.gui = sf::box<GuiCard>();
			card.gui->init(*card.svPrefab, card.svId);

			svToCard.insertDuplicate(svId, cardId);

		} else if (const auto *e = event.as<sv::RemoveCardEvent>()) {

			if (Character *chr = findCharacter(e->prevOwnerId)) {
				sf::findRemoveSwap(chr->cardIds, e->cardId);
			}

			if (Card *card = findCard(e->cardId)) {
				sf::reset(*card);
				freeCardIds.push((uint32_t)(card - cards.data));
			}

		} else if (const auto *e = event.as<sv::StatusAddEvent>()) {

			uint32_t svId = e->status.id;

			uint32_t statusId = statuses.size;
			if (freeStatusIds.size > 0) {
				statusId = freeStatusIds.popValue();
			} else {
				statuses.push();
			}

			Status &status = statuses[statusId];
			status.svId = svId;
			status.svPrefab = systems.entities.findPrefab(e->status.prefabName);
			status.characterSvId = e->status.characterId;

			svToStatus.insertDuplicate(svId, statusId);

			if (status.svPrefab) {
				if (Character *chr = findCharacter(e->status.characterId)) {
					Entity &entity = systems.entities.entities[chr->entityId];
					if (auto c = status.svPrefab->findComponent<sv::StatusComponent>()) {
						sf::Vec3 pos = entity.transform.transformPoint(chr->centerOffset);

						if (!ctx.immediate) {
							systems.effect->spawnOneShotEffect(systems, c->startEffect, pos);
						}

						status.tickEffect = c->tickEffect;
						status.endEffect = c->endEffect;

						if (c->activeEffect) {
							status.effectId = systems.effect->spawnAttachedEffect(systems, c->activeEffect, chr->entityId, chr->centerOffset);
						}
					}
				}
			}

		} else if (const auto *e = event.as<sv::StatusRemoveEvent>()) {

			if (Status *status = findStatus(e->statusId)) {
				if (status->effectId != ~0u) {
					systems.effect->removeAttachedEffect(status->effectId);
				}

				if (status->endEffect) {
					if (Character *chr = findCharacter(status->characterSvId)) {
						Entity &entity = systems.entities.entities[chr->entityId];
						sf::Vec3 pos = entity.transform.transformPoint(chr->centerOffset);
						systems.effect->spawnOneShotEffect(systems, status->endEffect, pos);
					}
				}

				freeStatusIds.push((uint32_t)(status - statuses.data));
				sf::reset(*status);
			}

			svToStatus.removeOne(e->statusId, ~0u);

		} else if (const auto *e = event.as<sv::StatusTickEvent>()) {
			if (Status *status = findStatus(e->statusId)) {

				if (status->tickEffect) {
					if (Character *chr = findCharacter(status->characterSvId)) {
						Entity &entity = systems.entities.entities[chr->entityId];
						sf::Vec3 pos = entity.transform.transformPoint(chr->centerOffset);
						systems.effect->spawnOneShotEffect(systems, status->tickEffect, pos);
					}
				}

			}
		} else if (const auto *e = event.as<sv::MoveEvent>()) {

			uint32_t chrId = svToCharacter.findOne(e->characterId, ~0u);
			if (chrId != ~0u) {
				Character &character = characters[chrId];
				character.tile = e->position;

				if (ctx.immediate || e->instant || e->waypoints.size == 0) {
					Transform transform;
					transform.position = sf::Vec3((float)e->position.x, 0.0f, (float)e->position.y);
					systems.entities.updateTransform(systems, character.entityId, transform);
				} else {
					Entity &entity = systems.entities.entities[character.entityId];
					sf::Vec3 pos = entity.transform.position;

					if (ctx.begin) {
						moveWaypointIndex = 0;
						moveVelocity = sf::Vec3();
						moveEndTime = 0.0f;
					}

					moveVelocity *= exp2f(dt*-13.0f);

					Transform transform = entity.transform;

					bool end = false;
					if (moveWaypointIndex < e->waypoints.size) {
						const sv::Waypoint &wp = e->waypoints[moveWaypointIndex];
						sf::Vec3 target = sf::Vec3((float)wp.position.x, 0.0f, (float)wp.position.y);
						sf::Vec3 delta = target - pos;
						float len = sf::length(delta);

						faceTowardsPosition(transform, target, dt, 10.0f);

						moveVelocity += delta * (dt / sf::max(len, 0.1f)) * 45.0f;

						if (len < 0.5f) {
							moveWaypointIndex++;
						}
					} else {
						sf::Vec3 target = sf::Vec3((float)e->position.x, 0.0f, (float)e->position.y);
						sf::Vec3 delta = target - pos;
						float len = sf::length(delta);

						if (moveEndTime == 0.0f) {
							systems.characterModel->removeTag(systems.entities, character.entityId, symRun);
						}

						moveEndTime = sf::max(moveEndTime + dt * 4.0f, 1.0f);

						moveVelocity += delta * (dt / sf::max(len, 0.5f)) * 10.0f;

						transform.position += delta * sf::min(1.0f, dt / len * moveEndTime);

						if (len < 0.01f) {
							moveVelocity = sf::Vec3(0.0f);
							transform.position = target;
							end = true;
						}
					}

					transform.position += moveVelocity * dt;
					systems.entities.updateTransform(systems, character.entityId, transform);

					if (ctx.begin) {
						systems.characterModel->addTag(systems.entities, character.entityId, symRun);
					}

					return end;
				}
			}

		} else if (const auto *e = event.as<sv::GiveCardEvent>()) {

			if (e->previousOwnerId != e->ownerId && e->previousOwnerId && e->ownerId && !ctx.immediate) {
				CardTrade &trade = cardTrades.push();
				trade.srcSvId = e->previousOwnerId;
				trade.dstSvId = e->ownerId;
				trade.cardSvId = e->cardId;
			}

			if (Character *chr = findCharacter(e->previousOwnerId)) {
				if (uint32_t *ptr = sf::find(chr->cardIds, e->cardId)) {
					chr->cardIds.removeOrderedPtr(ptr);
				}
			}

			if (Character *chr = findCharacter(e->ownerId)) {
				chr->cardIds.push(e->cardId);
			}

		} else if (const auto *e = event.as<sv::SelectCardEvent>()) {
			uint32_t characterId = svToCharacter.findOne(e->ownerId, ~0u);
			uint32_t cardId = svToCard.findOne(e->cardId, ~0u);

			if (characterId != ~0u && cardId != ~0u) {
				equipCardImp(systems, characterId, cardId, e->slot);
			}
		} else if (const auto *e = event.as<sv::UnselectCardEvent>()) {
			if (Character *chr = findCharacter(e->ownerId)) {
				SelectedCard &selected = chr->selectedCards[e->slot];
				if (selected.currentSvId) {
					if (Card *card = findCard(selected.currentSvId)) {
						unequipCardImp(systems, *chr, *card);
					}
					selected.prevSvId = selected.currentSvId;
					selected.currentSvId = 0;
				}
			}
		} else if (const auto *e = event.as<sv::TurnUpdateEvent>()) {

			if (e->turnInfo.startTurn && !ctx.immediate) {
				if (ctx.timer < 0.5f) return false;
			}

			turnInfo = e->turnInfo;
		} else if (const auto *e = event.as<sv::MeleeAttackEvent>()) {
			if (ctx.immediate) return true;

			if (Character *chr = findCharacter(e->meleeInfo.attackerId)) {
				if (ctx.begin) {
					systems.characterModel->addOneShotTag(systems.entities, chr->entityId, symMelee);
				}

				uint32_t targetEntityId = systems.entities.svToEntity.findOne(e->meleeInfo.targetId, ~0u);
				if (targetEntityId != ~0u) {
					Entity &targetEntity = systems.entities.entities[targetEntityId];
					Entity &entity = systems.entities.entities[chr->entityId];

					Transform transform = entity.transform;
					faceTowardsPosition(transform, targetEntity.transform.position, dt, 10.0f);
					systems.entities.updateTransform(systems, chr->entityId, transform);
				}

				sf::SmallArray<sf::Symbol, 16> events;
				systems.characterModel->queryFrameEvents(systems.entities, chr->entityId, events);
				if (sf::find(events, symHit)) return true;
			}

			// Failsafe
			if (ctx.timer < 2.0f) return false;
		} else if (const auto *e = event.as<sv::DamageEvent>()) {
			if (ctx.immediate) return true;

			if (Character *chr = findCharacter(e->damageInfo.targetId)) {
				Entity &entity = systems.entities.entities[chr->entityId];

				if (ctx.begin) {
					systems.characterModel->addOneShotTag(systems.entities, chr->entityId, symStagger);

					DamageNumber &damageNumber = damageNumbers.push();
					damageNumber.text.format("-%u", e->damageRoll.total);
					damageNumber.position = entity.transform.position + chr->centerOffset;
					damageNumber.origin = guiResources.damageFont->measureText(damageNumber.text, DamageNumber::BaseHeight) * sf::Vec2(-0.5f, 0.1f);
				}
			}


			if (ctx.timer < 0.5f) return false;
		} else if (const auto *e = event.as<sv::CastSpellEvent>()) {
			if (ctx.immediate) return true;

			Character *chr = findCharacter(e->spellInfo.casterId);
			if (!chr) return true;
			Entity &casterEntity = systems.entities.entities[chr->entityId];

			if (ctx.begin) {
				castDone = false;
				if (e->spellInfo.manualCast) {
					systems.characterModel->addOneShotTag(systems.entities, chr->entityId, symCast);
					castAnimDone = false;

					if (sv::Prefab *spellPrefab = systems.entities.findPrefab(e->spellInfo.spellName)) {
						if (auto *spellComp = spellPrefab->findComponent<sv::SpellComponent>()) {
							if (spellComp->castEffect) {
								uint32_t castEffectEntityId = systems.effect->spawnOneShotEffect(systems, spellComp->castEffect, sf::Vec3());
								if (castEffectEntityId != ~0u) {
									CharacterModelSystem::AttachDesc desc;
									desc.boneName = symEffect;
									systems.characterModel->addAttachedEntity(systems, chr->entityId, castEffectEntityId, desc);
								}
							}
						}
					}

				} else {
					castAnimDone = true;
				}
			}

			if (!castAnimDone) {
				sf::SmallArray<sf::Symbol, 16> events;
				systems.characterModel->queryFrameEvents(systems.entities, chr->entityId, events);
				if (sf::find(events, symCast)) castAnimDone = true;

				uint32_t targetEntityId = systems.entities.svToEntity.findOne(e->spellInfo.targetId, ~0u);
				if (targetEntityId != ~0u) {
					Entity &targetEntity = systems.entities.entities[targetEntityId];
					Entity &entity = systems.entities.entities[chr->entityId];

					Transform transform = entity.transform;
					faceTowardsPosition(transform, targetEntity.transform.position, dt, 10.0f);
					systems.entities.updateTransform(systems, chr->entityId, transform);
				}


				// Failsafe
				if (ctx.timer >= 2.0f) castAnimDone = true;
			}

			if (!castAnimDone) return false;

			if (!castDone) {
				castDone = true;
				if (sv::Prefab *spellPrefab = systems.entities.findPrefab(e->spellInfo.spellName)) {
					for (sv::Component *comp : spellPrefab->components) {

						if (auto *c = comp->as<sv::ProjectileComponent>()) {
							Projectile &projectile = projectiles.push();

							Transform transform;
							transform.position = casterEntity.transform.transformPoint(chr->centerOffset);

							projectile.entityId = systems.entities.addEntity(systems, 0, transform, c->prefabName);
							projectile.hitEffect = c->hitEffect;
							projectile.targetSvId = e->spellInfo.targetId;

							projectile.flightSpeed = c->flightSpeed;
						}
					}
				}
			}

			// Failsafe 2
			if (ctx.timer >= 5.0f) return true;

			if (projectiles.size > 0) return false;
		}

		return true;
	}

	// -- API

	GameSystemImp(const SystemsDesc &desc)
	{
		camera.previous.origin = camera.current.origin = sf::Vec3(desc.persist.camera.x, 0.0f, desc.persist.camera.y);
		camera.previous.zoom = camera.current.zoom = desc.persist.zoom;

		guiRoot = sf::box<gui::Widget>(1000);
	}

	void updateCamera(FrameArgs &frameArgs) override
	{
		InputUpdateArgs inputArgs;
		inputArgs.dt = frameArgs.dt;
		inputArgs.resolution = sf::Vec2(frameArgs.resolution);
		inputArgs.clipToWorld = sf::inverse(frameArgs.mainRenderArgs.worldToClip);
		#if !defined(SP_NO_APP)
			inputArgs.dpiScale = sapp_dpi_scale();
		#endif
		inputArgs.events = frameArgs.events;
		inputArgs.mouseBlocked = ImGui::GetIO().WantCaptureMouse;
		inputArgs.keyboardBlocked = ImGui::GetIO().WantCaptureKeyboard;
		inputArgs.simulateTouch = simulateTouch;

		input.update(inputArgs);

		if (input.keyDown[SAPP_KEYCODE_F3] && !input.prevKeyDown[SAPP_KEYCODE_F3]) {
			showDebugMenu = true;
		}

		float scrollAmount = 0.0f;

		for (const sapp_event &e : frameArgs.events) {

			if (e.type == SAPP_EVENTTYPE_MOUSE_SCROLL) {

				if (!input.mouseBlocked) {
					scrollAmount += -e.scroll_y;
				}

			}
		}

		for (DragPointer &dragPointer : dragPointers) {
			dragPointer.active = false;
		}

		for (Pointer &p : input.pointers) {
			PointerState &ps = pointerStates[p.id];
			ps.id = p.id;
			ps.active = true;
			ps.hitGuiThisFrame = false;

			if (ps.hitBackground) continue;

			gui::GuiPointer gp;
			gp.id = p.id;
			gp.position = p.current.pos * frameArgs.guiResolution;
			gp.delta = (p.current.pos - p.prev.pos) * frameArgs.guiResolution;
			gp.button = (gui::GuiPointer::Button)p.button;
            gp.canTap = p.canTap;

			if (p.action == Pointer::Down) {
				gp.action = gui::GuiPointer::Down;
			} else if (p.action == Pointer::Hold) {
				if (p.canTap && p.time > 0.4f) {
					gp.action = gui::GuiPointer::LongPress;
				} else if (p.dragFactor > 0.0f) {
					gp.action = gui::GuiPointer::Drag;
					gp.dragFactor = p.dragFactor;
				} else {
					gp.action = gui::GuiPointer::Hold;
				}
			} else if (p.action == Pointer::Up) {
				if (p.canTap && p.time < 0.5f) {
					gp.action = gui::GuiPointer::Tap;
					gp.end = true;
				} else {
					gp.action = gui::GuiPointer::Up;
					gp.end = true;
				}
			} else if (p.action == Pointer::Cancel) {
				gp.action = gui::GuiPointer::Cancel;
				gp.end = true;
			}

			if (ps.trackWidget) {
				gp.trackWidget = ps.trackWidget;
				ps.trackWidget->onPointer(gp);
			} else if (gp.action != gui::GuiPointer::NoAction) {
				bool ate = guiRoot->onPointer(gp);
				if (gp.trackWidget) {
					ps.trackWidget = gp.trackWidget;
				}
				if (gp.trackWidget || ate || gp.blocked) {
					ps.hitGui = true;
					ps.hitGuiThisFrame = true;
				}
			}

			if (gp.dropType) {
				DragPointer *dragPointer = nullptr;
				for (DragPointer &d : dragPointers) {
					if (d.guiPointer.id == gp.id) {
						dragPointer = &d;
						break;
					}
				}
				if (!dragPointer) {
					dragPointer = &dragPointers.push();
					dragPointer->smoothPosition = gp.position;
				} else {
					sf::Vec2 delta = dragPointer->guiPointer.delta;
					gui::lerpExp(dragPointer->smoothVelocity, delta, 10.0f, 5.0f, frameArgs.dt);
					gui::lerpExp(dragPointer->smoothPosition, gp.position, 35.0f, 10.0f, frameArgs.dt);
				}

				dragPointer->active = true;
				gui::GuiPointer &gp2 = dragPointer->guiPointer;
				gp2.id = gp.id;
				gp2.position = gp.position;
				gp2.delta = gp.delta;
				gp2.dragFactor = gp.dragFactor;
				gp2.button = gp.button;
				if (gp.action == gui::GuiPointer::Up || gp.action == gui::GuiPointer::Tap) {
					gp2.action = gui::GuiPointer::DropCommit;
					gp2.end = true;
				} else if (gp.action == gui::GuiPointer::Cancel) {
					gp2.action = gui::GuiPointer::DropCancel;
					gp2.end = true;
				} else {
					gp2.action = gui::GuiPointer::DropHover;
				}
				gp2.dropType = std::move(gp.dropType);
				gp2.dropData = std::move(gp.dropData);
				gp2.dropOffset = gp.dropOffset;
				gp2.dropSize = gp.dropSize;
			}
		}

		for (uint32_t i = 0; i < dragPointers.size; i++) {
			DragPointer &dragPointer = dragPointers[i];
			if (dragPointer.active) {
				dragPointer.time += frameArgs.dt;
				bool ate = guiRoot->onPointer(dragPointer.guiPointer);
				if (dragPointer.guiPointer.trackWidget || ate || dragPointer.guiPointer.blocked) {
					dragPointer.hitGuiThisFrame = true;
				} else {
					dragPointer.hitGuiThisFrame = false;
				}
			} else {
				dragPointers.removeSwap(i--);
			}
		}

		if (sf::abs(scrollAmount) > 0.0f) {
			Pointer *mousePointer = nullptr;
			for (Pointer &p : input.pointers) {
				if (p.button <= Pointer::LastMouse) {
					mousePointer = &p;
					break;
				}
			}

			if (mousePointer) {
				gui::GuiPointer gp;
				gp.position = mousePointer->current.pos * frameArgs.guiResolution;
				gp.delta = (mousePointer->current.pos - mousePointer->prev.pos) * frameArgs.guiResolution;
				gp.button = (gui::GuiPointer::Button)mousePointer->button;
				gp.action = gui::GuiPointer::Scroll;
				gp.scrollDelta = scrollAmount;
				if (guiRoot->onPointer(gp)) {
					scrollAmount = 0.0f;
				}
			}
		}

		camera.zoomDelta += scrollAmount * 0.1f;

		const float cameraDt = 0.001f;
		camera.timeDelta = sf::min(camera.timeDelta + frameArgs.dt, 0.1f);

		float aspect = (float)frameArgs.resolution.x / (float)frameArgs.resolution.y;

		static const float cameraExp = 0.02f;
		static const float cameraLinear = 0.00005f;
		static const float decayExp = 0.008f;
		static const float decayLinear = 0.00005f;
		static const float decayTouchExp = 0.004f;
		static const float decayTouchLinear = 0.00005f;

		sf::Vec2 cameraMove;

		{
			if (input.keyDown[SAPP_KEYCODE_A] || input.keyDown[SAPP_KEYCODE_LEFT]) cameraMove.x -= 1.0f;
			if (input.keyDown[SAPP_KEYCODE_D] || input.keyDown[SAPP_KEYCODE_RIGHT]) cameraMove.x += 1.0f;
			if (input.keyDown[SAPP_KEYCODE_W] || input.keyDown[SAPP_KEYCODE_UP]) cameraMove.y -= 1.0f;
			if (input.keyDown[SAPP_KEYCODE_S] || input.keyDown[SAPP_KEYCODE_DOWN]) cameraMove.y += 1.0f;

			if (sf::lengthSq(cameraMove) > 1.0f) {
				cameraMove = sf::normalize(cameraMove);
			}
		}

		while (camera.timeDelta >= cameraDt) {
			camera.timeDelta -= cameraDt;
			camera.previous = camera.current;
            
            camera.zoomDelta *= 0.99f;
            camera.zoomDelta -= sf::clamp(camera.zoomDelta, -0.0005f, 0.0005f);

			camera.velocity *= 0.99f;

			float accelSpeed = exp2f(camera.current.zoom * 0.5f) * cameraDt * 80.0f;
			camera.velocity.x += cameraMove.x * accelSpeed;
			camera.velocity.z += cameraMove.y * accelSpeed;

			sf::Vec3 eye;
			sf::Mat34 worldToView;
			sf::Mat44 viewToClip;
			camera.current.asMatrices(eye, worldToView, viewToClip, aspect);
			sf::Mat44 clipToWorld = sf::inverse(viewToClip * worldToView);

            uint32_t numDrags = 0;
            sf::Vec3 dragStart;
            sf::Vec3 dragCurrent;

			for (Pointer &p : input.pointers.slice()) {
				PointerState &ps = pointerStates[p.id];
				if (ps.hitGui) continue;

                if ((p.button == Pointer::MouseMiddle || p.button == Pointer::Touch)
                    && p.action == Pointer::Hold) {
					ps.hitBackground = true;

					p.current.worldRay = pointerToWorld(clipToWorld, p.current.pos);

					if (p.button == Pointer::Touch) {
						camera.touchMove += cameraDt * 4.0f;
					} else {
						camera.touchMove -= cameraDt * 4.0f;
					}
					camera.touchMove = sf::clamp(camera.touchMove, 0.0f, 1.0f);

					sf::Vec3 a = intersectHorizontalPlane(0.0f, p.dragStart.worldRay);
					sf::Vec3 b = intersectHorizontalPlane(0.0f, p.current.worldRay);
					float dragFactor = sf::max(0.0f, p.dragFactor);
					if (p.button == Pointer::MouseMiddle) dragFactor = 1.0f;
					if (dragFactor < 1.0f) b = sf::lerp(a, b, dragFactor);
			
                    numDrags += 1;
                    dragStart += a;
                    dragCurrent += b;
				}
			}
            
            if (numDrags > 0) {
                dragStart /= (float)numDrags;
                dragCurrent /= (float)numDrags;
                
                if (numDrags > 1) {
                    camera.touchMove -= cameraDt * 20.0f * (float)numDrags;
                    camera.touchMove = sf::clamp(camera.touchMove, 0.0f, 1.0f);

                    float dragZoom = 0.0f;
                    for (Pointer &p : input.pointers) {
						PointerState &ps = pointerStates[p.id];
						if (ps.hitGui) continue;
    
                        if ((p.button == Pointer::MouseMiddle || p.button == Pointer::Touch)
                            && p.action == Pointer::Hold) {

                            sf::Vec3 a = intersectHorizontalPlane(0.0f, p.dragStart.worldRay);
                            sf::Vec3 b = intersectHorizontalPlane(0.0f, p.current.worldRay);
							float dragFactor = sf::max(0.0f, p.dragFactor);
							if (p.button == Pointer::MouseMiddle) dragFactor = 1.0f;
							if (dragFactor < 1.0f) b = sf::lerp(a, b, dragFactor);
    
                            float ad = sf::length(a - dragStart);
                            float bd = sf::length(b - dragCurrent);
    
                            if (ad > 0.1f && bd > 0.1f) {
                                float zoom = log2f(ad / bd);

                                dragZoom += zoom;
                            }
                        }
                    }
                    dragZoom /= (float)numDrags;
                    camera.zoomDelta = dragZoom;
                }
                
                camera.targetDelta = dragStart - dragCurrent;
            }

			sf::Vec3 delta = camera.targetDelta;
			float deltaLen = sf::length(delta);

			camera.current.origin += camera.velocity * cameraDt;
            
			camera.zoomDelta = sf::clamp(camera.zoomDelta, -10.0f, 10.0f);
            camera.current.zoom += camera.zoomDelta * 0.01f;
            camera.current.zoom = sf::clamp(camera.current.zoom, -1.5f, 1.5f);

			if (deltaLen > 0.00001f) {
				float applyLen = sf::min(cameraLinear + deltaLen*cameraExp, deltaLen);
				camera.current.origin += delta * (applyLen / deltaLen);

				{
					float exp = sf::lerp(decayExp, decayTouchExp, camera.touchMove);
					float lin = sf::lerp(decayLinear, decayTouchLinear, camera.touchMove);
					float decayLen = sf::min(lin + deltaLen*exp, deltaLen);
					camera.targetDelta -= camera.targetDelta * (decayLen / deltaLen);
				}

			} else {
				camera.current.origin += delta;
				camera.targetDelta = sf::Vec3();
			}
		}

		if (showDebugMenu) {
			ImGui::SetNextWindowSize(ImVec2(200.0f, 200.0f), ImGuiCond_Appearing);
			if (ImGui::Begin("Game Debug", &showDebugMenu)) {
				ImGui::Checkbox("Simulate touch", &simulateTouch);
				if (ImGui::Button("Pointers")) showDebugPointers = true;
			}
			ImGui::End();
		}

		if (showDebugPointers) {
			ImGui::SetNextWindowSize(ImVec2(400.0f, 100.0f), ImGuiCond_Appearing);
			if (ImGui::Begin("Pointers"), &showDebugPointers) {
				sf::SmallStringBuf<128> str;
				for (Pointer &p : input.pointers) {
					str.clear();
					p.formatDebugString(str);
					ImGui::Text("%s", str.data);
				}
			}
			ImGui::End();
		}

		Camera::State state = Camera::lerp(camera.previous, camera.current, camera.timeDelta / cameraDt);

		sf::Vec3 eye;
		sf::Mat34 worldToView;
		sf::Mat44 viewToClip;
		state.asMatrices(eye, worldToView, viewToClip, aspect);
		sf::Mat44 worldToClip = viewToClip * worldToView;

		sf::Mat44 clipToWorld = sf::inverse(worldToClip);
		for (Pointer &p : input.pointers) {
			p.current.worldRay = pointerToWorld(clipToWorld, p.current.pos);
		}

		frameArgs.mainRenderArgs.cameraPosition = eye;
		frameArgs.mainRenderArgs.worldToView = worldToView;
		frameArgs.mainRenderArgs.viewToClip = viewToClip;
		frameArgs.mainRenderArgs.worldToClip = worldToClip;
		frameArgs.mainRenderArgs.frustum = sf::Frustum(frameArgs.mainRenderArgs.worldToClip, sp::getClipNearW());
	}

	void writePersist(Systems &systems, ClientPersist &persist) override
	{
		persist.camera.x = camera.current.origin.x;
		persist.camera.y = camera.current.origin.z;
		persist.zoom = camera.current.zoom;
	}

	void update(const sv::ServerState &svState, Systems &systems, const FrameArgs &frameArgs) override
	{
		while (queuedEvents.size > 0) {
			if (!applyEventImp(systems, *queuedEvents[0], queuedEventContext)) {
				queuedEventContext.begin = false;
				queuedEventContext.timer += frameArgs.dt;
				break;
			} else {
				queuedEvents.removeOrdered(0); // TODO( O(n^2) )
				queuedEventContext.begin = true;
				queuedEventContext.timer = 0.0f;
			}
		}

		for (uint32_t i = 0; i < 9; i++) {
			if (input.keyDown[SAPP_KEYCODE_1 + i] && !input.prevKeyDown[SAPP_KEYCODE_1 + i]) {
				if (selectedCardSlot != i) {
					selectedCardSlot = i;
				} else {
					selectedCardSlot = ~0u;
				}
			}
		}

		// Update projectiles
		for (uint32_t i = 0; i < projectiles.size; i++) {
			Projectile &projectile = projectiles[i];
			Entity &entity = systems.entities.entities[projectile.entityId];

			sf::Vec3 targetPos = entity.transform.position;

			if (Character *targetChr = findCharacter(projectile.targetSvId)) {
				Entity &targetEntity = systems.entities.entities[targetChr->entityId];
				targetPos = targetEntity.transform.transformPoint(targetChr->centerOffset);
			} else {
				uint32_t targetEntityId = systems.entities.svToEntity.findOne(projectile.targetSvId, ~0u);
				if (targetEntityId != ~0u) {
					Entity &targetEntity = systems.entities.entities[targetEntityId];
					targetPos = targetEntity.transform.position;
				}
			}

			sf::Vec3 delta = targetPos - entity.transform.position;
			float deltaLen = sf::length(delta);

			float speed = projectile.flightSpeed * frameArgs.dt;
			if (deltaLen <= speed) {
				if (projectile.hitEffect) {
					systems.effect->spawnOneShotEffect(systems, projectile.hitEffect, targetPos);
				}

				entity.transform.position = targetPos;
				systems.entities.removeEntityQueued(projectile.entityId);
				projectiles.removeSwap(i--);
			} else {
				Transform transform = entity.transform;
				transform.position += delta * (speed / deltaLen);
				systems.entities.updateTransform(systems, projectile.entityId, transform);
			}
		}

		// Verify selected card
		if (selectedCardSlot != ~0u) {
			bool valid = false;
			if (Character *chr = findCharacter(selectedCharacterId)) {
				if (chr->selectedCards[selectedCardSlot].currentSvId) {
					valid = true;
				}
			}

			if (dragPointers.size > 0) {
				valid = false;
			}

			if (!valid) {
				selectedCardSlot = ~0u;
			}
		}

		if (selectedCardSlot != ~0u) {
			selectedCardTime = sf::min(1.0f, selectedCardTime + frameArgs.dt * 4.0f);
		} else {
			selectedCardTime = sf::max(0.0f, selectedCardTime - frameArgs.dt * 4.0f);
		}

		if (Character *chr = findCharacter(turnInfo.characterId)) {
			sf::Vec3 tilePos = sf::Vec3((float)chr->tile.x, 0.0f, (float)chr->tile.y);
			sf::Vec4 color = sf::Vec4(0.8f, 0.8f, 0.8f, 1.0f) * 0.3f;
			sf::Mat34 t;
			t.cols[0] = sf::Vec3(1.0f, 0.0f, 0.0f);
			t.cols[1] = sf::Vec3(0.0f, 0.0f, 1.0f);
			t.cols[2] = sf::Vec3(0.0f, 1.0f, 0.0f);
			t.cols[3] = tilePos + sf::Vec3(0.0f, 0.05f, 0.0f);
			systems.billboard->addBillboard(guiResources.characterActive, t, color, 1.0f);
		}

		for (Pointer &pointer : input.pointers) {
			PointerState *pointerState = pointerStates.find(pointer.id);
			if (!pointerState) continue;

			uint32_t svId = 0;
			uint32_t entityId = systems.tapArea->getClosestTapAreaEntity(systems.area, pointer.current.worldRay);
			if (entityId != ~0u) {
				Entity &entity = systems.entities.entities[entityId];
				svId = entity.svId;
			}

			if (pointer.action == Pointer::Down) {
				pointerState->startSvId = svId;
			}
			pointerState->currentSvId = svId;

			didHoverTile = false;

			if (!pointerState->hitGui) {
				bool didClick = false;
				bool didHover = false;

				if (pointer.button == Pointer::Touch && pointer.action == Pointer::Up && pointer.time < 0.3f && pointer.canTap) didClick = true;
				if (pointer.button == Pointer::MouseLeft && pointer.action == Pointer::Down && pointer.canTap) didClick = true;
				if (pointer.button == Pointer::MouseHover && pointer.action == Pointer::Down) didHover = true;

				if (frameArgs.editorOpen) {
					didClick = false;
					didHover = false;
				}

				Character *chr = findCharacter(pointerState->startSvId);
				Character *caster = nullptr;
				Card *card = nullptr;
				bool canTarget = true;
				if (selectedCardSlot != ~0u) {
					caster = findCharacter(selectedCharacterId);
					card = findCard(caster->selectedCards[selectedCardSlot].currentSvId);
					if (chr && caster && card && svState.canTarget(caster->svId, chr->svId, card->svPrefab->name)) {
						canTarget = true;
					} else {
						canTarget = false;
					}
				}

				if (chr && canTarget) {
					if (didClick) {

						if (selectedCardSlot != ~0u) {
							auto action = sf::box<sv::UseCardAction>();

							action->characterId = selectedCharacterId;
							action->targetId = chr->svId;
							action->cardId = caster->selectedCards[selectedCardSlot].currentSvId;

							requestedActions.push(std::move(action));
							selectedCardSlot = ~0u;
						} else {
							if (chr->svId != selectedCharacterId) {
								selectedCardSlot = ~0u;
								selectedCharacterId = chr->svId;
								selectedCharacterTime = 0.0f;
								moveSelectTime = 0.0f;
							}
						}

					} else if (didHover) {
						bool showHover = false;

						if (chr->svId != selectedCharacterId || selectedCardSlot != ~0u) {
							showHover = true;
						}

						if (showHover) {
							sf::Vec3 tilePos = sf::Vec3((float)chr->tile.x, 0.0f, (float)chr->tile.y);
							sf::Vec4 color = sf::Vec4(0.8f, 0.8f, 0.8f, 1.0f) * 0.3f;
							sf::Mat34 t;
							t.cols[0] = sf::Vec3(1.0f, 0.0f, 0.0f);
							t.cols[1] = sf::Vec3(0.0f, 0.0f, 1.0f);
							t.cols[2] = sf::Vec3(0.0f, 1.0f, 0.0f);
							t.cols[3] = tilePos + sf::Vec3(0.0f, 0.05f, 0.0f);
							systems.billboard->addBillboard(guiResources.characterSelect, t, color, 1.0f);
						}
					}
				} else if (moveSet.distanceToTile.size() > 0 && selectedCardSlot == ~0u) {

					sf::Vec3 rayPos = intersectHorizontalPlane(0.0f, pointer.current.worldRay);
					sf::Vec2 tileF = sf::Vec2(rayPos.x, rayPos.z);
					sf::Vec2i tile = sf::Vec2i(sf::floor(tileF + sf::Vec2(0.5f)));

					if (sv::ReachableTile *reach = moveSet.distanceToTile.findValue(tile)) {
						if (didClick) {
							auto action = sf::box<sv::MoveAction>();

							action->characterId = selectedCharacterId;
							action->tile = tile;
							action->waypoints.resizeUninit(reach->distance);
							uint32_t waypointIx = action->waypoints.size;
							action->waypoints[--waypointIx] = tile;

							sf::Vec2i prev = reach->previous;
							while (sv::ReachableTile *prevReach = moveSet.distanceToTile.findValue(prev)) {
								action->waypoints[--waypointIx] = prev;
								prev = prevReach->previous;
							}

							requestedActions.push(std::move(action));

						} else if (didHover) {
							hoveredTile = tile;
							didHoverTile = true;

							float &amount = tileHoverAmount[tile];
							amount = sf::min(1.0f, amount + frameArgs.dt * 20.0f);

							sf::Vec2i prev = reach->previous;
							while (sv::ReachableTile *prevReach = moveSet.distanceToTile.findValue(prev)) {
								float &amount = tileTrailAmount[prev];
								amount = sf::min(1.0f, amount + frameArgs.dt * 3.0f);
								prev = prevReach->previous;
							}
						}
					}
				}
			}
		}

		for (uint32_t i = 0; i < tileHoverAmount.size(); i++) {
			auto &pair = tileHoverAmount.data[i];
			pair.val -= frameArgs.dt * 10.0f;
			if (pair.val <= 0.0f) {
				tileHoverAmount.remove(pair.key);
				i--;
			}
		}

		for (uint32_t i = 0; i < tileTrailAmount.size(); i++) {
			auto &pair = tileTrailAmount.data[i];
			pair.val -= frameArgs.dt * 2.0f;
			if (pair.val <= 0.0f) {
				tileTrailAmount.remove(pair.key);
				i--;
			}
		}

		for (DragPointer &dragPointer : dragPointers) {
			PointerState *pointerState = pointerStates.find(dragPointer.guiPointer.id);
			if (!pointerState || pointerState->hitGuiThisFrame) continue;

			if (dragPointer.guiPointer.dropType == guiCardSym) {
				GuiCard *guiCard = dragPointer.guiPointer.dropData.cast<GuiCard>();
				if (Character *chr = findCharacter(pointerState->currentSvId)) {
					sf::Vec3 tilePos = sf::Vec3((float)chr->tile.x, 0.0f, (float)chr->tile.y);

					sf::Vec4 color = sf::Vec4(0.8f, 0.8f, 1.0f, 1.0f) * 0.7f;
					sf::Mat34 t;
					t.cols[0] = sf::Vec3(1.0f, 0.0f, 0.0f);
					t.cols[1] = sf::Vec3(0.0f, 0.0f, 1.0f);
					t.cols[2] = sf::Vec3(0.0f, 1.0f, 0.0f);
					t.cols[3] = tilePos + sf::Vec3(0.0f, 0.05f, 0.0f);
					systems.billboard->addBillboard(guiResources.characterSelect, t, color, 1.0f);

					if (dragPointer.guiPointer.action == gui::GuiPointer::DropCommit) {
						auto action = sf::box<sv::GiveCardAction>();
						action->ownerId = chr->svId;
						action->cardId = guiCard->svId;
						requestedActions.push(std::move(action));
					}
				}
			}
		}

		if (queuedEvents.size > 0) {
			moveSet.distanceToTile.clear();
			moveSelectTime = 0.0f;
		}

		if (Character *chr = findCharacter(selectedCharacterId)) {
			sf::Vec3 tilePos = sf::Vec3((float)chr->tile.x, 0.0f, (float)chr->tile.y);
			float t = selectedCharacterTime;
			selectedCharacterTime += frameArgs.dt;

			{
				float fade = gui::smoothEnd(sf::min(t*7.0f, 1.0f));
				float scale = 1.0f + (1.0f - fade) * 0.4f;
				float alpha = fade;
				sf::Vec4 col = sf::Vec4(0.8f, 0.5f, 0.3f, 1.0f) * alpha;
				sp::Sprite *sprite = guiResources.characterSelect;
				sf::Mat34 t;
				t.cols[0] = sf::Vec3(1.0f, 0.0f, 0.0f) * scale;
				t.cols[1] = sf::Vec3(0.0f, 0.0f, 1.0f) * scale;
				t.cols[2] = sf::Vec3(0.0f, 1.0f, 0.0f);
				t.cols[3] = tilePos + sf::Vec3(0.0f, 0.05f, 0.0f);
				systems.billboard->addBillboard(sprite, t, col);
			}

			if (turnInfo.movementLeft > 0 && turnInfo.characterId == chr->svId && moveSelectTime > 0.0f && !frameArgs.editorOpen) {
				sv::PathfindOpts opts;
				opts.isBlockedFn = &sv::isBlockedByPropOrCharacter;
				opts.maxDistance = turnInfo.movementLeft;
				sv::findReachableSet(moveSet, svState, opts, chr->tile);

				static const sf::Vec3 col0 = sp::srgbToLinearHex(0xfaf3dd);
				static const sf::Vec3 col1 = sp::srgbToLinearHex(0xc8d5b9);
				static const sf::Vec3 col2 = sp::srgbToLinearHex(0xbfdcae);
				static const sf::Vec3 colors[] = {
					sf::lerp(col0, col1, 1.0f/4.0f),
					sf::lerp(col0, col1, 2.0f/4.0f),
					sf::lerp(col0, col1, 3.0f/4.0f),
					sf::lerp(col0, col1, 4.0f/4.0f),
					sf::lerp(col1, col2, 1.0f/4.0f),
					sf::lerp(col1, col2, 2.0f/4.0f),
					sf::lerp(col1, col2, 3.0f/4.0f),
					sf::lerp(col1, col2, 4.0f/4.0f),
				};

				sf::Vec4 inactiveColor = sf::Vec4(0.5f, 0.5f, 0.5f, 1.0f) * 0.3f;

				sf::SmallArray<float, 64> wave;
				for (uint32_t i = 0; i <= opts.maxDistance; i++) {
					float t = moveSelectTime * 2.0f - (float)i * 0.9f;
					wave.push(powf(sinf(t) * 0.5f + 0.5f, 3.0f));
				}

				for (auto &pair : moveSet.distanceToTile) {
					sf::Vec2i tile = pair.key;

					float t = moveSelectTime - (float)pair.val.distance * 0.03f;

					float w = wave[pair.val.distance];

					float fade = gui::smoothEnd(sf::clamp(t * 8.0f, 0.0f, 1.0f));
					sf::Vec4 color = sf::Vec4(colors[sf::min(pair.val.distance, (uint32_t)sf_arraysize(colors) - 1)], 1.0f);
					color *= fade * (w * 0.5f + 0.5f);

					color = sf::lerp(color, inactiveColor, gui::smoothStep(selectedCardTime));

					float scale = 1.0f - (1.0f - fade) * 0.25f;

					if (float *amount = tileTrailAmount.findValue(tile)) {
						float t = gui::smoothStep(*amount);
						color = sf::lerp(color, sf::Vec4(1.0f), gui::smoothStep(t * w * 0.5f));
					}

					if (float *amount = tileHoverAmount.findValue(tile)) {
						float t = gui::smoothStep(*amount);
						color = sf::lerp(color, sf::Vec4(1.0f), t);
						scale *= sf::lerp(1.0f, 0.5f, t);
					}

					
					sf::Vec3 tilePos = sf::Vec3((float)pair.key.x, 0.0f, (float)pair.key.y);
					sf::Vec3 prevPos = sf::Vec3((float)pair.val.previous.x, 0.0f, (float)pair.val.previous.y);


					sf::Mat34 mat;
					mat.cols[0] = sf::Vec3(1.0f, 0.0f, 0.0f) * scale;
					mat.cols[1] = sf::Vec3(0.0f, 0.0f, 1.0f) * scale;
					mat.cols[2] = sf::Vec3(0.0f, 1.0f, 0.0f);
					mat.cols[3] = tilePos + sf::Vec3(0.0f, 0.05f, 0.0f);
					systems.billboard->addBillboard(guiResources.characterMove, mat, color);
				}
			}

			moveSelectTime += frameArgs.dt;
		}

		{
			gui::GuiBuilder &b = guiBuilder;
			b.init(guiRoot);

			Character *chr = findCharacter(selectedCharacterId);
			sv::CharacterComponent *chrComp = chr ? chr->svPrefab->findComponent<sv::CharacterComponent>() : NULL;
			if (chr && chrComp) {

				if (turnInfo.characterId == selectedCharacterId && !frameArgs.editorOpen) {
					auto bt = b.push<gui::WidgetButton>();
					if (bt->created) {
						bt->text = sf::Symbol("End Turn");
						bt->font = guiResources.buttonFont;
						bt->sprite = guiResources.buttonSprite;
						bt->fontHeight = 40.0f;
					}
					bt->boxOffset = sf::Vec2(10.0f, 10.0f);
					bt->boxExtent = sf::Vec2(140.0f, 60.0f);

					if (bt->pressed) {
						auto action = sf::box<sv::EndTurnAction>();
						action->characterId = chr->svId;
						requestedActions.push(std::move(action));
					}

					b.pop();
				}

				{
					float hotbarCardHeight = 140.0f;

					auto blk = b.push<gui::WidgetBlockPointer>(chr->svId);
					auto ll = b.push<gui::WidgetLinearLayout>(chr->svId);

					ll->boxExtent = sf::Vec2(gui::Inf, hotbarCardHeight);
					ll->direction = gui::DirX;
					ll->padding = 10.0f;

					blk->boxOffset.x = 20.0f;
					blk->boxOffset.y = frameArgs.guiResolution.y - ll->boxExtent.y - 20.0f;

					uint32_t lastMeleeSlot = chrComp->meleeSlots;
					uint32_t lastSkillSlot = lastMeleeSlot + chrComp->skillSlots;
					uint32_t lastSpellSlot = lastSkillSlot + chrComp->spellSlots;
					uint32_t lastItemSlot = lastSpellSlot + chrComp->itemSlots;

					sf::SmallArray<gui::WidgetCardSlot*, 16> slots;

					for (uint32_t slot = 0; slot < sv::NumSelectedCards; slot++) {
						GuiCardSlot guiSlot = GuiCardSlot::Count;
						if (slot < lastMeleeSlot) {
							guiSlot = GuiCardSlot::Melee;
						} else if (slot < lastSkillSlot) {
							guiSlot = GuiCardSlot::Skill;
						} else if (slot < lastSpellSlot) {
							guiSlot = GuiCardSlot::Spell;
						} else if (slot < lastItemSlot) {
							guiSlot = GuiCardSlot::Item;
						}


						if (guiSlot != GuiCardSlot::Count) {
							auto sl = b.push<gui::WidgetCardSlot>(slot);
							slots.push(sl);

							if (sl->created) {
								sl->startAnim = 1.0f + (float)slot * 0.2f;
							}

							sl->slot = guiSlot;
							sl->slotIndex = slot;

							if (sl->wantSelect) {
								if (selectedCardSlot != slot) {
									selectedCardSlot = slot;
								} else {
									selectedCardSlot = ~0u;
								}
							}

							uint32_t cardId = chr->selectedCards[slot].currentSvId;
							if (Card *card = findCard(cardId)) {
								sl->card = card->gui;
							} else {
								sl->card.reset();
							}

							if (sl->droppedCard) {
								auto action = sf::box<sv::SelectCardAction>();
								action->ownerId = chr->svId;
								action->slot = slot;
								action->cardId = sl->droppedCard->svId;
								requestedActions.push(std::move(action));
								sl->droppedCard.reset();
							}

							b.pop(); // CardSlot
						}
					}

					for (uint32_t i = 0; i < slots.size; i++) {
						slots[i]->selected = i == selectedCardSlot;
					}

					b.pop(); // LinearLayout
					b.pop(); // BlockPointer
				}

				auto inventoryButton = b.push<gui::WidgetToggleButton>(chr->svId);
				if (inventoryButton->created) {
					inventoryButton->inactiveSprite = guiResources.inventory;
					inventoryButton->activeSprite = guiResources.inventoryOpen;
				} else {
					if (input.keyDown[SAPP_KEYCODE_E] && !input.prevKeyDown[SAPP_KEYCODE_E]) {
						inventoryButton->active = !inventoryButton->active;
					}
				}
				inventoryButton->boxOffset.x = frameArgs.guiResolution.x - 140.0f;
				inventoryButton->boxOffset.y = frameArgs.guiResolution.y - 140.0f;
				inventoryButton->boxExtent.x = 120.0f;
				inventoryButton->boxExtent.y = 120.0f;

				b.pop(); // WidgetToggleButton

				if (inventoryButton->active) {
					const uint32_t numCols = 3;
					float inventoryCardWidth = 150.0f;

					auto sc = b.push<gui::WidgetScroll>();
					sc->scrollSpeed = 250.0f;

					auto gl = b.push<gui::WidgetGridLayout>();
					gl->direction = gui::DirY;
					gl->margin = 10.0f;
					gl->padding = 10.0f;

					sc->boxExtent.x = inventoryCardWidth*(float)numCols + gl->padding*(float)numCols + 2.0f*gl->margin;
					sc->boxExtent.y = frameArgs.guiResolution.y - 160.0f - 20.0f;
					sc->boxOffset.x = frameArgs.guiResolution.x - sc->boxExtent.x - 20.0f;
					sc->boxOffset.y = 20.0f;
					sc->direction = gui::DirY;

					uint32_t numCards = sf::max(numCols * 3, chr->cardIds.size);
					numCards += (numCols - numCards % numCols) % numCols;
					for (uint32_t i = 0; i < numCards; i++) {
						auto cd = b.push<gui::WidgetCard>(i);
						cd->boxExtent = sf::Vec2(inventoryCardWidth, gui::Inf);
						cd->card.reset();
						if (i < chr->cardIds.size) {
							if (Card *card = findCard(chr->cardIds[i])) {
								cd->card = card->gui;
							}
						}

						b.pop();
					}

					b.pop(); // LinearLayout
					b.pop(); // Scroll
				}
			}

			b.finish();
		}

		{
			gui::GuiLayout layout;
			layout.dt = frameArgs.dt;
			layout.frameIndex = frameArgs.frameIndex;
			layout.resources = &guiResources;
			guiRoot->layout(layout, frameArgs.guiResolution, frameArgs.guiResolution);
			gui::Widget::finishLayout(guiWorkArray, guiRoot);
		}

		for (uint32_t i = 0; i < pointerStates.size(); i++) {
			PointerState &ps = pointerStates.data[i];
			if (!ps.active) {
				pointerStates.remove(ps.id);
				i--;
			} else {
				ps.active = false;
			}
		}
	}

	void updateTransform(Systems &systems, uint32_t entityId, const EntityComponent &ec, const TransformUpdate &update) override
	{
	}

	void remove(Systems &systems, uint32_t entityId, const EntityComponent &ec) override
	{
	}

	void applyEventImmediate(Systems &systems, const sv::Event &event) override
	{
		EventContext ctx;
		ctx.immediate = ctx.begin = true;
		bool finished = applyEventImp(systems, event, ctx);
		sf_assert(finished);
	}
    
	void applyEventQueued(Systems &systems, const sf::Box<sv::Event> &event) override
	{
		queuedEvents.push(event);
	}

	void getRequestedActions(sf::Array<sf::Box<sv::Action>> &actions) override
	{
		actions.reserveGeometric(actions.size + requestedActions.size);
		for (sf::Box<sv::Action> &box : requestedActions) {
			actions.push(std::move(box));
		}
		requestedActions.clear();
	}

	void handleGui(Systems &systems, const GuiArgs &guiArgs) override
	{
		sp::Canvas &canvas = *guiArgs.canvas;
		const FrameArgs &frameArgs = systems.frameArgs;

		// In-world GUI
		for (uint32_t i = 0; i < cardTrades.size; i++) {
			CardTrade &trade = cardTrades[i];
			float t = trade.time;
			float moveOffset = 0.6f;
			float moveDuration = 0.4f;
			float fadeInDuration = 0.15f;
			float fadeOutDuration = 0.3f;

			float fade = gui::smoothEnd(sf::min(sf::min(t/fadeInDuration, (1.0f-t)/fadeOutDuration), 1.0f));

			Character *srcChr = findCharacter(trade.srcSvId);
			Character *dstChr = findCharacter(trade.dstSvId);
			Card *card = findCard(trade.cardSvId);
			if (srcChr && dstChr && card) {
				sf::Vec3 srcPos = systems.entities.entities[srcChr->entityId].transform.position;
				sf::Vec3 dstPos = systems.entities.entities[dstChr->entityId].transform.position;

				float moveT = (t - moveOffset) / moveDuration;
				sf::Vec3 pos = sf::lerp(srcPos, dstPos, gui::smoothStep(sf::clamp(moveT, 0.0f, 1.0f)));

				pos.y += sf::lerp(0.25f, 0.25f + logf(1.0f + t * 0.2f), fade);

				sf::Vec4 projected = frameArgs.mainRenderArgs.worldToClip * sf::Vec4(pos, 1.0f);
				sf::Vec2 offset = sf::Vec2(projected.x / projected.w, projected.y / projected.w);
				offset = (offset + sf::Vec2(1.0f, -1.0f)) * sf::Vec2(0.5f, -0.5f) * guiArgs.resolution;
				float height = 1.5f / projected.w * guiArgs.resolution.y;
				height = sf::clamp(height, 120.0f, 300.0f);

				float scale = sf::lerp(0.5f, 1.0f, fade);
				sf::Mat23 mat;
				mat = sf::mat2D::translate(offset) * sf::mat2D::scale(height/800.0f*scale) * sf::mat2D::translate(-250.0f, -800.0f);
				canvas.pushTint(sf::Vec4(fade));
				canvas.pushTransform(mat);
				renderCard(canvas, *card->gui);
				canvas.popTransform();
				canvas.popTint();

			}

			trade.time += frameArgs.dt * 0.75f;
			if (trade.time >= 1.0f) {
				cardTrades.removeOrdered(i);
				i--;
			}
		}

		for (uint32_t i = 0; i < damageNumbers.size; i++) {
			DamageNumber &damageNumber = damageNumbers[i];

			float t = damageNumber.time + frameArgs.dt;
			damageNumber.time = t;

			float fade = gui::smoothEnd(sf::clamp((1.0f - t) * 10.0f, 0.0f, 1.0f));

			sf::Vec3 pos = damageNumber.position + sf::Vec3(0.0f, 0.5f, 0.0f);

			sf::Vec4 projected = frameArgs.mainRenderArgs.worldToClip * sf::Vec4(pos, 1.0f);
			sf::Vec2 offset = sf::Vec2(projected.x / projected.w, (projected.y + t) / projected.w);
			offset = (offset + sf::Vec2(1.0f, -1.0f)) * sf::Vec2(0.5f, -0.5f) * guiArgs.resolution;
			float height = 0.5f / projected.w * guiArgs.resolution.y;
			height = sf::clamp(height, 20.0f, 50.0f);

			{
				sp::TextDraw draw;
				draw.string = damageNumber.text;
				draw.font = guiResources.damageFont;
				draw.transform = sf::mat2D::translate(offset) * sf::mat2D::scale(height * (1.0f / DamageNumber::BaseHeight)) * sf::mat2D::translate(damageNumber.origin);
				draw.height = DamageNumber::BaseHeight;
				draw.color = sf::Vec4(0.0f, 0.0f, 0.0f, 1.0f) * fade;
				draw.weight = 0.3f;
				canvas.drawText(draw);
				draw.color = sf::Vec4(1.0f, 1.0f, 1.0f, 1.0f) * fade;
				draw.weight = 0.48f;
				canvas.drawText(draw);
			}

			if (t >= 1.0f) {
				damageNumbers.removeOrdered(i);
				i--;
			}
		}

		{
			gui::GuiPaint paint;
			paint.canvas = &canvas;
			paint.resources = &guiResources;
			paint.crop.max = guiArgs.resolution;
			guiRoot->paint(paint);
		}

		for (DragPointer &dragPointer : dragPointers) {
			float t = sf::clamp(dragPointer.time * 7.0f, 0.0f, 1.0f);
			t = gui::smoothStep(t);

			float dragHeight = 150.0f;
			sf::Vec2 dragOffset = dragPointer.smoothPosition + sf::Vec2(-20.0f, -dragHeight - 10.0f);
			sf::Vec2 dragSize = sf::Vec2(GuiCard::canvasXByY * dragHeight, dragHeight);

			sf::Vec2 offset = sf::lerp(dragPointer.guiPointer.dropOffset, dragOffset, t);
			sf::Vec2 size = sf::lerp(dragPointer.guiPointer.dropSize, dragSize, t);

			if (dragPointer.guiPointer.dropType == guiCardSym && dragPointer.hitGuiThisFrame) {
				GuiCard *guiCard = dragPointer.guiPointer.dropData.cast<GuiCard>();

				sf::Mat23 mat;
				mat.m00 = size.x * (1.0f/500.0f);
				mat.m11 = size.y * (1.0f/800.0f);
				mat.m02 = offset.x;
				mat.m12 = offset.y;

				float tilt = sf::dot(sf::Vec2(1.0f, 0.2f), dragPointer.smoothVelocity);
				tilt = sf::copysign(logf(1.0f + sf::abs(tilt)*3.0f), tilt);
				tilt = sf::clamp(tilt * 0.2f, -1.0f, 1.0f);
				tilt *= sf::abs(tilt);

				float defaultTilt = sf::lerp(t*-0.1f, 0.0f, gui::smoothStep(sf::min(sf::abs(tilt), 1.0f)));

				mat = mat * sf::mat2D::translateY(800.0f) * sf::mat2D::rotate(defaultTilt + tilt*0.25f) * sf::mat2D::translateY(-800.0f);

				canvas.pushTransform(mat);
				renderCard(canvas, *guiCard);
				canvas.popTransform();
			}
		}
	}

};

sf::Box<GameSystem> GameSystem::create(const SystemsDesc &desc) { return sf::box<GameSystemImp>(desc); }

}
