#include "GameSystem.h"

#include "server/Pathfinding.h"
#include "server/LineRasterizer.h"

#include "client/CharacterModelSystem.h"
#include "client/BillboardSystem.h"
#include "client/AreaSystem.h"
#include "client/TapAreaSystem.h"
#include "client/EffectSystem.h"
#include "client/LightSystem.h"
#include "client/EnvLightSystem.h"
#include "client/VisFogSystem.h"
#include "client/AudioSystem.h"

#include "game/DebugDraw.h"

#include "sf/UintMap.h"
#include "sf/Reflection.h"
#include "sf/Sort.h"

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
#include "client/gui/WidgetCharacter.h"
#include "client/gui/GuiBuilder.h"
#include "client/gui/GuiResources.h"

#include "ext/sokol/sokol_gl.h"
#include "sf/Reflection.h"

namespace cl {

static const sf::Symbol guiCardSym { "GuiCard" };
static const sf::Symbol symEffect { "Effect" };
static const sf::Symbol symMelee { "Melee" };
static const sf::Symbol symStagger { "Stagger" };
static const sf::Symbol symCast { "Cast" };
static const sf::Symbol symHit { "Hit" };
static const sf::Symbol symRun { "Run" };
static const sf::Symbol symOpen { "Open" };
static const sf::Symbol symOpening { "Opening" };
static const sf::Symbol symUse { "Use" };
static const sf::Symbol symSkill { "Skill" };
static const sf::Symbol symFootstep { "Footstep" };

static const constexpr float TapCancelDistance = 0.03f;
static const constexpr float TapCancelDistanceSq = TapCancelDistance * TapCancelDistance;

static bool keyMatch(sf::Slice<const sf::Symbol> keys, sf::Slice<const sf::Symbol> locks)
{
	for (const sf::Symbol &key : keys) {
		for (const sf::Symbol &lock : locks) {
			if (key == lock) return true;
		}
	}
	return false;
}

struct Camera
{
	struct State
	{
		sf::Vec3 origin;
        float zoom = 0.0f;

		void asMatrices(sf::Vec3 &eye, sf::Mat34 &worldToView, sf::Mat44 &viewToClip, float aspect, bool unlimited)
		{
            eye = origin + sf::Vec3(0.0f, 5.0f, 1.0f) * exp2f(zoom);
            worldToView = sf::mat::look(eye, sf::Vec3(0.0f, -1.0f, sf::min(-0.6f + 0.2f * zoom, -0.3f)));
			float near = 0.1f, far = unlimited ? 1000.0f : 100.0f;
			if (sg_query_features().origin_top_left) {
				viewToClip = sf::mat::perspectiveD3D(1.0f, aspect, near, far);
			} else {
				viewToClip = sf::mat::perspectiveGL(1.0f, aspect, near, far);
			}
		}
	};

	State previous;
	State current;

	float touchMove = 0.0f;
	sf::Vec3 targetDelta;
	sf::Vec3 target;
	float targetTime = 0.0f;
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
	bool autoSelectCharacter = true;

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
		int32_t health;

		sf::Vec3 centerOffset;
		sp::SpriteRef statusActiveIcon;
		sp::SpriteRef statusInactiveIcon;

		sf::Array<uint32_t> cardIds;
		SelectedCard selectedCards[sv::NumSelectedCards];
		sv::Character sv;
	};

	struct Card
	{
		sf::Box<sv::Prefab> svPrefab;
		uint32_t prefabId;
		uint32_t svId;

		sf::Box<GuiCard> gui;
		uint32_t cooldownLeft = 0;

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

	struct TapTarget
	{
		enum Type
		{
			Unknown,
			Character,
			Door,
		};

		Type type = Unknown;
		uint32_t svId = 0;
	};

	struct PointerState
	{
		uint64_t id;
		bool active = true;
		bool hitGui = false;
		bool hitGuiThisFrame = false;
		bool hitBackground = false;
		TapTarget startTarget;
		TapTarget currentTarget;
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
		sf::Vec3 srcPosition;
		uint32_t srcSvId;
		uint32_t dstSvId;
		uint32_t cardSvId;
		float time = 0.0f;
	};

	struct CardUse
	{
		sf::Vec3 position;
		sf::Box<GuiCard> guiCard;
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

		sf::SmallStringBuf<32> text;
		sf::Vec3 position;
		sf::Vec2 origin;
		sf::Vec3 color = sf::Vec3(1.0f);
		float time = 0.0f;
	};

	struct TutorialState
	{
		bool enabled = true;
		bool hasMoved = false;
		bool cardHasAnyTargets = false;
		sf::StringBuf text;
		float requestActionCooldown = 0.0f;
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
	sf::Array<CardUse> cardUses;

	bool didHoverTile = false;
	sf::Vec2i hoveredTile;
	sf::HashMap<sf::Vec2i, float> tileHoverAmount;
	sf::HashMap<sf::Vec2i, float> tileTrailAmount;

	EventContext queuedEventContext;
	sf::Array<sf::Box<sv::Event>> queuedEvents;

	sf::Array<Projectile> projectiles;

	sv::TurnInfo turnInfo;
	bool turnChanged = false;

	uint32_t selectedCardSlot = ~0u;
	float selectedCardTime = 0.0f;

	bool showDebugMenu = false;
	bool showDebugPointers = false;
	bool simulateTouch = false;
	bool visualizeEnvLighting = false;
	bool visualizeEnvSpheres = false;
	bool debugDisableVisFog = false;
	bool debugUnlimitedCamera = false;
	EnvVisualizeSphereOpts visualizeEnvSphereOpts;

	bool castAnimDone = false;
	bool castDone = false;
	bool doorOpened = false;

	uint32_t moveWaypointIndex = ~0u;
	sf::Vec3 moveVelocity;
	float moveEndTime = 0.0f;
	float moveFoostepCooldown = 0.0f;
	uint32_t moveSoundId = ~0u;

	float autoSelectCooldown = 0.0f;

	sf::Array<DamageNumber> damageNumbers;

	TutorialState tutorial;

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
			character.health = e->character.health;
			character.sv = e->character;

			if (character.svPrefab) {
				if (auto *c = character.svPrefab->findComponent<sv::CharacterComponent>()) {
					character.centerOffset = c->centerOffset;
					character.statusActiveIcon.load(c->statusActiveIcon);
					character.statusInactiveIcon.load(c->statusInactiveIcon);
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
				uint32_t chrId = (uint32_t)(chr - characters.data);
				freeCharacterIds.push(chrId);
				svToCharacter.removeExistingPair(e->characterId, chrId);
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
			card.gui->cooldownLeft = e->card.cooldownLeft;
			card.cooldownLeft = e->card.cooldownLeft;

			svToCard.insertDuplicate(svId, cardId);

		} else if (const auto *e = event.as<sv::RemoveCardEvent>()) {

			if (Character *chr = findCharacter(e->prevOwnerId)) {
				sf::findRemoveSwap(chr->cardIds, e->cardId);
			}

			if (Card *card = findCard(e->cardId)) {
				sf::reset(*card);
				uint32_t cardId = (uint32_t)(card - cards.data);
				freeCardIds.push(cardId);
				svToCard.removeExistingPair(e->cardId, cardId);
			}

		} else if (const auto *e = event.as<sv::UseCardEvent>()) {
			if (ctx.immediate) return true;

			Character *chr = findCharacter(e->characterId);
			if (!chr) return true;

			if (ctx.begin) {
				Entity &entity = systems.entities.entities[chr->entityId];

				if (Card *card = findCard(e->cardId)) {
					CardUse &use = cardUses.push();
					use.position = entity.transform.transformPoint(chr->centerOffset);
					use.guiCard = card->gui;
				}
			}

			if (e->targetId && e->targetId != e->characterId) {
				uint32_t targetEntityId = systems.entities.svToEntity.findOne(e->targetId, ~0u);
				if (targetEntityId != ~0u) {
					Entity &targetEntity = systems.entities.entities[targetEntityId];
					Entity &entity = systems.entities.entities[chr->entityId];

					Transform transform = entity.transform;
					faceTowardsPosition(transform, targetEntity.transform.position, dt, 7.0f);
					systems.entities.updateTransform(systems, chr->entityId, transform);
				}
			}

			if (ctx.timer < 0.8f) return false;

		} else if (const auto *e = event.as<sv::CardCooldownStartEvent>()) {

			if (Card *card = findCard(e->cardId)) {
				card->gui->cooldownLeft = e->cooldown - 1;
				card->cooldownLeft = e->cooldown;
			}

		} else if (const auto *e = event.as<sv::CardCooldownTickEvent>()) {

			if (Card *card = findCard(e->cardId)) {
				if (card->cooldownLeft > 0) {
					card->cooldownLeft--;
				}
				card->gui->cooldownLeft = card->cooldownLeft;
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
				character.sv.tile = e->position;

				if (ctx.immediate || e->instant || e->waypoints.size == 0) {
					Transform transform;
					transform.position = sf::Vec3((float)e->position.x, 0.0f, (float)e->position.y);
					systems.entities.updateTransform(systems, character.entityId, transform);
				} else {
					Entity &entity = systems.entities.entities[character.entityId];
					sf::Vec3 pos = entity.transform.position;

					moveFoostepCooldown -= dt;

					if (ctx.begin) {
						moveSoundId = ~0u;
						if (auto *c = character.svPrefab->findComponent<sv::CharacterComponent>()) {
							if (c->footstepSound.loop) {
								moveSoundId = systems.audio->playAttached(systems.entities, character.entityId, c->footstepSound, sf::Vec3());
							} else {
								systems.audio->playOneShot(c->footstepSound, pos);
							}
							moveFoostepCooldown = 0.2f;
						}
					}

					if (moveFoostepCooldown <= 0.0f && moveSoundId == ~0u) {
						if (auto *c = character.svPrefab->findComponent<sv::CharacterComponent>()) {
							if (c->footstepSound.soundName) {
								sf::SmallArray<sf::Symbol, 64> frameEvents;
								systems.characterModel->queryFrameEvents(systems.entities, character.entityId, frameEvents);
								if (sf::find(frameEvents, symFootstep)) {
									systems.audio->playOneShot(c->footstepSound, pos);
									moveFoostepCooldown = 0.2f;
								}
							}
						}
					}


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

					if (end) {
						if (moveSoundId != ~0u) {
							systems.audio->removeAttached(systems.entities, moveSoundId);
							moveSoundId = ~0u;
						} else if (moveFoostepCooldown <= 0.0f) {
							if (auto *c = character.svPrefab->findComponent<sv::CharacterComponent>()) {
								systems.audio->playOneShot(c->footstepSound, pos);
								moveFoostepCooldown = 0.2f;
							}
						}
					}

					return end;
				}
			}

		} else if (const auto *e = event.as<sv::GiveCardEvent>()) {
			float delay = 0.0f;

			if (e->previousOwnerId != e->ownerId && e->previousOwnerId && e->ownerId && !ctx.immediate) {
				delay = 0.2f;
				if (ctx.begin) {
					CardTrade &trade = cardTrades.push();
					trade.srcSvId = e->previousOwnerId;
					trade.dstSvId = e->ownerId;
					trade.cardSvId = e->cardId;
				}
			} else if (e->info.fromWorld && e->ownerId && !ctx.immediate) {
				delay = 1.0f;
				if (ctx.begin) {
					sf::Vec2i tile = e->info.worldTile;
					CardTrade &trade = cardTrades.push();
					trade.srcPosition = sf::Vec3((float)tile.x, 1.0f, (float)tile.y);
					trade.dstSvId = e->ownerId;
					trade.cardSvId = e->cardId;
				}
			}

			if (Character *chr = findCharacter(e->previousOwnerId)) {
				if (uint32_t *ptr = sf::find(chr->cardIds, e->cardId)) {
					chr->cardIds.removeOrderedPtr(ptr);
				}
			}

			if (!ctx.immediate) {
				if (ctx.timer < delay) return false;
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

		} else if (const auto *e = event.as<sv::AddPropEvent>()) {
			sv::Prefab *prefab = systems.entities.findPrefab(e->prop.prefabName);

			if (prefab->findComponent<sv::DoorComponent>()){ 
				if ((e->prop.flags & sv::Prop::NoCollision) != 0) {
					uint32_t entityId;
					sf::UintFind find = systems.entities.svToEntity.findAll(e->prop.id);
					while (find.next(entityId)) {
						systems.characterModel->addTag(systems.entities, entityId, symOpen);
					}
				}
			}

		} else if (const auto *e = event.as<sv::DoorOpenEvent>()) {

			if (!ctx.immediate) {
				if (ctx.begin) {
					doorOpened = false;
					if (Character *chr = findCharacter(e->characterId)) {
						systems.characterModel->addOneShotTag(systems.entities, chr->entityId, symUse);
					}
				}

				bool doOpen = false;
				if (!doorOpened) {
					if (Character *chr = findCharacter(e->characterId)) {
						sf::SmallArray<sf::Symbol, 16> events;
						systems.characterModel->queryFrameEvents(systems.entities, chr->entityId, events);
						if (sf::find(events, symUse)) doOpen = true;
						// Failsafe
						if (ctx.timer >= 2.0f) doOpen = true;
					} else {
						doOpen = true;
					}
				}

				if (doOpen) {
					uint32_t entityId;
					sf::UintFind find = systems.entities.svToEntity.findAll(e->propId);
					while (find.next(entityId)) {
						systems.characterModel->addTag(systems.entities, entityId, symOpen);
						systems.characterModel->addOneShotTag(systems.entities, entityId, symOpening);
					}
				} else {
					return false;
				}
				doorOpened = true;
			}

		} else if (const auto *e = event.as<sv::TurnUpdateEvent>()) {

			if (e->turnInfo.startTurn) {
				moveSelectTime = 0.0f;
				autoSelectCooldown = 0.0f;
				selectedCardSlot = ~0u;
				tutorial.hasMoved = false;
			} else {
				tutorial.hasMoved = true;
			}

			if (!ctx.immediate && ctx.begin && e->turnInfo.startTurn) {
				if (Character *chr = findCharacter(turnInfo.characterId)) {
					if (!chr->sv.enemy && chr->sv.playerClientId == systems.frameArgs.localClientId) {
						sf::Vec3 pos = sf::Vec3((float)chr->tile.x, 0.0f, (float)chr->tile.y);
						sf::Vec4 projected = systems.frameArgs.mainRenderArgs.worldToClip * sf::Vec4(pos, 1.0f);
						sf::Vec2 offset = sf::Vec2(projected.x / projected.w, projected.y / projected.w);
						sf::Vec2 screenMin = sf::Vec2(-0.7f, -0.8f);
						sf::Vec2 screenMax = sf::Vec2(0.7f, 0.8f);
						sf::Vec2 clamped = sf::clamp(offset, screenMin, screenMax);
						if (clamped != offset) {
							sf::Vec2 clampMin = sf::Vec2(-0.4f, -0.4f);
							sf::Vec2 clampMax = sf::Vec2(0.4f, 0.4f);
							clamped = sf::clamp(offset, clampMin, clampMax);
							sf::Mat44 clipToWorld = sf::inverse(systems.frameArgs.mainRenderArgs.worldToClip);
							sf::Vec4 rayBegin = clipToWorld * sf::Vec4(clamped.x, clamped.y, 0.0f, 1.0f);
							sf::Vec4 rayEnd = clipToWorld * sf::Vec4(clamped.x, clamped.y, 1.0f, 1.0f);
							sf::Ray ray;
							ray.origin = sf::Vec3(rayBegin.v) / rayBegin.w;
							ray.direction = sf::normalize(sf::Vec3(rayEnd.v) / rayEnd.w - ray.origin);
							sf::Vec3 target = intersectHorizontalPlane(0.0f, ray);
							sf::Vec3 delta = camera.current.origin - target;
							camera.target = pos + delta;
							camera.targetTime = 3.0f;
						}
					}
				}
			}

			if (e->turnInfo.startTurn && !ctx.immediate && !e->immediate) {
				if (ctx.timer > 0.5f) turnInfo = e->turnInfo;
				if (ctx.timer < 0.75f) return false;
			}

			turnChanged = true;
			turnInfo = e->turnInfo;

		} else if (const auto *e = event.as<sv::VisibleUpdateEvent>()) {

			systems.visFog->updateVisibility(*e, ctx.immediate);

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
				if (sf::find(events, symHit)) {
					if (Character *targetChr = findCharacter(e->meleeInfo.targetId)) {
						Entity &targetEntity = systems.entities.entities[targetChr->entityId];
						sf::Vec3 pos = targetEntity.transform.transformPoint(targetChr->centerOffset);
						if (sv::Prefab *prefab = systems.entities.findPrefab(e->meleeInfo.cardName)) {
							if (auto *c = prefab->findComponent<sv::CardMeleeComponent>()) {
								systems.audio->playOneShot(c->hitSound, pos);
							}
						}
					}
					return true;
				}
			}

			// Failsafe
			if (ctx.timer < 2.0f) return false;
		} else if (const auto *e = event.as<sv::DamageEvent>()) {
			if (ctx.immediate) {
				if (Character *chr = findCharacter(e->damageInfo.targetId)) {
					chr->health -= e->finalDamage;
				}
				return true;
			}

			if (Character *chr = findCharacter(e->damageInfo.targetId)) {
				Entity &entity = systems.entities.entities[chr->entityId];

				if (ctx.begin) {
					systems.characterModel->addOneShotTag(systems.entities, chr->entityId, symStagger);

					DamageNumber &damageNumber = damageNumbers.push();
					damageNumber.text.format("%u", e->finalDamage);
					if (e->damageIncrease != 0) {
						if (e->damageDecrease != 0) {
							damageNumber.text.format(" (+%d -%d)", e->damageIncrease, e->damageDecrease);
						} else {
							damageNumber.text.format(" (+%d)", e->damageIncrease);
						}
					} else if (e->damageDecrease) {
						damageNumber.text.format(" (-%d)", e->damageDecrease);
					}
					damageNumber.position = entity.transform.position + chr->centerOffset;
					damageNumber.origin = guiResources.damageFont->measureText(damageNumber.text, DamageNumber::BaseHeight) * sf::Vec2(-0.5f, 0.1f);
					chr->health -= (int32_t)e->finalDamage;

					if (auto *c = systems.entities.globalPrefabs.effectsComponent) {
						if (e->damageInfo.melee && c->meleeHitEffect) {
							sf::Vec3 targetPos = entity.transform.transformPoint(chr->centerOffset);
							systems.effect->spawnOneShotEffect(systems, c->meleeHitEffect, targetPos);
						}
					}

					if (auto *c = chr->svPrefab->findComponent<sv::CharacterComponent>()) {
						sf::Vec3 targetPos = entity.transform.transformPoint(chr->centerOffset);
						systems.audio->playOneShot(c->damageSound, targetPos, 0.15f);
					}
				}
			}


			if (ctx.timer < 0.5f) return false;

		} else if (const auto *e = event.as<sv::HealEvent>()) {
			if (ctx.immediate) {
				if (Character *chr = findCharacter(e->healInfo.targetId)) {
					chr->health += e->finalHeal;
				}
				return true;
			}

			if (Character *chr = findCharacter(e->healInfo.targetId)) {
				Entity &entity = systems.entities.entities[chr->entityId];

				if (ctx.begin) {
					DamageNumber &damageNumber = damageNumbers.push();
					damageNumber.text.format("+%u", e->finalHeal);
					if (e->healIncrease != 0) {
						if (e->healDecrease != 0) {
							damageNumber.text.format(" (+%d -%d)", e->healIncrease, e->healDecrease);
						} else {
							damageNumber.text.format(" (+%d)", e->healIncrease);
						}
					} else if (e->healDecrease) {
						damageNumber.text.format(" (-%d)", e->healDecrease);
					}
					damageNumber.position = entity.transform.position + chr->centerOffset;
					damageNumber.origin = guiResources.damageFont->measureText(damageNumber.text, DamageNumber::BaseHeight) * sf::Vec2(-0.5f, 0.1f);
					damageNumber.color = sf::Vec3(0.6f, 1.0f, 0.6f);
					chr->health += (int32_t)e->finalHeal;
				}
			}

			if (ctx.timer < 0.5f) return false;

		} else if (const auto *e = event.as<sv::TweakCharacterEvent>()) {

			if (Character *chr = findCharacter(e->character.id)) {
				chr->sv.enemy = e->character.enemy;
				chr->sv.originalEnemy = e->character.originalEnemy;
				chr->sv.dropCards = e->character.dropCards;
			}

		} else if (const auto *e = event.as<sv::SelectCharacterEvent>()) {

			if (Character *chr = findCharacter(e->characterId)) {
				chr->sv.playerClientId = e->clientId;
			}

		} else if (const auto *e = event.as<sv::ChangeTeamEvent>()) {

			if (Character *chr = findCharacter(e->characterId)) {
				chr->sv.enemy = e->enemy;

				if (e->playerClientId) {
					chr->sv.playerClientId = e->playerClientId;
				}
			}

		} else if (const auto *e = event.as<sv::CastSpellEvent>()) {
			if (ctx.immediate) return true;

			Character *chr = findCharacter(e->spellInfo.casterId);
			if (!chr) return true;
			Entity &casterEntity = systems.entities.entities[chr->entityId];

			const sf::Symbol *pAnim = &symCast;
			if (e->useItemAnimation) pAnim = &symUse;
			if (e->useSkillAnimation) pAnim = &symSkill;

			if (ctx.begin) {
				castDone = false;
				if (e->spellInfo.manualCast) {
					systems.characterModel->addOneShotTag(systems.entities, chr->entityId, *pAnim);
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
				if (sf::find(events, *pAnim)) castAnimDone = true;

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
				if (sv::Prefab *spellPrefab = systems.entities.findPrefab(e->spellInfo.spellName)) {
					if (auto *spellComp = spellPrefab->findComponent<sv::SpellComponent>()) {
						if (spellComp->hitEffect) {
							sf::Vec3 targetPos = casterEntity.transform.position;
							if (Character *targetChr = findCharacter(e->spellInfo.targetId)) {
								Entity &targetEntity = systems.entities.entities[targetChr->entityId];
								targetPos = targetEntity.transform.transformPoint(targetChr->centerOffset);
							} else {
								uint32_t targetEntityId = systems.entities.svToEntity.findOne(e->spellInfo.targetId, ~0u);
								if (targetEntityId != ~0u) {
									Entity &targetEntity = systems.entities.entities[targetEntityId];
									targetPos = targetEntity.transform.position;
								}
							}

							systems.effect->spawnOneShotEffect(systems, spellComp->hitEffect, targetPos);
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
		inputArgs.resolution = sf::Vec2(frameArgs.windowResolution);
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

		sf::Vec2i renderRes = frameArgs.mainRenderArgs.renderResolution;
		float aspect = (float)renderRes.x / (float)renderRes.y;

		static const float cameraExp = 0.02f;
		static const float cameraLinear = 0.00005f;
		static const float decayExp = 0.008f;
		static const float decayLinear = 0.00005f;
		static const float decayTouchExp = 0.004f;
		static const float decayTouchLinear = 0.00005f;

		sf::Vec2 cameraMove;

		if (camera.targetTime <= 0.0f) {
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
			camera.current.asMatrices(eye, worldToView, viewToClip, aspect, debugUnlimitedCamera);
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
				camera.targetTime = 0.0f;
            } else if (camera.targetTime > 0.0f) {
				camera.targetDelta = (camera.target - camera.current.origin) * 0.5f;
				float dist = 1.0f / sf::min(1.0f, 0.01f + sf::length(camera.targetDelta)*0.1f);
				camera.targetTime -= cameraDt * dist;
			}

			sf::Vec3 delta = camera.targetDelta;
			float deltaLen = sf::length(delta);

			camera.current.origin += camera.velocity * cameraDt;
            
			camera.zoomDelta = sf::clamp(camera.zoomDelta, -10.0f, 10.0f);
            camera.current.zoom += camera.zoomDelta * 0.01f;

			if (!debugUnlimitedCamera) {
				camera.current.zoom = sf::clamp(camera.current.zoom, -1.5f, 1.5f);
			}

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

		Camera::State state = Camera::lerp(camera.previous, camera.current, camera.timeDelta / cameraDt);

		sf::Vec3 eye;
		sf::Mat34 worldToView;
		sf::Mat44 viewToClip;
		state.asMatrices(eye, worldToView, viewToClip, aspect, debugUnlimitedCamera);
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

	sf::Box<void> preloadCard(const sv::Prefab &svPrefab) override
	{
		auto box = sf::box<GuiCard>();
		box->init(svPrefab, 0);
		return box;
	}

	void updateDebugMenu(Systems &systems)
	{
		if (showDebugMenu) {
			ImGui::SetNextWindowSize(ImVec2(200.0f, 200.0f), ImGuiCond_Appearing);
			if (ImGui::Begin("Game Debug", &showDebugMenu)) {
				static bool iblEnabled = true;
				if (ImGui::Checkbox("IBL enabled", &iblEnabled)) {
					systems.light->setIblEnabled(iblEnabled);
					systems.envLight->setIblEnabled(iblEnabled);
				}
				ImGui::Checkbox("Visualize env lighting", &visualizeEnvLighting);
				ImGui::Checkbox("Visualize env spheres", &visualizeEnvSpheres);
				if (visualizeEnvSpheres) {
					ImGui::Indent();
					ImGui::SliderFloat("Radius", &visualizeEnvSphereOpts.radius, 0.01f, 0.3f);
					ImGui::SliderFloat("Specular", &visualizeEnvSphereOpts.specular, 0.0f, 1.0f);
					ImGui::Unindent();
				}
				ImGui::Checkbox("Disable VisFog", &debugDisableVisFog);
				ImGui::Checkbox("Unlimited camera zoom", &debugUnlimitedCamera);
				ImGui::Checkbox("Simulate touch", &simulateTouch);
				if (ImGui::Button("Pointers")) showDebugPointers = true;
			}
			ImGui::End();
		}

		if (debugDisableVisFog) {
			systems.visFog->disableForFrame();
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

#if 0
		if (visualizeEnvLighting) {
			float aspect = (float)systems.frameArgs.resolution.x / (float)systems.frameArgs.resolution.y;
			sgl_matrix_mode_modelview();
			sgl_load_identity();
			sgl_matrix_mode_projection();
			sgl_load_identity();
			sgl_matrix_mode_texture();
			sgl_load_identity();
			sgl_enable_texture();
			sgl_texture(systems.envLight->getDebugLightingImage());
			sgl_begin_quads();
			sgl_c3f(1.0f, 1.0f, 1.0f);
			sgl_v2f_t2f(0.0f, 0.0f, 0.0f, 0.0f);
			sgl_v2f_t2f(0.5f, 0.0f, 1.0f, 0.0f);
			sgl_v2f_t2f(0.5f, -0.5f * aspect, 1.0f, 1.0f);
			sgl_v2f_t2f(0.0f, -0.5f * aspect, 0.0f, 1.0f);
			sgl_end();
			sgl_disable_texture();
		}
#endif
	}

	void updateTutorial(const sv::ServerState &svState, Systems &systems, const FrameArgs &frameArgs)
	{
		tutorial.text.clear();

		if (requestedActions.size > 0) {
			tutorial.requestActionCooldown = 0.5f;
		}

		if (tutorial.requestActionCooldown > 0.0f) {
			tutorial.requestActionCooldown -= frameArgs.dt;
			return;
		}

		if (queuedEvents.size > 0) {
			return;
		}

		if (selectedCharacterId == 0) {
			tutorial.text = "Wait for your turn!";
			if (Character *chr = findCharacter(turnInfo.characterId)) {
				if (!chr->sv.enemy) {
					tutorial.text = "Wait for your turn!\n\nAnother player is controlling this character! Use the [nb][b][[Select character][/][/] button to take control of this character.";
				}
			}
			return;
		}

		bool enemyVisible = false;

		bool suggestedCardIsOffensive = false;
		uint32_t suggestedBestCooldown = 0;
		uint32_t suggestedCardSlot = ~0u;
		Character *chr = findCharacter(selectedCharacterId);
		if (!chr) return;

		if (chr) {
			for (const sv::Character &enemyChr : svState.characters) {
				if (!enemyChr.enemy) continue;
				sf::Vec2i delta = enemyChr.tile - chr->tile;
				if (delta.x < 0) delta.x = -delta.x;
				if (delta.y < 0) delta.y = -delta.y;
				if (sf::max(delta.x, delta.y) > 20) continue;

				bool hitWall = false;
				sv::ConservativeLineRasterizer raster(chr->tile, enemyChr.tile);
				for (;;) {
					sf::Vec2i tile = raster.next();
					if (tile == enemyChr.tile) break;
					if (tile == chr->tile) continue;

					uint32_t id;
					sf::UintFind find = svState.getTileEntities(tile);
					while (find.next(id)) {
						sv::IdType type = sv::getIdType(id);
						if (type == sv::IdType::Prop) {
							const sv::Prop *prop = svState.props.find(id);
							if (prop && (prop->flags & sv::Prop::Wall) != 0) hitWall = true;
						}
					}
				}

				for (uint32_t slotI = 0; slotI < sv::NumSelectedCards; slotI++) {
					uint32_t cardId = chr->selectedCards[slotI].currentSvId;
					Card *card = findCard(cardId);
					if (!card) continue;
					if (card->cooldownLeft > 0) continue;

					uint32_t cooldown = 0;
					if (auto *c = card->svPrefab->findComponent<sv::CardComponent>()) {
						cooldown = c->cooldown;
					}

					if (svState.canTarget(chr->svId, enemyChr.id, card->svPrefab->name)) {
						if (slotI < suggestedCardSlot || cooldown > suggestedBestCooldown) {
							suggestedBestCooldown = cooldown;
							suggestedCardSlot = slotI;
							suggestedCardIsOffensive = true;
							enemyVisible = true;
						}
					}
				}

				if (!hitWall) {
					enemyVisible = true;
					break;
				}
			}
		}

		if (!suggestedCardIsOffensive) {
			for (uint32_t slotI = 0; slotI < sv::NumSelectedCards; slotI++) {
				uint32_t cardId = chr->selectedCards[slotI].currentSvId;
				Card *card = findCard(cardId);
				if (!card) continue;
				if (card->cooldownLeft > 0) continue;

				uint32_t cooldown = 0;
				if (auto *c = card->svPrefab->findComponent<sv::CardComponent>()) {
					cooldown = c->cooldown;
				}

				if (svState.canTarget(chr->svId, chr->svId, card->svPrefab->name)) {
					if (slotI < suggestedCardSlot || cooldown > suggestedBestCooldown) {
						suggestedBestCooldown = cooldown;
						suggestedCardSlot = slotI;
					}
				}
			}
		}

		if (!enemyVisible) {
			if (!tutorial.hasMoved) {
				tutorial.text = "Click a green square to move.";
			} else {
				tutorial.text = "Use the [nb][b][[End turn][/][/] button to finish your turn";
				if (turnInfo.movementLeft > 0) {
					tutorial.text.append(" or click another square to move further.");
				} else {
					tutorial.text.append(".");
				}
			}
			return;
		} else {
			if (selectedCardSlot == ~0u) {
				if (suggestedCardIsOffensive) {
					if (Card *card = findCard(chr->selectedCards[suggestedCardSlot].currentSvId)) {
						if (auto *cardComp = card->svPrefab->findComponent<sv::CardComponent>()) {
							if (turnInfo.movementLeft > 0) {
								tutorial.text.append("Click a green square to move or click");
							} else {
								tutorial.text.append("Click");
							}
							tutorial.text.append(" the card [nb][b]", cardComp->name, "[/][/] to select it.");
						}
					}
				} else {
					if (suggestedCardSlot != ~0u) {
						if (Card *card = findCard(chr->selectedCards[suggestedCardSlot].currentSvId)) {
							if (auto *cardComp = card->svPrefab->findComponent<sv::CardComponent>()) {
								if (turnInfo.movementLeft > 0) {
									tutorial.text.append("Click a green square to move closer to the enemies or click");
								} else {
									tutorial.text.append("Click");
								}
								tutorial.text.append(" the card [nb][b]", cardComp->name, "[/][/] to select it.");
							}
						}
					} else {
						if (turnInfo.movementLeft > 0) {
							tutorial.text.append("Move closer to the enemies or use");
						} else {
							tutorial.text.append("Use");
						}
						tutorial.text.append(" the [nb][b][[End turn][/][/] button to finish your turn");
					}
				}
			} else {
				if (tutorial.cardHasAnyTargets) {
					if (Card *card = findCard(chr->selectedCards[selectedCardSlot].currentSvId)) {
						if (auto *cardComp = card->svPrefab->findComponent<sv::CardComponent>()) {
							tutorial.text.append("Click on a highlighted target to use the card [nb][b]", cardComp->name, "[/][/]!");
						}
					}
				} else {
					if (Card *card = findCard(chr->selectedCards[selectedCardSlot].currentSvId)) {
						if (auto *cardComp = card->svPrefab->findComponent<sv::CardComponent>()) {
							tutorial.text.append("The selected card [nb][b]", cardComp->name, "[/][/] can't reach any targets! Select another card or click it again to deselect it.");
						}
					}
				}
			}

		}
	}

	void updateGui(const sv::ServerState &svState, Systems &systems, const FrameArgs &frameArgs)
	{
		gui::GuiBuilder &b = guiBuilder;
		b.init(guiRoot);

		Character *chr = findCharacter(selectedCharacterId);
		sv::CharacterComponent *chrComp = chr ? chr->svPrefab->findComponent<sv::CharacterComponent>() : NULL;

		{
			auto ll = b.push<gui::WidgetLinearLayout>();

			ll->boxOffset = sf::Vec2(20.0f, 20.0f);
			ll->boxExtent = sf::Vec2(frameArgs.guiResolution.x, gui::Inf);
			ll->direction = gui::DirY;
			ll->padding = 10.0f;
			ll->anchor = 0.0f;

			for (Character &playerChr : characters) {
				if (!playerChr.svId) continue;
				if (playerChr.sv.enemy) continue;

				auto ch = b.push<gui::WidgetCharacter>(playerChr.svId);
				ch->boxExtent = sf::Vec2(200.0f, 50.0f);
				ch->currentHealth = playerChr.health;
				ch->maxHealth = playerChr.sv.maxHealth;
				ch->turnChanged = turnChanged;
				ch->turnActive = turnInfo.characterId == playerChr.svId;

				if (playerChr.svId == turnInfo.characterId) {
					ch->icon = playerChr.statusActiveIcon;
				} else {
					ch->icon = playerChr.statusInactiveIcon;
				}

				if (ch->clicked) {
					Entity &entity = systems.entities.entities[playerChr.entityId];
					sf::Vec3 pos = entity.transform.position;
					pos.y = 0.0f;
					pos.z += 2.0f;
					camera.target = pos;
					camera.targetTime = 3.0f;
				}

				b.pop();
			}

			if (tutorial.enabled) {
				auto ll1 = b.push<gui::WidgetLinearLayout>(10);
				ll1->direction = gui::DirY;
				ll1->boxExtent = sf::Vec2(200.0f, gui::Inf);
				ll1->anchor = 0.0f;
				ll1->marginBefore = 10.0f;
				ll1->padding = 10.0f;

				{
					auto rt = b.push<gui::WidgetRichText>(30);
					rt->text = "[b]Camera controls[/b]\n\nMove: WASD / arrow keys / middle click\n\nZoom: Scroll wheel\n\nFocus: Click character icon";
					rt->style = &guiResources.tutorialRichStyle;
					rt->fontHeight = 15.0f;

					b.pop(); // RichText
				}

				if (tutorial.text.size > 0) {
					auto rt = b.push<gui::WidgetRichText>(40);
					rt->text.clear();
					rt->text.append("[b]Hint[/b]\n\n");
					rt->text.append(tutorial.text);
					rt->style = &guiResources.tutorialRichStyle;
					rt->fontHeight = 15.0f;

					b.pop(); // RichText
				}

#if 0
				{
					auto bt = b.push<gui::WidgetButton>(100);
					if (bt->created) {
						bt->text = sf::Symbol("Hide tutorial");
						bt->font = guiResources.buttonFont;
						bt->sprite = guiResources.buttonSprite;
						bt->fontHeight = 15.0f;
					}

					bt->boxExtent = sf::Vec2(80.0f, 20.0f);

					if (bt->pressed) {
						tutorial.enabled = false;
					}

					b.pop(); // Button
				}
#endif

				b.pop(); // LinearLayout
			}

			b.pop(); // LinearLayout
		}

		if (Character *turnChr = findCharacter(turnInfo.characterId)) {
			if (!turnChr->sv.enemy && turnChr->sv.playerClientId != svState.localClientId) {

				auto bt = b.push<gui::WidgetButton>(1);
				if (bt->created) {
					bt->text = sf::Symbol("Select character");
					bt->font = guiResources.buttonFont;
					bt->sprite = guiResources.buttonSprite;
					bt->fontHeight = 20.0f;
				}
				float width = 120.0f;
				bt->boxOffset = sf::Vec2(frameArgs.guiResolution.x * 0.5f - width * 0.5f, 20.0f);
				bt->boxExtent = sf::Vec2(width, 30.0f);

				if (bt->pressed) {
					auto action = sf::box<sv::SelectCharacterAction>();
					action->characterId = turnChr->svId;
					action->clientId = svState.localClientId;
					requestedActions.push(std::move(action));
				}

				b.pop();
			}
		}

		if (chr && chrComp) {

			if (turnInfo.characterId == selectedCharacterId && !frameArgs.editorOpen) {
				auto bt = b.push<gui::WidgetButton>(2);
				if (bt->created) {
					bt->text = sf::Symbol("End Turn");
					bt->font = guiResources.buttonFont;
					bt->sprite = guiResources.buttonSprite;
					bt->fontHeight = 40.0f;
				}
				bt->boxOffset = sf::Vec2(frameArgs.guiResolution.x - 140.0f - 20.0f, 20.0f);
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

						if (chr->sv.originalEnemy) {
							sl->allowDrag = false;
						}

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

	void update(const sv::ServerState &svState, Systems &systems, const FrameArgs &frameArgs) override
	{
		updateDebugMenu(systems);
		updateGui(svState, systems, frameArgs);

		#if 0
		{
			ImGui::Begin("Event queue");
			sf::Type *eventType = sf::typeOf<sv::Event>();
			for (const sv::Event *event :queuedEvents) {
				ImGui::Text("%s", eventType->getPolymorphTypeByValue(event->type)->name.data);
			}
			ImGui::End();
		}
		#endif

		turnChanged = false;
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

		// Character auto-selection
		if (autoSelectCharacter) {
			selectedCharacterId = 0;
			if (Character *chr = findCharacter(turnInfo.characterId)) {
				const sv::Character *svChr = svState.characters.find(chr->svId);
				if (svChr && !svChr->enemy) {
					if (svChr->playerClientId == svState.localClientId) {
						selectedCharacterId = chr->svId;
					} else if (svChr->playerClientId == 0) {
						if (autoSelectCooldown <= 0.0f) {
							auto action = sf::box<sv::SelectCharacterAction>();
							action->characterId = chr->svId;
							action->clientId = svState.localClientId;
							requestedActions.push(std::move(action));
						} else {
							autoSelectCooldown -= frameArgs.dt;
						}
					}
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
				if (Card *card = findCard(chr->selectedCards[selectedCardSlot].currentSvId)) {
					if (card->cooldownLeft == 0) {
						valid = true;
					}
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

		sf::SmallArray<uint32_t, 4> openableDoorSvIds;

		tutorial.cardHasAnyTargets = false;
		if (selectedCardSlot != ~0u) {
			Character *caster = findCharacter(selectedCharacterId);
			Card *card = findCard(caster->selectedCards[selectedCardSlot].currentSvId);
			if (caster && card) {
				for (uint32_t characterModelId : systems.visibleAreas.get(AreaGroup::CharacterModel)) {
					uint32_t entityId = systems.characterModel->getEntityId(characterModelId);
					Entity &entity = systems.entities.entities[entityId];
					if (!entity.svId) continue;

					if (svState.canTarget(caster->svId, entity.svId, card->svPrefab->name)) {
						HighlightDesc desc = { };
						desc.color = sf::Vec3(1.0f);
						systems.characterModel->addFrameHighlightToModel(characterModelId, desc, frameArgs);
						tutorial.cardHasAnyTargets = true;
					}
				}

				// Check for doors to open with a key card
				if (auto *keyComp = card->svPrefab->findComponent<sv::CardKeyComponent>()) {
					static const sf::Vec2i dirs[] = { { -1, 0 }, { +1, 0 }, { 0, -1 }, { 0, +1 } };
					for (const sf::Vec2i &dir : dirs) {
						sf::Vec2i tile = caster->tile + dir;
						uint32_t svId;
						sf::UintFind find = svState.getTileEntities(tile);
						while (find.next(svId)) {
							if (sv::getIdType(svId) != sv::IdType::Prop) continue;
							const sv::Prop *prop = svState.props.find(svId);
							if (!prop) continue;
							if ((prop->flags & sv::Prop::NoCollision) != 0) continue;
							const sv::Prefab *doorPrefab = svState.prefabs.find(prop->prefabName);
							if (!doorPrefab) continue;
							const sv::DoorComponent *doorComp = doorPrefab->findComponent<sv::DoorComponent>();
							if (!doorComp) continue;
							if (!keyMatch(keyComp->keyNames, doorComp->keyNames)) continue;

							HighlightDesc desc = { };
							desc.color = sf::Vec3(1.0f);

							openableDoorSvIds.push(svId);

							uint32_t entityId;
							sf::UintFind entityFind = systems.entities.svToEntity.findAll(svId);
							while (entityFind.next(entityId)) {
								systems.characterModel->addFrameHighlightToEntity(systems.entities, entityId, desc, frameArgs);
							}
						}
					}
				}
			}
		}

		for (Pointer &pointer : input.pointers) {
			PointerState *pointerState = pointerStates.find(pointer.id);
			if (!pointerState) continue;

			sf::SmallArray<HoveredTapArea, 64> hoveredTapAreas;

			systems.tapArea->getHoveredTapAreas(hoveredTapAreas, systems.area, pointer.current.worldRay);
			sf::sort(hoveredTapAreas);

			TapTarget target;

			for (HoveredTapArea &hover : hoveredTapAreas) {
				Entity &entity = systems.entities.entities[hover.entityId];
				Prefab &prefab = systems.entities.prefabs[entity.prefabId];

				// Always select characters
				if (findCharacter(entity.svId)) {
					target = { TapTarget::Character, entity.svId };
					break;
				}

				// Select closed doors
				if (prefab.svPrefab->findComponent<sv::DoorComponent>()) {
					if (const sv::Prop *prop = svState.props.find(entity.svId)) {
						if ((prop->flags & sv::Prop::NoCollision) == 0) {
							target = { TapTarget::Door, entity.svId };
						}
					}
				}

			}

			if (pointer.action == Pointer::Down) {
				pointerState->startTarget = target;
			}
			pointerState->currentTarget = target;

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

				Character *chr = nullptr;
				Character *caster = nullptr;
				Card *card = nullptr;
				bool canTarget = true;

				if (pointerState->startTarget.type == TapTarget::Character) {
					chr = findCharacter(pointerState->startTarget.svId);
				}

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
							if (chr->svId != selectedCharacterId && !autoSelectCharacter) {
								selectedCardSlot = ~0u;
								selectedCharacterId = chr->svId;
								selectedCharacterTime = 0.0f;
								moveSelectTime = 0.0f;
							}
						}

					} else if (didHover) {
						bool showHover = false;

						if ((chr->svId != selectedCharacterId && !autoSelectCharacter) || selectedCardSlot != ~0u) {
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
				} else if (pointerState->startTarget.type == TapTarget::Door) {

					uint32_t doorId = pointerState->startTarget.svId;
					if (didClick) {
						if (sf::find(openableDoorSvIds, doorId)) {
							auto action = sf::box<sv::OpenDoorAction>();

							action->characterId = selectedCharacterId;
							action->doorId = pointerState->startTarget.svId;
							if (selectedCardSlot != ~0u) {
								action->cardId = caster->selectedCards[selectedCardSlot].currentSvId;
							} else {
								action->cardId = 0;
							}

							requestedActions.push(std::move(action));
							selectedCardSlot = ~0u;
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
							amount = sf::min(1.0f, amount + frameArgs.dt * 10.0f);

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
			if (!didHoverTile || hoveredTile != pair.key) {
				pair.val -= frameArgs.dt * 10.0f;
			}
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
				if (pointerState->currentTarget.type == TapTarget::Character) {
					if (Character *chr = findCharacter(pointerState->currentTarget.svId)) {
						if (!chr->sv.originalEnemy) {
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

		if (tutorial.enabled) {
			updateTutorial(svState, systems, frameArgs);
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
			if (srcChr) {
				trade.srcPosition = systems.entities.entities[srcChr->entityId].transform.position;
			}

			Character *dstChr = findCharacter(trade.dstSvId);
			Card *card = findCard(trade.cardSvId);
			if (dstChr && card) {
				sf::Vec3 srcPos = trade.srcPosition;
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
				GuiCard::RenderOpts opts = { };
				opts.showCooldown = false;
				renderCard(canvas, *card->gui, opts);
				canvas.popTransform();
				canvas.popTint();

			}

			trade.time += frameArgs.dt * 0.75f;
			if (trade.time >= 1.0f) {
				cardTrades.removeOrdered(i);
				i--;
			}
		}

		for (uint32_t i = 0; i < cardUses.size; i++) {
			CardUse &use = cardUses[i];
			float t = use.time;
			float fadeInDuration = 0.15f;
			float fadeOutDuration = 0.3f;

			float fade = gui::smoothEnd(sf::min(sf::min(t/fadeInDuration, (1.0f-t)/fadeOutDuration), 1.0f));

			sf::Vec3 pos = use.position;
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
			GuiCard::RenderOpts opts = { };
			opts.showCooldown = false;
			renderCard(canvas, *use.guiCard, opts);
			canvas.popTransform();
			canvas.popTint();

			use.time += frameArgs.dt * 0.75f;
			if (use.time >= 1.0f) {
				cardUses.removeOrdered(i);
				i--;
			}
		}

		for (uint32_t i = 0; i < damageNumbers.size; i++) {
			DamageNumber &damageNumber = damageNumbers[i];

			float t = damageNumber.time + frameArgs.dt * 0.7f;
			damageNumber.time = t;

			float fade = gui::smoothEnd(sf::clamp((1.0f - t) * 10.0f, 0.0f, 1.0f));

			sf::Vec3 pos = damageNumber.position + sf::Vec3(0.0f, 0.5f, 0.0f);

			sf::Vec4 projected = frameArgs.mainRenderArgs.worldToClip * sf::Vec4(pos, 1.0f);
			sf::Vec2 offset = sf::Vec2(projected.x / projected.w, (projected.y + gui::smoothEnd(t*0.5f)*1.8f) / projected.w);
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
				draw.color = sf::Vec4(damageNumber.color, 1.0f) * fade;
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
				GuiCard::RenderOpts opts = { };
				opts.showCooldown = false;
				renderCard(canvas, *guiCard, opts);
				canvas.popTransform();
			}
		}
	}

	bool getVisualizeGI() const override
	{
		return visualizeEnvLighting;
	}

	bool getVisualizeGISpheres(EnvVisualizeSphereOpts &opts) const override
	{
		if (visualizeEnvSpheres) {
			opts = visualizeEnvSphereOpts;
			return true;
		} else {
			return false;
		}
	}


};

sf::Box<GameSystem> GameSystem::create(const SystemsDesc &desc) { return sf::box<GameSystemImp>(desc); }

}
