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
		float time = 0.0f;
		bool active = true;
	};

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

	gui::GuiResources guiResources;

	sf::ImplicitHashMap<PointerState, sv::KeyId> pointerStates;

	sf::Array<gui::Widget*> guiWorkArray;
	gui::GuiBuilder guiBuilder;
	sf::Box<gui::Widget> guiRoot;

	sf::Array<DragPointer> dragPointers;

	bool showDebugMenu = false;
	bool showDebugPointers = false;
	bool simulateTouch = false;

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
				}

				dragPointer->active = true;
				gui::GuiPointer &gp2 = dragPointer->guiPointer;
				gp2.id = gp.id;
				gp2.position = gp.position;
				gp2.delta = gp.delta;
				gp2.dragFactor = gp.dragFactor;
				gp2.button = gp.button;
				if (p.action == gui::GuiPointer::Up || p.action == gui::GuiPointer::Tap) {
					gp2.action = gui::GuiPointer::DropCommit;
					gp2.end = true;
				} else if (p.action == gui::GuiPointer::Cancel) {
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
				guiRoot->onPointer(dragPointer.guiPointer);
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
						requestedActions.push(action);
					}
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

			} else if (selectedCharacterId != 0 && false) {

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

		if (selectedCharacterId != 0 && false) {
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

		{
			gui::GuiBuilder &b = guiBuilder;
			b.init(guiRoot);

			Character *chr = findCharacter(selectedCharacterId);
			sv::CharacterComponent *chrComp = chr ? chr->svPrefab->findComponent<sv::CharacterComponent>() : NULL;
			if (chr && chrComp) {

				{
					float hotbarCardHeight = 140.0f;

					auto ll = b.push<gui::WidgetLinearLayout>(chr->svId);
					ll->direction = gui::DirX;
					ll->boxExtent = sf::Vec2(gui::Inf, hotbarCardHeight);
					ll->boxOffset.x = 20.0f;
					ll->boxOffset.y = frameArgs.guiResolution.y - ll->boxExtent.y - 20.0f;
					ll->padding = 10.0f;

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

						if (guiSlot != GuiCardSlot::Count) {
							auto sl = b.push<gui::WidgetCardSlot>(slot);
							if (sl->created) {
								sl->startAnim = 1.0f + (float)slot * 0.2f;
							}

							sl->slot = guiSlot;
							sl->slotIndex = slot;

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
								requestedActions.push(action);
								sl->droppedCard.reset();
							}

							b.pop(); // CardSlot
						}
					}

					b.pop(); // LinearLayout
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
			card.gui = sf::box<GuiCard>();
			card.gui->init(*card.svPrefab, card.svId);

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

		} else if (const auto *e = event.as<sv::GiveCardEvent>()) {

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

		{
			gui::GuiPaint paint;
			paint.canvas = &canvas;
			paint.resources = &guiResources;
			paint.crop.max = guiArgs.resolution;
			guiRoot->paint(paint);
		}

		for (DragPointer &dragPointer : dragPointers) {
			float t = sf::clamp(dragPointer.time * 7.0f, 0.0f, 1.0f);
			t = t * t * (3.0f - 2.0 * t);

			float dragHeight = 150.0f;
			sf::Vec2 dragOffset = dragPointer.guiPointer.position + sf::Vec2(-20.0f, -dragHeight - 10.0f);
			sf::Vec2 dragSize = sf::Vec2(GuiCard::canvasXByY * dragHeight, dragHeight);

			sf::Vec2 offset = sf::lerp(dragPointer.guiPointer.dropOffset, dragOffset, t);
			sf::Vec2 size = sf::lerp(dragPointer.guiPointer.dropSize, dragSize, t);

			if (dragPointer.guiPointer.dropType == guiCardSym) {
				GuiCard *guiCard = dragPointer.guiPointer.dropData.cast<GuiCard>();

				sf::Mat23 mat;
				mat.m00 = size.x * (1.0f/500.0f);
				mat.m11 = size.y * (1.0f/800.0f);
				mat.m02 = offset.x;
				mat.m12 = offset.y;

				mat = mat * sf::mat2D::rotate(t*-0.1f);

				canvas.pushTransform(mat);
				renderCard(canvas, *guiCard);
				canvas.popTransform();
			}
		}

#if 0

		canvas.pushCrop(sf::Vec2(0.0f, guiArgs.resolution.y - 60.0f), sf::Vec2(1000.0f, guiArgs.resolution.y));
		canvas.pushCrop(sf::Vec2(150.0f, 0.0f), sf::Vec2(300.0f, 10000.0f));

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

		canvas.popCrop();
		canvas.popCrop();

#endif
	}

};

sf::Box<GameSystem> GameSystem::create(const SystemsDesc &desc) { return sf::box<GameSystemImp>(desc); }

}
