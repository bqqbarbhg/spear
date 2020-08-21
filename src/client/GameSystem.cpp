#include "GameSystem.h"

#include "server/Pathfinding.h"

#include "client/CharacterModelSystem.h"

#include "game/DebugDraw.h"

#include "sf/UintMap.h"
#include "sf/Reflection.h"

#include "client/GuiCard.h"

#include "sp/Renderer.h"

#include "ext/sokol/sokol_app.h"
#include "ext/imgui/imgui.h"

namespace cl {

static Transform getCharacterTransform(const sv::Character &chr)
{
	Transform ret;
	ret.position = sf::Vec3((float)chr.tile.x, 0.0f, (float)chr.tile.y);
	return ret;
}

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

	struct Pointer
	{
		enum Button
		{
			MouseHover,
			MouseLeft,
			MouseRight,
			MouseMiddle,
			Touch,

			LastMouse = MouseMiddle,
		};

		enum Action
		{
			Down,
			Hold,
			Up,
			Cancel,
		};

        enum HitType
        {
            None,
            UI,
            Background,
        };

		struct Position
		{
			sf::Vec2 pos;
			sf::Ray worldRay;
		};

		uintptr_t touchId = 0;
		Button button;
		Action action;
        HitType hitType;
        uint32_t hitIndex;

		float time = 0.0f;
		float dragFactor = -0.1f;

		Position start;
        Position dragStart;
		Position previous;
		Position current;

		void formatDebugString(sf::StringBuf &str) const
		{
			const char *buttonStr, *actionStr;
			switch (button) {
			case MouseHover: buttonStr = "Hover"; break;
			case MouseLeft: buttonStr = "Left"; break;
			case MouseRight: buttonStr = "Right"; break;
			case MouseMiddle: buttonStr = "Middle"; break;
			case Touch: buttonStr = "Touch"; break;
			default: buttonStr = "(???)"; break;
			}

			switch (action) {
			case Down: actionStr = "Down"; break;
			case Hold: actionStr = "Hold"; break;
			case Up: actionStr = "Up"; break;
			case Cancel: actionStr = "Cancel"; break;
			default: actionStr = "(???)"; break;
			}

			str.format("(%2.2fs) %s %s - (%.2f, %.2f) [drag:%.2f]", time, buttonStr, actionStr,
				current.pos.x, current.pos.y, dragFactor);
		}
	};

	struct ScreenToWorld
	{
		sf::Vec2 rcpResolution;
		sf::Mat44 clipToWorld;
	};

	static Pointer::Button sappToPointerButton(sapp_mousebutton button)
	{
		sf_assert(button >= SAPP_MOUSEBUTTON_LEFT && button <= SAPP_MOUSEBUTTON_MIDDLE);
		return (Pointer::Button)((uint32_t)button + 1);
	}

	static sf::Ray pointerToWorld(const sf::Mat44 &clipToWorld, const sf::Vec2 &relativePos)
	{
		sf::Ray ray;
		sf::Vec2 clipMouse = relativePos * sf::Vec2(+2.0f, -2.0f) + sf::Vec2(-1.0f, +1.0f);
		sf::Vec4 rayBegin = clipToWorld * sf::Vec4(clipMouse.x, clipMouse.y, 0.0f, 1.0f);
		sf::Vec4 rayEnd = clipToWorld * sf::Vec4(clipMouse.x, clipMouse.y, 1.0f, 1.0f);
		ray.origin = sf::Vec3(rayBegin.v) / rayBegin.w;
		ray.direction = sf::normalize(sf::Vec3(rayEnd.v) / rayEnd.w - ray.origin);
		return ray;
	}

	static Pointer::Position sappToPointerPosition(const ScreenToWorld &stw, const sf::Vec2 &pos)
	{
		Pointer::Position p;
		p.pos = stw.rcpResolution * pos;
		p.worldRay = pointerToWorld(stw.clipToWorld, p.pos);
		return p;
	}

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

	sf::Array<Pointer> pointers;

	Camera camera;

	bool keyDown[SAPP_MAX_KEYCODES] = { };

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
    
    void initPointer(Pointer &pointer, const Pointer::Position &pos)
    {
        pointer.dragStart = pointer.current = pointer.previous = pointer.start = pos;
        
        // TODO: Hitscan
        pointer.hitType = Pointer::Background;
        pointer.hitIndex = 0;
    }
    
	// -- API

	GameSystemImp(const SystemsDesc &desc)
	{
		camera.previous.origin = camera.current.origin = sf::Vec3(desc.persist.camera.x, 0.0f, desc.persist.camera.y);
		camera.previous.zoom = camera.current.zoom = desc.persist.zoom;
	}

	void updateCamera(FrameArgs &frameArgs) override
	{
		ScreenToWorld screenToWorld;
		screenToWorld.rcpResolution = sf::Vec2(1.0f) / sf::Vec2(frameArgs.resolution);
		screenToWorld.clipToWorld = sf::inverse(frameArgs.mainRenderArgs.worldToClip);

		for (uint32_t i = 0; i < pointers.size; i++) {
			Pointer &p = pointers[i];
			p.time += frameArgs.dt;

			p.previous = p.current;

			if (p.action == Pointer::Down) {
				p.action = Pointer::Hold;
			} else if (p.action == Pointer::Up) {
				pointers.removeOrdered(i);
				i--;
			} else if (p.action == Pointer::Cancel) {
				pointers.removeOrdered(i);
				i--;
			}
		}

		bool mouseBlocked = ImGui::GetIO().WantCaptureMouse;

		bool unfocused = false;
		for (const sapp_event &e : frameArgs.events) {

			if (e.type == SAPP_EVENTTYPE_MOUSE_DOWN) {

				if (!mouseBlocked) {
					Pointer::Position pos = sappToPointerPosition(screenToWorld, sf::Vec2(e.mouse_x, e.mouse_y));

					Pointer *pointer = nullptr;
					Pointer::Button button = sappToPointerButton(e.mouse_button);
					for (Pointer &p : pointers) {
						if (p.button == button) {
							pointer = &p;
							break;
						}
					}

					if (!pointer) {
						pointer = &pointers.push();
						pointer->button = button;
					}

					pointer->time = 0.0f;
					pointer->action = Pointer::Down;
					initPointer(*pointer, pos);
				}
					
			} else if (e.type == SAPP_EVENTTYPE_MOUSE_MOVE) {
				Pointer::Position pos = sappToPointerPosition(screenToWorld, sf::Vec2(e.mouse_x, e.mouse_y));

				for (Pointer &p : pointers) {
					if (p.button <= Pointer::LastMouse) {
						p.current = pos;
					}
				}

			} else if (e.type == SAPP_EVENTTYPE_MOUSE_UP) {
				Pointer::Position pos = sappToPointerPosition(screenToWorld, sf::Vec2(e.mouse_x, e.mouse_y));

				Pointer::Button button = sappToPointerButton(e.mouse_button);
				for (Pointer &p : pointers) {
					if (p.button == button) {
						p.action = Pointer::Up;
						p.current = pos;
					}
				}

			} else if (e.type == SAPP_EVENTTYPE_MOUSE_LEAVE) {

				for (Pointer &p : pointers) {
					if (p.button <= Pointer::LastMouse) {
						p.action = Pointer::Cancel;
					}
				}

			} else if (e.type == SAPP_EVENTTYPE_TOUCHES_BEGAN) {

				if (!mouseBlocked) {
					for (const sapp_touchpoint &touch : sf::slice(e.touches, e.num_touches)) {
						if (!touch.changed) continue;
						
						Pointer::Position pos = sappToPointerPosition(screenToWorld, sf::Vec2(touch.pos_x, touch.pos_y));

						Pointer *pointer = nullptr;
						for (Pointer &p : pointers) {
							if (p.button == Pointer::Touch && p.touchId == touch.identifier) {
								pointer = &p;
								break;
							}
						}

						if (!pointer) {
							pointer = &pointers.push();
							pointer->button = Pointer::Touch;
							pointer->touchId = touch.identifier;
						}

						pointer->time = 0.0f;
						pointer->action = Pointer::Down;
						initPointer(*pointer, pos);
					}
				}

			} else if (e.type == SAPP_EVENTTYPE_TOUCHES_MOVED) {

				for (const sapp_touchpoint &touch : sf::slice(e.touches, e.num_touches)) {
                    if (!touch.changed) continue;
                    
					Pointer::Position pos = sappToPointerPosition(screenToWorld, sf::Vec2(touch.pos_x, touch.pos_y));

					for (Pointer &p : pointers) {
						if (p.button == Pointer::Touch && p.touchId == touch.identifier) {
							p.current = pos;
						}
					}
				}

			} else if (e.type == SAPP_EVENTTYPE_TOUCHES_ENDED) {

				for (const sapp_touchpoint &touch : sf::slice(e.touches, e.num_touches)) {
                    if (!touch.changed) continue;
                    
					Pointer::Position pos = sappToPointerPosition(screenToWorld, sf::Vec2(touch.pos_x, touch.pos_y));
					for (Pointer &p : pointers) {
						if (p.button == Pointer::Touch && p.touchId == touch.identifier) {
							p.action = Pointer::Up;
							p.current = pos;
						}
					}
				}

			} else if (e.type == SAPP_EVENTTYPE_TOUCHES_CANCELLED) {

				for (const sapp_touchpoint &touch : sf::slice(e.touches, e.num_touches)) {
                    if (!touch.changed) continue;
                    
					Pointer::Position pos = sappToPointerPosition(screenToWorld, sf::Vec2(touch.pos_x, touch.pos_y));
					for (Pointer &p : pointers) {
						if (p.button == Pointer::Touch && p.touchId == touch.identifier) {
							p.action = Pointer::Cancel;
							p.current = pos;
						}
					}
				}

			} else if (e.type == SAPP_EVENTTYPE_MOUSE_SCROLL) {

				if (!mouseBlocked) {
					camera.zoomDelta += e.scroll_y * -0.1f;
				}

			} else if (e.type == SAPP_EVENTTYPE_KEY_DOWN) {

				if (!ImGui::GetIO().WantCaptureKeyboard) {
					keyDown[e.key_code] = true;
				}

			} else if (e.type == SAPP_EVENTTYPE_KEY_UP) {

				keyDown[e.key_code] = false;


			} else if (e.type == SAPP_EVENTTYPE_FOCUSED) {

				unfocused = false;

			} else if (e.type == SAPP_EVENTTYPE_UNFOCUSED) {

				unfocused = true;

			}
		}

		if (unfocused) {
			sf::memZero(keyDown);
			for (Pointer &pointer : pointers) {
				pointer.action = Pointer::Cancel;
			}
		}

		for (Pointer &p : pointers) {
			float move = sf::length(p.current.pos - p.previous.pos);
			float dist = 0.2f + sf::lengthSq(p.current.pos - p.start.pos);
			float time = 0.1f + sf::min(p.time, 0.3f);
			p.dragFactor = sf::min(1.0f, p.dragFactor + dist * move * time * 2000.0f);
            
            // Resample dragStart for same class (at least for Background)
            if (p.button == Pointer::Touch
                && p.hitType == Pointer::Background
                && (p.action == Pointer::Up || p.action == Pointer::Cancel)) {
                for (Pointer &p2 : pointers) {
                    if (p2.button == Pointer::Touch
                        && p2.hitType == p.hitType
                        && p2.action == Pointer::Hold) {
                        p2.dragStart.worldRay = pointerToWorld(screenToWorld.clipToWorld, p2.current.pos);
                    }
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
			if (keyDown[SAPP_KEYCODE_A] || keyDown[SAPP_KEYCODE_LEFT]) cameraMove.x -= 1.0f;
			if (keyDown[SAPP_KEYCODE_D] || keyDown[SAPP_KEYCODE_RIGHT]) cameraMove.x += 1.0f;
			if (keyDown[SAPP_KEYCODE_W] || keyDown[SAPP_KEYCODE_UP]) cameraMove.y -= 1.0f;
			if (keyDown[SAPP_KEYCODE_S] || keyDown[SAPP_KEYCODE_DOWN]) cameraMove.y += 1.0f;

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

			for (Pointer &p : pointers) {
				p.current.worldRay = pointerToWorld(clipToWorld, p.current.pos);

                if ((p.button == Pointer::MouseMiddle || p.button == Pointer::Touch)
                    && p.action == Pointer::Hold && p.hitType == Pointer::Background) {

					if (p.button == Pointer::Touch) {
						camera.touchMove += cameraDt * 4.0f;
					} else {
						camera.touchMove -= cameraDt * 4.0f;
					}
					camera.touchMove = sf::clamp(camera.touchMove, 0.0f, 1.0f);
                    
                    numDrags += 1;
                    dragStart += intersectHorizontalPlane(0.0f, p.dragStart.worldRay);
                    dragCurrent += intersectHorizontalPlane(0.0f, p.current.worldRay);
				}
			}
            
            if (numDrags > 0) {
                dragStart /= (float)numDrags;
                dragCurrent /= (float)numDrags;
                
                if (numDrags > 1) {
                    camera.touchMove -= cameraDt * 20.0f * (float)numDrags;
                    camera.touchMove = sf::clamp(camera.touchMove, 0.0f, 1.0f);

                    float dragZoom = 0.0f;
                    for (Pointer &p : pointers) {
                        p.current.worldRay = pointerToWorld(clipToWorld, p.current.pos);
    
                        if ((p.button == Pointer::MouseMiddle || p.button == Pointer::Touch)
                            && p.action == Pointer::Hold && p.hitType == Pointer::Background) {
                            sf::Vec3 a = intersectHorizontalPlane(0.0f, p.dragStart.worldRay);
                            sf::Vec3 b = intersectHorizontalPlane(0.0f, p.current.worldRay);
    
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


		if (ImGui::Begin("Pointers")) {
			sf::SmallStringBuf<128> str;
			for (Pointer &p : pointers) {
				str.clear();
				p.formatDebugString(str);
				ImGui::Text("%s", str.data);
			}
		}
        ImGui::End();

		Camera::State state = Camera::lerp(camera.previous, camera.current, camera.timeDelta / cameraDt);

		sf::Vec3 eye;
		sf::Mat34 worldToView;
		sf::Mat44 viewToClip;
		state.asMatrices(eye, worldToView, viewToClip, aspect);

		frameArgs.mainRenderArgs.cameraPosition = eye;
		frameArgs.mainRenderArgs.worldToView = worldToView;
		frameArgs.mainRenderArgs.viewToClip = viewToClip;
		frameArgs.mainRenderArgs.worldToClip = viewToClip * worldToView;
		frameArgs.mainRenderArgs.frustum = sf::Frustum(frameArgs.mainRenderArgs.worldToClip, sp::getClipNearW());
	}

	void writePersist(Systems &systems, ClientPersist &persist) override
	{
		persist.camera.x = camera.current.origin.x;
		persist.camera.y = camera.current.origin.z;
		persist.zoom = camera.current.zoom;
	}

	void update(const sv::ServerState &svState, const FrameArgs &frameArgs) override
	{
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

sf::Box<GameSystem> GameSystem::create(const SystemsDesc &desc) { return sf::box<GameSystemImp>(desc); }

}
