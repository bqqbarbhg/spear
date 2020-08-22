#include "GameSystem.h"

#include "server/Pathfinding.h"

#include "client/CharacterModelSystem.h"
#include "client/BillboardSystem.h"
#include "client/AreaSystem.h"
#include "client/TapAreaSystem.h"

#include "game/DebugDraw.h"

#include "sf/UintMap.h"
#include "sf/Reflection.h"

#include "client/GuiCard.h"

#include "sp/Renderer.h"

#include "ext/sokol/sokol_app.h"
#include "ext/imgui/imgui.h"

#include "client/InputState.h"

namespace cl {

static const constexpr float TapCancelDistance = 0.03f;
static const constexpr float TapCancelDistanceSq = TapCancelDistance * TapCancelDistance;

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

struct GameSystemImp final : GameSystem
{
	uint32_t selectedCharacterId = 0;

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

	struct TapTarget
	{
		enum Action
		{
			Hover,
			Start,
			Finish,
		};

		Action action = Action::Start;
		uint32_t svId = 0;
		sf::Vec2 tile;
		sf::Vec2i tileInt;
	};

	static sf::Vec3 intersectHorizontalPlane(float height, const sf::Ray &ray)
	{
		float t = (ray.origin.y - height) / - ray.direction.y;
		sf::Vec3 v = ray.origin + ray.direction * t;
		v.y = height;
		return v;
	}

	sf::Array<Character> characters;
	sf::Array<uint32_t> freeCharacterIds;

	sf::Array<Card> cards;
	sf::Array<uint32_t> freeCardIds;

	sf::UintMap svToCharacter;
	sf::UintMap entityToCharacter;

	sf::UintMap svToCard;

	InputState input;
	sf::HashMap<uint64_t, TapTarget> tapTargets;

	sf::Array<sf::Box<sv::Action>> requestedActions;

	Camera camera;

	sv::ReachableSet moveSet;
	sf::Array<sf::Vec2i> moveWaypoints;

	GuiCardResources guiCardRes;

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

	GameSystemImp(const SystemsDesc &desc)
	{
		camera.previous.origin = camera.current.origin = sf::Vec3(desc.persist.camera.x, 0.0f, desc.persist.camera.y);
		camera.previous.zoom = camera.current.zoom = desc.persist.zoom;
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
		inputArgs.simulateTouch = false;

		input.update(inputArgs);

		for (const sapp_event &e : frameArgs.events) {

			if (e.type == SAPP_EVENTTYPE_MOUSE_SCROLL) {

				if (!input.mouseBlocked) {
					camera.zoomDelta += e.scroll_y * -0.1f;
				}

			}
		}

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

			for (Pointer &p : input.pointers) {
                if ((p.button == Pointer::MouseMiddle || p.button == Pointer::Touch)
                    && p.action == Pointer::Hold) {

					p.current.worldRay = pointerToWorld(clipToWorld, p.current.pos);

					if (p.button == Pointer::Touch) {
						camera.touchMove += cameraDt * 4.0f;
					} else {
						camera.touchMove -= cameraDt * 4.0f;
					}
					camera.touchMove = sf::clamp(camera.touchMove, 0.0f, 1.0f);

					sf::Vec3 a = intersectHorizontalPlane(0.0f, p.dragStart.worldRay);
					sf::Vec3 b = intersectHorizontalPlane(0.0f, p.current.worldRay);
					float dragAmount = sf::max(0.0f, p.dragAmount);
					if (p.button == Pointer::MouseMiddle) dragAmount = 1.0f;
					if (dragAmount < 1.0f) b = sf::lerp(a, b, dragAmount);
			
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
    
                        if ((p.button == Pointer::MouseMiddle || p.button == Pointer::Touch)
                            && p.action == Pointer::Hold) {
                            sf::Vec3 a = intersectHorizontalPlane(0.0f, p.dragStart.worldRay);
                            sf::Vec3 b = intersectHorizontalPlane(0.0f, p.current.worldRay);
							float dragAmount = sf::max(0.0f, p.dragAmount);
							if (p.button == Pointer::MouseMiddle) dragAmount = 1.0f;
							if (dragAmount < 1.0f) b = sf::lerp(a, b, dragAmount);
    
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

#if 1
		if (ImGui::Begin("Pointers")) {
			sf::SmallStringBuf<128> str;
			for (Pointer &p : input.pointers) {
				str.clear();
				p.formatDebugString(str);
				ImGui::Text("%s", str.data);
			}
		}
        ImGui::End();
#endif

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
		for (uint32_t i = 0; i < tapTargets.size(); i++) {
			TapTarget &target = tapTargets.data[i].val;
			if (target.action == TapTarget::Finish || target.action == TapTarget::Hover) {
				tapTargets.remove(tapTargets.data[i].key);
				i--;
			}
		}

		for (Pointer &pointer : input.pointers) {
			if (pointer.action == Pointer::Down
				&& (pointer.button == Pointer::MouseHover || pointer.button == Pointer::MouseLeft || pointer.button == Pointer::Touch)) {

				sf::Vec3 tilePos = intersectHorizontalPlane(0.0f, pointer.start.worldRay);

				TapTarget target;
				uint32_t entityId = systems.tapArea->getClosestTapAreaEntity(systems.area, pointer.start.worldRay);

				target.tile.x = tilePos.x;
				target.tile.y = tilePos.z;
				target.tileInt = sf::Vec2i(sf::floor(target.tile + sf::Vec2(0.5f)));

				if (pointer.button == Pointer::MouseHover) {
					target.action = TapTarget::Hover;
				} else {
					target.action = TapTarget::Start;
				}

				if (entityId != ~0u) {
					Entity &entity = systems.entities.entities[entityId];
					target.svId = entity.svId;
				}

				tapTargets.insert(pointer.id, target);
			}

			if (pointer.action == Pointer::Up || pointer.action == Pointer::Cancel || !pointer.canTap) {
				if (pointer.action == Pointer::Up && pointer.canTap) {
					if (TapTarget *target = tapTargets.findValue(pointer.id)) {
						target->action = TapTarget::Finish;
					}
				} else {
					tapTargets.remove(pointer.id);
				}
			}
		}

		if (frameArgs.editorOpen) {
			tapTargets.clear();
		}

		bool hasNonHover = false;
		for (auto &pair : tapTargets) {
			if (pair.val.action != TapTarget::Hover) {
				hasNonHover = true;
			}
		}

		for (auto &pair : tapTargets) {
			TapTarget &target = pair.val;

			if (hasNonHover && target.action == TapTarget::Hover) continue;

			uint32_t chrId = svToCharacter.findOne(target.svId, ~0u);
			if (chrId != ~0u) {
				Character &chr = characters[chrId];
				sf::Vec3 tilePos = sf::Vec3((float)chr.tile.x, 0.0f, (float)chr.tile.y);

				sf::Vec4 color;
				if (target.action == TapTarget::Hover) {
					color = sf::Vec4(0.2f, 0.2f, 0.2f, 1.0f) * 0.5f;
				} else {
					color = sf::Vec4(0.5f, 0.2f, 0.2f, 1.0f) * 0.5f;
				}

				if (target.action == TapTarget::Finish) {
					selectedCharacterId = chr.svId;
				}

				sp::SpriteRef sprite { "Assets/Billboards/Character_Select.png" };
				sf::Mat34 t;
				t.cols[0] = sf::Vec3(1.0f, 0.0f, 0.0f);
				t.cols[1] = sf::Vec3(0.0f, 0.0f, 1.0f);
				t.cols[2] = sf::Vec3(0.0f, 1.0f, 0.0f);
				t.cols[3] = tilePos + sf::Vec3(0.0f, 0.05f, 0.0f);
				systems.billboard->addBillboard(sprite, t, color, 1.0f);

			} else if (selectedCharacterId != 0) {

				sf::Vec2i targetTile = target.tileInt;
				if (sv::ReachableTile *moveTile = moveSet.distanceToTile.findValue(targetTile)) {
					moveWaypoints.clear();

					do {
						moveWaypoints.push(targetTile);

						sf::Vec3 tilePos = sf::Vec3((float)targetTile.x, 0.0f, (float)targetTile.y);
						sf::Vec3 prevPos = sf::Vec3((float)moveTile->previous.x, 0.0f, (float)moveTile->previous.y);

						sp::SpriteRef sprite { "Assets/Billboards/Character_Path.png" };

						Billboard bb;

						sf::Vec4 color;
						if (target.action == TapTarget::Hover) {
							bb.color = sf::Vec4(0.2f, 0.2f, 0.2f, 1.0f) * 0.5f;
						} else {
							bb.color = sf::Vec4(0.5f, 0.2f, 0.2f, 1.0f) * 0.5f;
						}

						bb.sprite = sprite;
						bb.transform.cols[2] = sf::Vec3(0.0f, 1.0f, 0.0f);
						bb.transform.cols[1] = sf::normalizeOrZero(tilePos - prevPos);
						bb.transform.cols[0] = sf::cross(bb.transform.cols[2], bb.transform.cols[1]);
						bb.transform.cols[3] = tilePos + sf::Vec3(0.0f, 0.05f, 0.0f);
						bb.depth = 2.0f;
						bb.anchor.y = 1.0f;

						systems.billboard->addBillboard(bb);

						targetTile = moveTile->previous;
						moveTile = moveSet.distanceToTile.findValue(targetTile);
					} while (moveTile);

					if (target.action == TapTarget::Finish) {
						auto action = sf::box<sv::MoveAction>();
						action->charcterId = selectedCharacterId;
						action->tile = target.tileInt;
						action->waypoints = moveWaypoints;
						requestedActions.push(action);
					}

				}

			}
		}

		if (selectedCharacterId != 0) {
			uint32_t chrId = svToCharacter.findOne(selectedCharacterId, ~0u);
			if (chrId != ~0u) {
				Character &chr = characters[chrId];
				sf::Vec3 tilePos = sf::Vec3((float)chr.tile.x, 0.0f, (float)chr.tile.y);

				if (false)
				{
					sp::SpriteRef sprite { "Assets/Billboards/Character_Select.png" };
					sf::Mat34 t;
					t.cols[0] = sf::Vec3(1.0f, 0.0f, 0.0f);
					t.cols[1] = sf::Vec3(0.0f, 0.0f, 1.0f);
					t.cols[2] = sf::Vec3(0.0f, 1.0f, 0.0f);
					t.cols[3] = tilePos + sf::Vec3(0.0f, 0.05f, 0.0f);
					systems.billboard->addBillboard(sprite, t);
				}

				sv::PathfindOpts opts;
				opts.isBlockedFn = &sv::isBlockedByPropOrCharacter;
				opts.maxDistance = 5;
				sv::findReachableSet(moveSet, svState, opts, chr.tile);

				for (auto &pair : moveSet.distanceToTile) {
					sp::SpriteRef sprite { "Assets/Billboards/Character_Move.png" };

					sf::Vec3 tilePos = sf::Vec3((float)pair.key.x, 0.0f, (float)pair.key.y);
					sf::Vec3 prevPos = sf::Vec3((float)pair.val.previous.x, 0.0f, (float)pair.val.previous.y);

					sf::Mat34 t;
					t.cols[0] = sf::Vec3(1.0f, 0.0f, 0.0f);
					t.cols[1] = sf::Vec3(0.0f, 0.0f, 1.0f);
					t.cols[2] = sf::Vec3(0.0f, 1.0f, 0.0f);
					t.cols[3] = tilePos + sf::Vec3(0.0f, 0.05f, 0.0f);
					systems.billboard->addBillboard(sprite, t);
				}

			}
		}
	}

	void updateTransform(Systems &systems, uint32_t entityId, const EntityComponent &ec, const TransformUpdate &update) override
	{
	}

	void remove(Systems &systems, uint32_t entityId, const EntityComponent &ec) override
	{
	}

	void applyEvent(Systems &systems, const sv::Event &event, bool immediate) override
	{
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

		} else if (const auto *e = event.as<sv::MoveEvent>()) {

			uint32_t chrId = svToCharacter.findOne(e->characterId, ~0u);
			if (chrId != ~0u) {
				Character &character = characters[chrId];
				character.tile = e->position;

				Transform transform;
				transform.position = sf::Vec3((float)e->position.x, 0.0f, (float)e->position.y);
				systems.entities.updateTransform(systems, character.entityId, transform);
			}

		} else if (const auto *e = event.as<sv::SelectCardEvent>()) {
			uint32_t characterId = svToCharacter.findOne(e->ownerId, ~0u);
			uint32_t cardId = svToCard.findOne(e->cardId, ~0u);

			if (characterId != ~0u && cardId != ~0u) {
				equipCardImp(systems, characterId, cardId, e->slot);
			}

		}
	}

	void getRequestedActions(sf::Array<sf::Box<sv::Action>> &actions) override
	{
		actions.push(requestedActions);
		requestedActions.clear();
	}

	void handleGui(Systems &systems, const GuiArgs &guiArgs) override
	{
		sp::Canvas &canvas = *guiArgs.canvas;

		if (Character *chr = findCharacter(selectedCharacterId)) {
			sv::CharacterComponent *chrComp = chr->svPrefab->findComponent<sv::CharacterComponent>();
			if (chrComp) {
				float cardHeight = 100.0f;
				float cardWidth = cardHeight * GuiCard::canvasXByY;
				float cardPad = 10.0f;
				float cardMargin = 20.0f;

				float x = cardMargin;
				float y = guiArgs.resolution.y - cardMargin - cardHeight;

				canvas.draw(guiCardRes.inventory, sf::Vec2(guiArgs.resolution.x - cardMargin - cardWidth, y), sf::Vec2(cardWidth, cardHeight));

				uint32_t lastMeleeSlot = 1;
				uint32_t lastSkillSlot = lastMeleeSlot + chrComp->skillSlots;
				uint32_t lastSpellSlot = lastSkillSlot + chrComp->spellSlots;
				uint32_t lastItemSlot = lastSpellSlot + chrComp->itemSlots;

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

					SelectedCard &selected = chr->selectedCards[slot];
					if (Card *card = findCard(selected.currentSvId)) {
						canvas.pushTransform(sf::mat2D::translate(x, y) * sf::mat2D::scale(cardHeight / GuiCard::canvasHeight));
						cl::renderCard(canvas, card->gui);
						canvas.popTransform();
					} else if (guiSlot != GuiCardSlot::Count) {
						canvas.draw(guiCardRes.slotPlaceholders[(uint32_t)guiSlot], sf::Vec2(x, y), sf::Vec2(cardWidth, cardHeight));
					}

					x += cardWidth + cardPad;
				}
			}
		}
	}

};

sf::Box<GameSystem> GameSystem::create(const SystemsDesc &desc) { return sf::box<GameSystemImp>(desc); }

}
