#include "EditorState.h"

#include "client/ClientState.h"
#include "sf/Geometry.h"
#include "sf/Sort.h"

#include "ext/imgui/imgui.h"
#include "ext/imgui/ImGuizmo.h"

#include "game/ImguiSerialization.h"

namespace cl {

struct EditorState
{
	sf::Box<sv::ServerState> svState;
	sf::Box<cl::ClientState> clState;

	sf::HashSet<uint32_t> selectedSvIds;

	// Pending edits
	sf::Array<sf::Array<sf::Box<sv::Edit>>> pendingEdits;

	// Input
	bool mouseDown = false;
	bool prevMouseDown = false;

	// Dragging
	bool draggingSelection = false;
	bool didDragSelection = false;
	sf::Vec3 dragOrigin;
	uint32_t dragSvId = 0;
	sf::Vec2i dragPrevOffset;
	bool dragSvIdAlreadySelected = false;
	bool dragShiftDown = false;
	bool dragClone = false;
	sf::Array<sv::Prop> dragProps;
};

EditorState *editorCreate(const sf::Box<sv::ServerState> &svState, const sf::Box<cl::ClientState> &clState)
{
	EditorState *es = new EditorState();
	es->svState = svState;
	es->clState = clState;
	return es;
}

void editorFree(EditorState *es)
{
	delete es;
}

void editorUpdate(EditorState *es, const FrameArgs &frameArgs, const ClientInput &input)
{
	es->prevMouseDown = es->mouseDown;
	es->mouseDown = ImGui::GetIO().MouseDown[0];

	bool shiftDown = ImGui::IsKeyDown(SAPP_KEYCODE_LEFT_SHIFT);
	bool ctrlDown = ImGui::IsKeyDown(SAPP_KEYCODE_LEFT_CONTROL);

	ImGuizmo::BeginFrame();
	ImGuizmo::Enable(true);
	ImGuizmo::SetRect(0.0f, 0.0f, (float)input.resolution.x, (float)input.resolution.y);

	sf::Mat44 clipToWorld = sf::inverse(frameArgs.worldToClip);
	sf::Vec2 clipMouse = input.mousePosition * sf::Vec2(+2.0f, -2.0f) + sf::Vec2(-1.0f, +1.0f);
	sf::Vec4 rayBegin = clipToWorld * sf::Vec4(clipMouse.x, clipMouse.y, 0.0f, 1.0f);
	sf::Vec4 rayEnd = clipToWorld * sf::Vec4(clipMouse.x, clipMouse.y, 1.0f, 1.0f);
	sf::Vec3 rayOrigin = sf::Vec3(rayBegin.v) / rayBegin.w;
	sf::Vec3 rayDirection = sf::normalize(sf::Vec3(rayEnd.v) / rayEnd.w - rayOrigin);

	sf::Ray mouseRay = { rayOrigin, rayDirection };

	sf::Array<cl::EntityHit> hits;
	es->clState->editorPick(hits, mouseRay);

	sf::sortBy(hits, [](const cl::EntityHit &a) { return a.t; });

	if (es->draggingSelection) {
		float dragT = (es->dragOrigin.y - mouseRay.origin.y) / mouseRay.direction.y;
		sf::Vec3 dragPos = rayOrigin + rayDirection * dragT;
		sf::Vec2 dragTile = sf::Vec2(dragPos.x, dragPos.z) - sf::Vec2(es->dragOrigin.x, es->dragOrigin.z);
		sf::Vec2i dragTileOffset = sf::Vec2i(sf::floor(dragTile + sf::Vec2(0.5f)));

		if (dragTileOffset != es->dragPrevOffset) {
			es->didDragSelection = true;
			es->dragPrevOffset = dragTileOffset;

			sf::SmallArray<sf::Box<sv::Event>, 128> events;

			for (sv::Prop &prop : es->dragProps) {
				sv::PropTransform transform = prop.transform;
				transform.tile += dragTileOffset;

				es->svState->moveProp(events, prop.id, transform);
			}

			for (sv::Event *event : events) {
				es->clState->applyEvent(*event);
			}
		}

		ImGui::SetNextItemOpen(true);
		handleImgui(dragTileOffset, "dragTileOffset");
	}

	if (!es->mouseDown) {
		if (es->draggingSelection) {
			if (!es->didDragSelection) {

				if (es->dragClone) {
					sf::SmallArray<sf::Box<sv::Event>, 128> events;
					for (sv::Prop &prop : es->dragProps) {
						es->svState->removeProp(events, prop.id);
					}
					for (sv::Event *event : events) {
						es->clState->applyEvent(*event);
					}
				}

				if (es->dragSvIdAlreadySelected) {
					if (es->selectedSvIds.size() > 1) {
						if (es->dragShiftDown) {
							es->selectedSvIds.remove(es->dragSvId);
						} else {
							es->selectedSvIds.clear();
							es->selectedSvIds.insert(es->dragSvId);
						}
					} else {
						es->selectedSvIds.clear();
					}
				}
			} else {
				sf::Array<sf::Box<sv::Edit>> &edits = es->pendingEdits.push();
				if (es->dragClone) {
					for (sv::Prop &prop : es->dragProps) {
						auto ed = sf::box<sv::ClonePropEdit>();
						ed->clientId = es->svState->localClientId;
						ed->localId = prop.id;
						ed->prop = prop;
						ed->prop.transform.tile += es->dragPrevOffset;
						edits.push(ed);
					}
				} else {
					for (sv::Prop &prop : es->dragProps) {
						auto ed = sf::box<sv::MovePropEdit>();
						ed->propId = prop.id;
						ed->transform = prop.transform;
						ed->transform.tile += es->dragPrevOffset;
						edits.push(ed);
					}
				}
			}
		}

		es->draggingSelection = false;
		es->didDragSelection = false;
	}

	bool foundHover = false;
	for (cl::EntityHit &hit : hits) {
		Entity &entity = es->clState->systems.entities.entities[hit.entityId];
		if (entity.svId == 0) continue;
		foundHover = true;

		if (!es->draggingSelection) {
			es->clState->editorHighlight(hit.entityId, cl::EditorHighlight::Hover);
		}

		if (es->mouseDown && !es->prevMouseDown) {
			es->dragSvId = entity.svId;
			es->dragSvIdAlreadySelected = (es->selectedSvIds.find(entity.svId) != nullptr);
			es->dragOrigin = mouseRay.origin + mouseRay.direction * hit.t;
			es->draggingSelection = true;
			es->didDragSelection = false;
			es->dragShiftDown = shiftDown;
			es->dragClone = ctrlDown;
			es->dragPrevOffset = sf::Vec2i();

			if (!shiftDown && !es->dragSvIdAlreadySelected) es->selectedSvIds.clear();
			es->selectedSvIds.insert(entity.svId);

			es->dragProps.clear();
			for (uint32_t svId : es->selectedSvIds) {
				if (sv::Prop *prop = es->svState->props.find(svId)) {
					es->dragProps.push(*prop);
				}
			}

			if (es->dragClone) {
				es->selectedSvIds.clear();

				sf::SmallArray<sf::Box<sv::Event>, 128> events;

				for (sv::Prop &prop : es->dragProps) {
					uint32_t svId = es->svState->addProp(events, prop, true);
					es->selectedSvIds.insert(svId);
				}

				for (sv::Event *event : events) {
					es->clState->applyEvent(*event);
				}

				es->dragProps.clear();
				for (uint32_t svId : es->selectedSvIds) {
					if (sv::Prop *prop = es->svState->props.find(svId)) {
						es->dragProps.push(*prop);
					}
				}
			}
		}

		break;
	}

	if (es->mouseDown && !es->prevMouseDown) {
		if (!foundHover && !shiftDown) {
			es->selectedSvIds.clear();
		}
	}

	for (uint32_t svId : es->selectedSvIds) {
		uint32_t entityId;
		sf::UintFind find = es->clState->systems.entities.svToEntity.findAll(svId);
		while (find.next(entityId)) {
			es->clState->editorHighlight(entityId, cl::EditorHighlight::Select);
		}
	}

	return;
}

sf::Array<sf::Array<sf::Box<sv::Edit>>> &editorPendingEdits(EditorState *es)
{
	return es->pendingEdits;
}

#if 0
if (drawGizmo) {
	cl::Entity &entity = es->clState->systems.entities.entities[entityId];
	sf::Mat44 view = frameArgs.worldToView;
	sf::Mat44 proj = frameArgs.viewToClip;
	sf::Mat44 matrix = entity.transform.asMatrix();
	ImGuizmo::Manipulate(view.v, proj.v, ImGuizmo::TRANSLATE, ImGuizmo::WORLD, matrix.v);

	sf::Vec3 translation, rotation, scale;
	ImGuizmo::DecomposeMatrixToComponents(matrix.v, translation.v, rotation.v, scale.v);

	Transform transform;
	transform.position = translation;
	transform.rotation = sf::eulerAnglesToQuat(rotation * (sf::F_PI/180.0f));
	transform.scale = 1.0f;
	es->clState->systems.entities.updateTransform(es->clState->systems, entityId, transform);
}
#endif


}
