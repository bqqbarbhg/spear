#include "EditorState.h"

#include "client/ClientState.h"
#include "sf/Geometry.h"
#include "sf/Sort.h"
#include "server/Message.h"

#include "ext/imgui/imgui.h"
#include "ext/imgui/ImGuizmo.h"

#include "game/ImguiSerialization.h"

#include "sf/Reflection.h"

#include "sf/File.h"
#include "sp/Json.h"

namespace cl {

struct EditorFile
{
	sf::Symbol path;
	sf::CString name;

	EditorFile(sf::String prefix, sf::String fileName)
	{
		sf::SmallStringBuf<512> localPath;
		localPath.append(prefix, "/", fileName);
		path = sf::Symbol(localPath);
		name = sf::CString(sf::String(path).slice().drop(prefix.size + 1));
	}
};

struct EditorDir
{
	bool expanded = false;
	sf::Array<EditorDir> dirs;
	sf::StringBuf name;
	sf::StringBuf prefix;
	sf::Array<EditorFile> files;
};

struct EditorState
{
	sf::Box<sv::ServerState> svState;
	sf::Box<cl::ClientState> clState;

	sf::HashSet<uint32_t> selectedSvIds;

	// Pending edits
	sf::Array<sf::Box<sv::Event>> editEvents;
	EditorRequests requests;

	// Input
	bool mouseDown = false;
	bool prevMouseDown = false;

	// Prefab properties
	sf::Array<char> prefabCopyBuffer;
	sf::Symbol selectedPrefab;

	// Ghost prefab
	sf::HashSet<sf::Symbol> preloadedPrefabNames;
	uint32_t ghostPropId = 0;
	sf::Vec2i ghostPropPrevTile;

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

	// Imgui windows
	bool windowAssets = false;
	bool windowPrefabs = false;
	bool windowProperties = false;
	
	// Folder navigation
	EditorDir dirAssets;
	EditorDir dirPrefabs;

	// File/folder creation
	sf::StringBuf addPath;
	sf::StringBuf addInput;
	bool addPrefab = false;
	bool addFolder = false;
};

const sf::String materialSuffixes[] = {
	"_BaseColor.png",
	"_Base_Color.png",
	"_Height.png",
	"_Metallic.png",
	"_Normal.png",
	"_Roughness.png",
	"_Mixed_AO.png",
	"_Normal_DirectX.png",
	"_Normal_OpenGL.png",
	"_Emissive.png",
};

void setupEditorDir(EditorDir &dir, const sf::StringBuf &queryRoot, const sv::QueryDir &queryDir)
{
	if (queryRoot == dir.prefix) {

		sf::HashSet<sf::StringBuf> materials;
		for (const sv::QueryFile &file : queryDir.files) {
			bool addFile = true;
			for (sf::String suffix : materialSuffixes) {
				if (sf::endsWith(file.name, suffix)) {
					sf::String baseName = file.name.slice().dropRight(suffix.size);
					if (materials.insert(baseName).inserted) {
						dir.files.push(EditorFile(dir.prefix, baseName));
					}
					addFile = false;
					break;
				}
			}

			if (addFile) {
				dir.files.push(EditorFile(dir.prefix, file.name));
			}
		}

		for (const sv::QueryDir &qdir : queryDir.dirs) {
			EditorDir &child = dir.dirs.push();
			child.name = qdir.name;
			child.prefix.append(dir.prefix, "/", child.name);
		}

	} else {
		for (EditorDir &child : dir.dirs) {
			if (sf::beginsWith(queryRoot, child.prefix)) {
				setupEditorDir(child, queryRoot, queryDir);
			}
		}
	}
}

static void savePrefab(const sv::Prefab &prefab)
{
	#if SF_OS_WASM
		return;
	#endif

	jso_stream s = { };
	jso_init_file(&s, prefab.name.data);
	s.pretty = true;

	sp::writeJson(s, prefab);

	jso_close(&s);
}


EditorState *editorCreate(const sf::Box<sv::ServerState> &svState, const sf::Box<cl::ClientState> &clState)
{
	EditorState *es = new EditorState();

	es->svState = svState;
	es->clState = clState;

	es->dirAssets.prefix = "Assets";
	es->dirPrefabs.prefix = "Prefabs";

	return es;
}

void editorFree(EditorState *es)
{
	delete es;
}

bool editorPeekEventPre(EditorState *es, const sv::Event &event)
{
	if (const auto *e = event.as<sv::ReplaceLocalPropEvent>()) {
		if (e->clientId == es->svState->localClientId) {
			if (es->selectedSvIds.find(e->localId)) {
				es->selectedSvIds.remove(e->localId);
				es->selectedSvIds.insert(e->prop.id);
			}
		}
	}

	return false;
}

void editorAddQueryDir(EditorState *es, const sf::StringBuf &root, const sv::QueryDir &dir)
{
	if (sf::beginsWith(root, es->dirPrefabs.prefix)) {
		setupEditorDir(es->dirPrefabs, root, dir);
	} else if (sf::beginsWith(root, es->dirAssets.prefix)) {
		setupEditorDir(es->dirAssets, root, dir);
	}
}

void handleImguiMenu(EditorState *es)
{
	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {
				es->requests.undo = true;
			}

			if (ImGui::MenuItem("Redo", "CTRL+Y")) {
				es->requests.redo = true;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window")) {
			if (ImGui::MenuItem("Assets")) es->windowAssets = true;
			if (ImGui::MenuItem("Prefabs")) es->windowPrefabs = true;
			if (ImGui::MenuItem("Properties")) es->windowProperties = true;
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void handleImguiAssetDir(EditorState *es, EditorDir &dir)
{
	if (!dir.expanded) {
		dir.expanded = true;
		es->requests.queryDirs.push(dir.prefix);
		return;
	}

	for (EditorDir &d : dir.dirs) {
		sf::SmallStringBuf<128> nameCopy;
		nameCopy.append(d.name, "/");
		if (ImGui::TreeNode(nameCopy.data)) {
			handleImguiAssetDir(es, d);
			ImGui::TreePop();
		}
	}

	for (const EditorFile &f : dir.files) {

		ImGui::Button(f.name.data);
		if (ImGui::BeginDragDropSource(0)) {
			ImGui::Text("%s", f.path.data);
			ImGui::SetDragDropPayload("asset", f.path.data, f.path.size());
			ImGui::EndDragDropSource();
		}
	}
}

void handleImguiPrefabDir(EditorState *es, EditorDir &dir)
{
	if (!dir.expanded) {
		dir.expanded = true;
		es->requests.queryDirs.push(dir.prefix);
		return;
	}

	for (EditorDir &d : dir.dirs) {
		sf::SmallStringBuf<128> nameCopy;
		nameCopy.append(d.name, "/");

		if (es->addPath.size > 0 && sf::beginsWith(d.prefix, es->addPath)) {
			ImGui::SetNextItemOpen(true);
		}

		bool open = ImGui::TreeNode(nameCopy.data);

		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("New Prefab")) {
				es->addPath = d.prefix;
				es->addInput.clear();
				es->addPrefab = true;
			}
			if (ImGui::MenuItem("New Folder")) {
				es->addPath = d.prefix;
				es->addInput.clear();
				es->addFolder = true;
			}
			ImGui::EndPopup();
		}

		if (open) {
			if (es->addPath.size > 0 && es->addPath == d.prefix) {
				es->addInput.reserve(256);
				if (ImGui::InputText("##Name", es->addInput.data, es->addInput.capacity, ImGuiInputTextFlags_EnterReturnsTrue)) {
					es->addInput.resize(strlen(es->addInput.data));

					if (es->addPrefab) {
						sf::SmallStringBuf<128> fileName;
						fileName.append(es->addInput);
						if (!sf::endsWith(fileName, ".json")) {
							fileName.append(".json");
						}

						d.files.push(EditorFile(d.prefix, fileName));

						sf::Symbol prefabName = d.files.back().path;

						{
							sv::Prefab emptyPrefab;
							emptyPrefab.name = prefabName;
							savePrefab(emptyPrefab);
						}

						{
							sf::Array<sf::Box<sv::Edit>> &edits = es->requests.edits.push();
							auto ed = sf::box<sv::PreloadPrefabEdit>();
							ed->prefabName = prefabName;
							edits.push(ed);
						}

						es->selectedPrefab = prefabName;

					} else if (es->addFolder) {
						EditorDir &newD = d.dirs.push();
						newD.name = es->addInput;
						newD.prefix.append(d.prefix, "/", es->addInput);
						sf::createDirectory(newD.prefix);
					}

					es->addPath.clear();
					es->addPrefab = false;
					es->addFolder = false;
					es->addInput.clear();
				}
				ImGui::SetKeyboardFocusHere(-1);
			}

			handleImguiPrefabDir(es, d);
			ImGui::TreePop();
		}
	}

	for (const EditorFile &f : dir.files) {

		sf::String buttonText = f.name;
		sf::SmallStringBuf<128> localButtonText;

		bool doPreload = false;

		sf::Symbol prefabName = f.path;
		if (ImGui::Button(buttonText.data)) {
			es->selectedPrefab = prefabName;
			es->windowProperties = true;
			doPreload = true;

		}
		if (ImGui::BeginDragDropSource(0)) {
			ImGui::Text("%s", f.path.data);
			ImGui::SetDragDropPayload("prefab", f.path.data, f.path.size());
			ImGui::EndDragDropSource();
			doPreload = true;
		}

		if (doPreload && !es->svState->prefabs.find(prefabName)) {
			if (es->preloadedPrefabNames.insert(prefabName).inserted) {
				sf::Array<sf::Box<sv::Edit>> &edits = es->requests.edits.push();
				auto ed = sf::box<sv::PreloadPrefabEdit>();
				ed->prefabName = prefabName;
				edits.push(ed);
			}
		}
	}
}

void handleImguiDirectoryBrowsers(EditorState *es)
{
	ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_Appearing);
	if (es->windowAssets) {
		if (ImGui::Begin("Assets", &es->windowAssets)) {
			if (ImGui::Button("Refresh")) {
				es->dirAssets.expanded = false;
				es->dirAssets.dirs.clear();
				es->dirAssets.files.clear();
			}

			handleImguiAssetDir(es, es->dirAssets);
		}
		ImGui::End();
	}

	ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_Appearing);
	if (es->windowPrefabs) {
		if (ImGui::Begin("Prefabs", &es->windowPrefabs)) {
			if (ImGui::Button("Refresh")) {
				es->dirPrefabs.expanded = false;
				es->dirPrefabs.dirs.clear();
				es->dirPrefabs.files.clear();
			}

	#if 0
			ImGui::SameLine();
			if (ImGui::Button("Save all")) {
				saveModifiedObjects(c);
			}
	#endif

			handleImguiPrefabDir(es, es->dirPrefabs);
		}
		ImGui::End();
	}
}

static bool imguiCallback(void *user, ImguiStatus &status, void *inst, sf::Type *type, const sf::CString &label)
{
#if 0
	if (type == sf::typeOf<sf::Vec3>()) {
		sf::Vec3 *vec = (sf::Vec3*)inst;

		if (label == sf::String("position")) {
			status.modified |= ImGui::SliderFloat3(label.data, vec->v, -1.0f, +1.0f);
			status.changed |= ImGui::IsItemDeactivatedAfterEdit();
			return true;
		} else if (label == sf::String("stretch")) {
			status.modified |= ImGui::SliderFloat3(label.data, vec->v, -2.0f, +2.0f);
			status.changed |= ImGui::IsItemDeactivatedAfterEdit();
			return true;
		} else if (label == sf::String("rotation")) {
			status.modified |= ImGui::SliderFloat3(label.data, vec->v, -180.0f, 180.0f);
			status.changed |= ImGui::IsItemDeactivatedAfterEdit();
			return true;
		} else if (label == sf::String("color")) {
			status.modified |= ImGui::ColorEdit3(label.data, vec->v, 0);
			status.changed |= ImGui::IsItemDeactivatedAfterEdit();
			return true;
		}

	} else if (type == sf::typeOf<uint8_t[3]>()) {
		uint8_t *src = (uint8_t*)inst;

		if (label == sf::String("tintColor")) {
			float colorF[3] = { (float)src[0] / 255.0f, (float)src[1] / 255.0f, (float)src[2] / 255.0f };
			if (ImGui::ColorEdit3(label.data, colorF, 0)) {
				src[0] = (uint8_t)sf::clamp(colorF[0] * 255.0f, 0.0f, 255.0f);
				src[1] = (uint8_t)sf::clamp(colorF[1] * 255.0f, 0.0f, 255.0f);
				src[2] = (uint8_t)sf::clamp(colorF[2] * 255.0f, 0.0f, 255.0f);
				status.modified = true;
			}
			status.changed |= ImGui::IsItemDeactivatedAfterEdit();
			return true;
		}

	} else if (type == sf::typeOf<float>()) {
		float *v = (float*)inst;

		if (label == sf::String("intensity")) {
			status.modified |= ImGui::SliderFloat(label.data, v, 0.0f, 10.0f);
			status.changed |= ImGui::IsItemDeactivatedAfterEdit();
			return true;
		} else if (label == sf::String("radius")) {
			status.modified |= ImGui::SliderFloat(label.data, v, 0.0f, 10.0f);
			status.changed |= ImGui::IsItemDeactivatedAfterEdit();
			return true;
		} else if (label == sf::String("scale")) {
			status.modified |= ImGui::SliderFloat(label.data, v, 0.0f, 5.0f);
			status.changed |= ImGui::IsItemDeactivatedAfterEdit();
			return true;
		}

	} else if (type == sf::typeOf<sf::Symbol>()) {
		sf::Symbol *sym = (sf::Symbol*)inst;

		bool isModel = label == sf::String("model");
		bool isShadowModel = label == sf::String("shadowModel");
		bool isMaterial = label == sf::String("material");

		if (isModel || isShadowModel || isMaterial) {
			sf::SmallStringBuf<4096> textBuf;
			textBuf.append(*sym);
			if (ImGui::InputText(label.data, textBuf.data, textBuf.capacity, ImGuiInputTextFlags_AlignRight | ImGuiInputTextFlags_AutoSelectAll)) {
				status.modified = true;
				textBuf.resize(strlen(textBuf.data));
				if (ImGui::IsItemDeactivatedAfterEdit()) {
					*sym = sf::Symbol(textBuf);
					status.changed = true;
				}
			}

			if (ImGui::BeginDragDropTarget()) {
				const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("asset");
				if (payload) {
					*sym = sf::Symbol((const char*)payload->Data, payload->DataSize);
					status.modified = true;
					status.changed = true;
				}

				ImGui::EndDragDropTarget();
			}

			return true;
		}

	}
#endif

	return false;
}


void handleImguiPrefab(EditorState *es, ImguiStatus &status, sv::Prefab &prefab)
{
	ImGui::Text("%s", prefab.name.data);

	sf::Type *componentType = sf::typeOf<sv::Component>();

	for (uint32_t compI = 0; compI < prefab.components.size; compI++) {
		ImGui::PushID(compI);

		sf::PolymorphInstance poly = componentType->instGetPolymorph(prefab.components[compI].ptr);

		if (!ImGui::CollapsingHeader(poly.type->name.data, ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::PopID();
			continue;
		}

		bool doDelete = ImGui::Button("Delete");

		handleFieldsImgui(status, poly.inst, poly.type->type, imguiCallback, NULL);

		ImGui::PopID();

		if (doDelete) {
			prefab.components.removeOrdered(compI);
			status.changed = true;
			status.modified = true;
			compI--;
		}
	}

	sf::SmallArray<const char*, 128> itemNames;
	sf::SmallArray<sf::StringBuf, 128> names;
	componentType->getPolymorphTypeNames(names);

	itemNames.push("Add component");

	itemNames.reserve(names.size);
	for (sf::StringBuf &name : names) {
		itemNames.push(name.data);
	}

	int selected = 0;
	if (ImGui::Combo("##add", &selected, itemNames.data, itemNames.size)) {
		if (selected > 0) {
			sf::Box<sv::Component> box;
			sf::Type *boxType = sf::typeOf<decltype(box)>();
			const sf::PolymorphType *poly = boxType->elementType->getPolymorphTypeByName(names[selected - 1]);
			boxType->instSetPolymorph(&box, poly->type);

			prefab.components.push(box);

			status.changed = true;
			status.modified = true;
		}
	}
}

void handleImguiPropertiesWindow(EditorState *es)
{
	ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_Appearing);
	if (es->windowProperties) {
		if (ImGui::Begin("Properties", &es->windowProperties)) {
			if (sv::Prefab *sourcePrefab = es->svState->prefabs.find(es->selectedPrefab)) {

				sv::Prefab prefab;
				es->prefabCopyBuffer.clear();
				sf::writeBinary(es->prefabCopyBuffer, *sourcePrefab);
				sf::readBinary(es->prefabCopyBuffer.slice(), prefab);
				ImguiStatus status;

				handleImguiPrefab(es, status, prefab);

				if (status.modified || status.changed) {
					es->svState->reloadPrefab(es->editEvents, prefab);
				}

				if (status.changed) {
					sf::Array<sf::Box<sv::Edit>> &edits = es->requests.edits.push();
					auto ed = sf::box<sv::ModifyPrefabEdit>();
					ed->prefab = prefab;
					edits.push(ed);
				}
			}
		}
		ImGui::End();
	}
}

void editorUpdate(EditorState *es, const FrameArgs &frameArgs, const ClientInput &input)
{
	for (uint32_t i = 0; i < es->selectedSvIds.size(); i++) {
		uint32_t svId = es->selectedSvIds.data[i];
		if (!es->svState->isIdValid(svId)) {
			es->selectedSvIds.remove(svId);
			i--;
		}
	}

	es->prevMouseDown = es->mouseDown;
	es->mouseDown = ImGui::GetIO().MouseDown[0];

	bool shiftDown = ImGui::IsKeyDown(SAPP_KEYCODE_LEFT_SHIFT);
	bool ctrlDown = ImGui::IsKeyDown(SAPP_KEYCODE_LEFT_CONTROL);

	sf::Mat44 clipToWorld = sf::inverse(frameArgs.worldToClip);
	sf::Vec2 clipMouse = input.mousePosition * sf::Vec2(+2.0f, -2.0f) + sf::Vec2(-1.0f, +1.0f);
	sf::Vec4 rayBegin = clipToWorld * sf::Vec4(clipMouse.x, clipMouse.y, 0.0f, 1.0f);
	sf::Vec4 rayEnd = clipToWorld * sf::Vec4(clipMouse.x, clipMouse.y, 1.0f, 1.0f);
	sf::Vec3 rayOrigin = sf::Vec3(rayBegin.v) / rayBegin.w;
	sf::Vec3 rayDirection = sf::normalize(sf::Vec3(rayEnd.v) / rayEnd.w - rayOrigin);

	sf::Ray mouseRay = { rayOrigin, rayDirection };
	float rayT = rayOrigin.y / -rayDirection.y;
	sf::Vec3 rayPos = rayOrigin + rayDirection * rayT;
	sf::Vec2 mouseTile = sf::Vec2(rayPos.x, rayPos.z);
	sf::Vec2i mouseTileInt = sf::Vec2i(sf::floor(mouseTile + sf::Vec2(0.5f)));

	ImGuizmo::BeginFrame();
	ImGuizmo::Enable(true);
	ImGuizmo::SetRect(0.0f, 0.0f, (float)input.resolution.x, (float)input.resolution.y);

	if (es->draggingSelection) {
		float dragT = (es->dragOrigin.y - mouseRay.origin.y) / mouseRay.direction.y;
		sf::Vec3 dragPos = rayOrigin + rayDirection * dragT;
		sf::Vec2 dragTile = sf::Vec2(dragPos.x, dragPos.z) - sf::Vec2(es->dragOrigin.x, es->dragOrigin.z);
		sf::Vec2i dragTileOffset = sf::Vec2i(sf::floor(dragTile + sf::Vec2(0.5f)));

		if (dragTileOffset != es->dragPrevOffset) {
			es->didDragSelection = true;
			es->dragPrevOffset = dragTileOffset;

			for (sv::Prop &prop : es->dragProps) {
				sv::PropTransform transform = prop.transform;
				transform.tile += dragTileOffset;

				es->svState->moveProp(es->editEvents, prop.id, transform);
			}
		}
	}

	if (!es->mouseDown && !ImGui::GetIO().WantCaptureKeyboard) {
		for (sapp_event &event : input.events) {
			if (event.type != SAPP_EVENTTYPE_KEY_DOWN) continue;
			if (event.key_repeat) continue;

			if (event.key_code == SAPP_KEYCODE_DELETE) {
				sf::Array<sf::Box<sv::Edit>> &edits = es->requests.edits.push();
				for (uint32_t svId : es->selectedSvIds) {
					if (sv::Prop *prop = es->svState->props.find(svId)) {
						auto ed = sf::box<sv::RemovePropEdit>();
						ed->propId = prop->id;
						edits.push(ed);
					}
				}
				es->selectedSvIds.clear();
			} else if (event.key_code == SAPP_KEYCODE_Z && event.modifiers == SAPP_MODIFIER_CTRL) {
				es->requests.undo = true;
			} else if ((event.key_code == SAPP_KEYCODE_Z && event.modifiers == (SAPP_MODIFIER_CTRL|SAPP_MODIFIER_SHIFT))
				|| (event.key_code == SAPP_KEYCODE_Y && event.modifiers == SAPP_MODIFIER_CTRL)) {
				es->requests.redo = true;
			}
		}
	}

	if (!es->mouseDown) {
		if (es->draggingSelection) {
			if (!es->didDragSelection) {

				if (es->dragClone) {
					for (sv::Prop &prop : es->dragProps) {
						es->svState->removeProp(es->editEvents, prop.id);
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
				sf::Array<sf::Box<sv::Edit>> &edits = es->requests.edits.push();
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

	sf::Array<cl::EntityHit> hits;
	es->clState->editorPick(hits, mouseRay);

	sf::sortBy(hits, [](const cl::EntityHit &a) { return a.t; });

	bool draggingGhostProp = false;

	if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
		const ImGuiPayload *payload = ImGui::GetDragDropPayload();
		if (payload && !strcmp(payload->DataType, "prefab")) {
			sf::Symbol prefabName = sf::Symbol((const char*)payload->Data, payload->DataSize);
			if (sf::beginsWith(prefabName, "Prefabs/Props/")) {
				if (es->svState->prefabs.find(prefabName)) {
					draggingGhostProp = true; 

					if (!es->ghostPropId) {
						sv::Prop prop;
						prop.prefabName = prefabName;
						prop.transform.tile = mouseTileInt;
						es->ghostPropId = es->svState->addProp(es->editEvents, prop, true);
						es->ghostPropPrevTile = mouseTileInt;
					} else if (es->ghostPropPrevTile != mouseTileInt) {
						es->ghostPropPrevTile = mouseTileInt;

						sv::PropTransform transform;
						transform.tile = mouseTileInt;
						es->svState->moveProp(es->editEvents, es->ghostPropId, transform);
					}

					if (!es->mouseDown && es->prevMouseDown) {
						if (sv::Prop *prop = es->svState->props.find(es->ghostPropId)) {
							sf::Array<sf::Box<sv::Edit>> &edits = es->requests.edits.push();
							auto ed = sf::box<sv::ClonePropEdit>();
							ed->localId = es->ghostPropId;
							ed->clientId = es->svState->localClientId;
							ed->prop = *prop;
							edits.push(ed);
							es->ghostPropId = 0;
						}
					}
				}
			}
		}
	}

	if (!ImGui::GetIO().WantCaptureMouse) {

		bool foundHover = false;
		for (cl::EntityHit &hit : hits) {
			Entity &entity = es->clState->systems.entities.entities[hit.entityId];
			if (entity.svId == 0) continue;
			foundHover = true;

			if (!es->draggingSelection) {
				es->clState->editorHighlight(hit.entityId, cl::EditorHighlight::Hover);
			}

			if (es->mouseDown && !es->prevMouseDown) {

				if (ImGui::IsMouseDoubleClicked(0) && !shiftDown) {
					es->selectedSvIds.clear();
					es->selectedSvIds.insert(entity.svId);
				} else {
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

						for (sv::Prop &prop : es->dragProps) {
							uint32_t svId = es->svState->addProp(es->editEvents, prop, true);
							es->selectedSvIds.insert(svId);
						}

						es->dragProps.clear();
						for (uint32_t svId : es->selectedSvIds) {
							if (sv::Prop *prop = es->svState->props.find(svId)) {
								es->dragProps.push(*prop);
							}
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

	}

	if (!draggingGhostProp) {
		if (es->ghostPropId) {
			es->svState->removeProp(es->editEvents, es->ghostPropId);
			es->ghostPropId = 0;
		}
	}

	for (uint32_t svId : es->selectedSvIds) {
		uint32_t entityId;
		sf::UintFind find = es->clState->systems.entities.svToEntity.findAll(svId);
		while (find.next(entityId)) {
			es->clState->editorHighlight(entityId, cl::EditorHighlight::Select);
		}
	}

	if (es->ghostPropId != 0) {
		uint32_t entityId;
		sf::UintFind find = es->clState->systems.entities.svToEntity.findAll(es->ghostPropId);
		while (find.next(entityId)) {
			es->clState->editorHighlight(entityId, cl::EditorHighlight::Select);
		}
	}

	handleImguiMenu(es);
	handleImguiDirectoryBrowsers(es);
	handleImguiPropertiesWindow(es);

	for (sv::Event *event : es->editEvents) {
		es->clState->applyEvent(*event);
	}
	es->editEvents.clear();
}

EditorRequests &editorPendingRequests(EditorState *es)
{
	return es->requests;
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
