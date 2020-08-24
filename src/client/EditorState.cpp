#include "EditorState.h"

#include "client/ClientState.h"
#include "sf/Geometry.h"
#include "sf/Sort.h"
#include "server/Message.h"

#include "ext/imgui/imgui.h"
#include "ext/imgui/ImGuizmo.h"
#include "ext/sokol/sokol_app.h"

#include "client/ImGuiExt.h"
#include "client/BSpline.h"

#include "game/ImguiSerialization.h"
#include "game/DebugDraw.h"

#include "sf/Reflection.h"

#include "sf/File.h"
#include "sp/Json.h"

#include "server/FixedPoint.h"
#include "server/ServerStateReflection.h"

#include "ext/sokol/sokol_time.h"

#include "client/AreaSystem.h"

#include "sp/Srgb.h"

namespace cl {

sf_inline sf::Vec2i tileToFixed(const sf::Vec2i &v) {
	return sf::Vec2i(v.x << 16, v.y << 16);
}

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
	sf::Array<sf::Box<sv::Event>> events;

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
	sv::Prefab editedPrefab;
	sf::HashSet<sf::Symbol> modifiedPrefabs;
	uint64_t editedPrefabDirtyTime = 0;
	bool editedPrefabInvalidated = false;

	// Ghost prefab
	sf::HashSet<sf::Symbol> preloadedPrefabNames;
	uint32_t ghostPropId = 0;
	sf::Vec2i ghostPropPrevTile;

	// Dragging / gizmo
	bool draggingSelection = false;
	bool gizmoingSelection = false;
	bool didDragSelection = false;
	sf::Vec3 dragOrigin;
	uint32_t dragSvId = 0;
	uint32_t dragRotation = 0;
	uint32_t dragPrevRotation = 0;
	uint32_t dragScale = 0x10000;
	sf::Vec2i dragPrevOffset;
	int32_t dragPrevYOffset = 0;
	bool dragSvIdAlreadySelected = false;
	bool dragShiftDown = false;
	bool dragClone = false;
	bool dragStarted = false;
	bool dragSmooth = false;
	bool dragPrevSmooth = false;
	bool dragSmoothRotate = false;
	sf::Array<sv::Prop> dragProps;
	uint32_t gizmoIndex = 0;

	// Imgui windows
	bool windowAssets = false;
	bool windowPrefabs = false;
	bool windowProperties = false;
	bool windowDebugGameState = false;
	bool windowDebugEvents = false;
	bool windowErrors = false;
	bool windowHelp = false;

	// Visualization
	bool viewAreas = false;
	
	// Folder navigation
	EditorDir dirAssets;
	EditorDir dirPrefabs;

	// File/folder creation
	sf::StringBuf addPath;
	sf::StringBuf addInput;
	bool addPrefab = false;
	bool addFolder = false;

	// Errors
	uint32_t totalErrors = 0;
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

static void selectPrefab(EditorState *es, const sf::Symbol &prefabName)
{
	if (es->selectedPrefab == prefabName) return;
	es->selectedPrefab = prefabName;
	es->editedPrefabDirtyTime = 0;
	es->editedPrefabInvalidated = false;
	sf::reset(es->editedPrefab);
}

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

	if (prefab.name.size() > 0 && prefab.name.data[0] == '<') {
		return;
	}

	jso_stream s = { };
	jso_init_file(&s, prefab.name.data);
	s.pretty = true;

	sp::writeJson(s, prefab);

	jso_close(&s);
}

static void saveModifiedPrefabs(EditorState *es)
{
	for (const sf::Symbol &prefabName : es->modifiedPrefabs) {
		if (const sv::Prefab *prefab = es->svState->prefabs.find(prefabName)) {
			savePrefab(*prefab);
		}
	}
	es->modifiedPrefabs.clear();
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
	saveModifiedPrefabs(es);

	delete es;
}

void editorPeekSokolEvent(EditorState *es, const struct sapp_event *e)
{
	if (es->draggingSelection && es->dragSmooth && e->type == SAPP_EVENTTYPE_MOUSE_DOWN && e->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {
		#ifndef SP_NO_APP
			sapp_lock_mouse(true);
		#endif
		es->dragSmoothRotate = true;
	}
	if (e->type == SAPP_EVENTTYPE_MOUSE_UP && e->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {
		es->dragSmoothRotate = false;
	}
}

bool editorPeekEventPre(EditorState *es, const sf::Box<sv::Event> &event)
{
	es->events.push(event);
	if (es->events.size > 400) {
		es->events.removeOrdered(0, 200);
	}

	if (const auto *e = event->as<sv::ReplaceLocalPropEvent>()) {
		if (e->clientId == es->svState->localClientId) {
			if (es->selectedSvIds.find(e->localId)) {
				es->selectedSvIds.remove(e->localId);
				es->selectedSvIds.insert(e->prop.id);
			}
		}
	} else if (const auto *e = event->as<sv::MakeUniquePrefabEvent>()) {
		if (e->clientId == es->svState->localClientId) {
			if (es->selectedPrefab == e->prefabName) {
				es->selectedPrefab = e->uniquePrefabName;
				es->selectedSvIds.clear();
				for (uint32_t propId : e->propIds) {
					es->selectedSvIds.insert(propId);
				}
			}
		}
	} else if (const auto *e = event->as<sv::ReloadPrefabEvent>()) {
		if (e->prefab.name == es->selectedPrefab) {
			bool edited = false;
			if (es->editedPrefabDirtyTime != 0 && stm_sec(stm_since(es->editedPrefabDirtyTime)) < 5.0) {
				edited = true;
			}

			if (!edited) {
				sf::reset(es->editedPrefab);
			} else {
				es->editedPrefabInvalidated = true;
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

void editorPreRefresh(EditorState *es)
{
	es->clState.reset();
}

void editorPostRefresh(EditorState *es, const sf::Box<cl::ClientState> &clState)
{
	es->clState = clState;
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
			if (ImGui::MenuItem("Help")) es->windowHelp = true;
			if (ImGui::MenuItem("Assets")) es->windowAssets = true;
			if (ImGui::MenuItem("Prefabs")) es->windowPrefabs = true;
			if (ImGui::MenuItem("Properties")) es->windowProperties = true;
			if (ImGui::MenuItem("Errors")) es->windowErrors = true;
			if (ImGui::BeginMenu("Debug")) {
				if (ImGui::MenuItem("Game State")) es->windowDebugGameState = true;
				if (ImGui::MenuItem("Events")) es->windowDebugEvents = true;
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Areas")) es->viewAreas = !es->viewAreas;
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

static const char *helpText = R"(
General:
- Ctrl+Z: Undo
- Ctrl+Y / Ctrl+Shift+Z: Redo
- Tab (hold): Slow motion
- F5: Reload client

Moving props:
- Left click: Select
- Shift+left click: Add/remove from multiselect
- Drag (hold left click): Move objects
- Right click while dragging: Rotate 90 degrees
- Ctrl+drag: Clone objects
- Delete: Delete selected objects
- 1/2/3: Switch between translate/rotate/scale tool (press again to disable)

Properties:
- Right click to copy/paste/reset
- Drag&drop prefabs/assets to fields

Curve/gradient editor:
- Double click background: Add new node/stop
- Double click node: Remove node/stop
)";

void handleImguiHelpWindow(EditorState *es)
{
	ImGui::SetNextWindowSize(ImVec2(600, 600), ImGuiCond_Appearing);
	if (es->windowHelp) {
		if (ImGui::Begin("Help", &es->windowHelp)) {
			ImGui::TextWrapped("%s", helpText);
		}

		ImGui::End();
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

			ImGui::SameLine();
			if (ImGui::Button("Save all")) {
				saveModifiedPrefabs(es);
			}

			handleImguiPrefabDir(es, es->dirPrefabs);
		}
		ImGui::End();
	}
}

void handleImguiErrorsWindow(EditorState *es, sf::Slice<const sf::StringBuf> errors)
{
	ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_Appearing);
	if (es->windowErrors) {
		if (ImGui::Begin("Errors", &es->windowErrors)) {
			for (uint32_t i = (uint32_t)errors.size; i > 0; i--) {
				const sf::StringBuf &err = errors[i - 1];
				ImGui::TextWrapped("> %s", err.data);
			}
		}
		ImGui::End();
	}
}

void handleImguiDebugWindows(EditorState *es)
{
	ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_Appearing);
	if (es->windowDebugGameState) {
		if (ImGui::Begin("Game State", &es->windowDebugGameState)) {
			ImguiStatus status;
			handleFieldsImgui(status, (void*)es->svState.ptr, sf::typeOf<sv::ServerState>(), nullptr, 0);
		}
		ImGui::End();
	}

	ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_Appearing);
	if (es->windowDebugEvents) {
		if (ImGui::Begin("Events", &es->windowDebugEvents)) {

			sf::Type *eventType = sf::typeOf<sv::Event>();

			ImguiStatus status;
			for (uint32_t i = es->events.size; i > 0; i--) {
				const sv::Event &event = *es->events[i - 1];
				ImGui::PushID((const void*)&event);

				sf::PolymorphInstance poly = eventType->instGetPolymorph((void*)&event);

				if (!ImGui::CollapsingHeader(poly.type->name.data, 0)) {
					ImGui::PopID();
					continue;
				}

				handleFieldsImgui(status, poly.inst, poly.type->type, NULL, NULL);

				ImGui::PopID();
			}

		}
		ImGui::End();
	}
}

static void handleCopyPasteImp(ImguiStatus &status, void *inst, sf::Type *type, const sf::CString &label, const char *tooltip)
{
	sf::SmallStringBuf<256> id;
	id.append("##copypaste-", label);
	handleInstCopyPasteImgui(status, inst, type, tooltip, id.data);
}

static bool imguiCallback(void *user, ImguiStatus &status, void *inst, sf::Type *type, const sf::CString &label, sf::Type *parentType, const char **outTooltip)
{
	EditorState *es = (EditorState*)user;
	sv::ReflectionInfo info = sv::getTypeReflectionInfo(parentType, label);
	*outTooltip = info.description;

	if (type == sf::typeOf<sv::BSpline2>()) {
		sv::BSpline2 &spline = *(sv::BSpline2*)inst;

		bool open = ImGui::TreeNode(label.data);

		handleCopyPasteImp(status, inst, type, label, info.description);

		if (open) {

			spline.points.reserveGeometric(spline.points.size + 1);
			int num = (int)spline.points.size;
			ImEditState editState = cl::ImBSplineEditor("##spline", ImVec2(0.0f, 200.0f), spline.points.data->v, &num, (int)spline.points.capacity);
			spline.points.size = (uint32_t)num;

			if (editState == ImEditState::Changed) {
				status.modified = true;
				status.changed = true;
			} else if (editState == ImEditState::Dragging) {
				status.modified = true;
			}

			ImGui::TreePop();
		}

		return true;
	} else if (type == sf::typeOf<sv::Gradient>()) {
		sv::Gradient &gradient = *(sv::Gradient*)inst;

		bool open = ImGui::TreeNode(label.data);

		handleCopyPasteImp(status, inst, type, label, info.description);

		if (open) {

			int selected = -1;

			gradient.points.reserveGeometric(gradient.points.size + 1);
			int num = (int)gradient.points.size;
			ImEditState editState = cl::ImGradientEditor("##gradient", ImVec2(0.0f, 20.0f), (float*)gradient.points.data,
				&num, (int)gradient.points.capacity, &selected, gradient.defaultColor.v);
			gradient.points.size = (uint32_t)num;

			sf::Vec3 *color = &gradient.defaultColor;
			if (selected >= 0 && selected < (int)gradient.points.size) {
				color = &gradient.points[selected].color;
			}

			status.modified |= ImGui::ColorPicker3("##color", color->v, ImGuiColorEditFlags_NoSmallPreview|ImGuiColorEditFlags_NoLabel);
			status.changed |= ImGui::IsItemDeactivatedAfterEdit();

			if (editState == ImEditState::Changed) {
				status.changed = true;
			} else if (editState == ImEditState::Dragging) {
				status.modified = true;
			}

			ImGui::TreePop();
		}

		return true;
	} else if (type == sf::typeOf<sf::Vec2>()) {
		sf::Vec2 &vec = *(sf::Vec2*)inst;

		status.modified |= ImGui::InputFloat2(label.data, vec.v, 4);
		status.changed |= ImGui::IsItemDeactivatedAfterEdit();

		handleCopyPasteImp(status, inst, type, label, info.description);

		return true;
	} else if (type == sf::typeOf<sf::Vec3>()) {
		sf::Vec3 &vec = *(sf::Vec3*)inst;

		if (info.color) {
			status.modified |= ImGui::ColorEdit3(label.data, vec.v, 0);
			status.changed |= ImGui::IsItemDeactivatedAfterEdit();
			handleCopyPasteImp(status, inst, type, label, info.description);
		} else {
			status.modified |= ImGui::InputFloat3(label.data, vec.v, 4);
			status.changed |= ImGui::IsItemDeactivatedAfterEdit();
		}

		handleCopyPasteImp(status, inst, type, label, info.description);

		return true;
	} else if (type == sf::typeOf<sf::Vec2i>()) {
		sf::Vec2i &vec = *(sf::Vec2i*)inst;

		if (info.fixedBits) {
			double v[2];
			double scale = (double)(1u << (uint32_t)info.fixedBits);
			v[0] = (double)vec.x / scale;
			v[1] = (double)vec.y / scale;

			if (ImGui::InputScalarN(label.data, ImGuiDataType_Double, v, 2, NULL, NULL, "%.4f")) {
				vec.x = (int32_t)sf::clamp(v[0] * scale, (double)INT32_MIN, (double)INT32_MAX);
				vec.y = (int32_t)sf::clamp(v[1] * scale, (double)INT32_MIN, (double)INT32_MAX);
				status.modified = true;
			}
			status.changed |= ImGui::IsItemDeactivatedAfterEdit();

			handleCopyPasteImp(status, inst, type, label, info.description);

		} else {
			status.modified |= ImGui::InputInt2(label.data, vec.v);
			status.changed |= ImGui::IsItemDeactivatedAfterEdit();
			handleCopyPasteImp(status, inst, type, label, info.description);
		}

		return true;
	} else if (type == sf::typeOf<sf::Vec3i>()) {
		sf::Vec3i &vec = *(sf::Vec3i*)inst;

		status.modified |= ImGui::InputInt3(label.data, vec.v);
		status.changed |= ImGui::IsItemDeactivatedAfterEdit();
		handleCopyPasteImp(status, inst, type, label, info.description);

		return true;
	} else if (type == sf::typeOf<uint8_t[3]>()) {
		uint8_t *src = (uint8_t*)inst;

		if (info.color) {

			float colorF[3] = { (float)src[0] / 255.0f, (float)src[1] / 255.0f, (float)src[2] / 255.0f };
			if (ImGui::ColorEdit3(label.data, colorF, 0)) {
				src[0] = (uint8_t)sf::clamp(colorF[0] * 255.0f, 0.0f, 255.0f);
				src[1] = (uint8_t)sf::clamp(colorF[1] * 255.0f, 0.0f, 255.0f);
				src[2] = (uint8_t)sf::clamp(colorF[2] * 255.0f, 0.0f, 255.0f);
				status.modified = true;
			}
			status.changed |= ImGui::IsItemDeactivatedAfterEdit();
			handleCopyPasteImp(status, inst, type, label, info.description);

			return true;
		}
	} else if (type == sf::typeOf<sf::Symbol>()) {
		sf::Symbol &sym = *(sf::Symbol*)inst;

		if (info.asset || info.prefab) {

			sf::SmallStringBuf<4096> textBuf;
			textBuf.reserveGeometric(sym.size() * 2);
			textBuf.append(sym);
			if (ImGui::InputText(label.data, textBuf.data, textBuf.capacity, ImGuiInputTextFlags_AlignRight | ImGuiInputTextFlags_AutoSelectAll)) {
				status.modified = true;
				textBuf.resize(strlen(textBuf.data));
				if (ImGui::IsItemDeactivatedAfterEdit()) {
					sym = sf::Symbol(textBuf);
					status.changed = true;
				}
			}
			handleCopyPasteImp(status, inst, type, label, info.description);

			if (info.prefab && ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0)) {
				selectPrefab(es, sym);
			}

			if (ImGui::BeginDragDropTarget()) {
				const char *key = NULL;
				if (info.asset) key = "asset";
				if (info.prefab) key = "prefab";
				const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(key);
				if (payload) {
					sym = sf::Symbol((const char*)payload->Data, payload->DataSize);
					status.modified = true;
					status.changed = true;
				}

				ImGui::EndDragDropTarget();
			}

			return true;

		} else if (info.multiline) {

			sf::SmallStringBuf<4096> textBuf;
			textBuf.reserveGeometric(sym.size() * 2);
			textBuf.append(sym);
			if (ImGui::InputTextMultiline(label.data, textBuf.data, textBuf.capacity, ImVec2(0.0f, 200.0f))) {
				status.modified = true;
				textBuf.resize(strlen(textBuf.data));
				if (ImGui::IsItemDeactivatedAfterEdit()) {
					sym = sf::Symbol(textBuf);
					status.changed = true;
				}
			}
			handleCopyPasteImp(status, inst, type, label, info.description);

			return true;
		}
	}

	return false;
}

void handleImguiPrefab(EditorState *es, ImguiStatus &status, sv::Prefab &prefab)
{
	ImGui::Text("%s", prefab.name.data);

	uint32_t numProps = 0;
	uint32_t numSelectedProps = 0;
	if (const sf::UintSet *propIds = es->svState->prefabProps.findValue(prefab.name)) {
		for (uint32_t propId : *propIds) {
			if (sv::isIdLocal(propId)) continue;
			numProps++;
			if (es->selectedSvIds.find(propId)) {
				numSelectedProps++;
			}
		}
	}

	if (ImGui::Button("Make Unique")) {
		sf::Array<sf::Box<sv::Edit>> &edits = es->requests.edits.push();

		auto ed = sf::box<sv::MakeUniquePrefabEdit>();
		ed->prefabName = prefab.name;
		ed->clientId = es->svState->localClientId;

		if (const sf::UintSet *propIds = es->svState->prefabProps.findValue(prefab.name)) {
			for (uint32_t propId : *propIds) {
				if (sv::isIdLocal(propId)) continue;
				numProps++;
				if (es->selectedSvIds.find(propId)) {
					ed->propIds.push(propId);
				}
			}
		}

		edits.push(ed);
	}
	ImGui::SameLine();
	ImGui::Text("%u/%u props", numSelectedProps, numProps);

	sf::Type *componentType = sf::typeOf<sv::Component>();

	for (uint32_t compI = 0; compI < prefab.components.size; compI++) {
		ImGui::PushID(compI);

		sf::PolymorphInstance poly = componentType->instGetPolymorph(prefab.components[compI].ptr);

		if (!ImGui::CollapsingHeader(poly.type->name.data, ImGuiTreeNodeFlags_DefaultOpen)) {
			handleInstCopyPasteImgui(status, poly.inst, poly.type->type, NULL);
			ImGui::PopID();
			continue;
		}
		handleInstCopyPasteImgui(status, poly.inst, poly.type->type, NULL);

		bool doDelete = ImGui::Button("Delete");

		handleFieldsImgui(status, poly.inst, poly.type->type, imguiCallback, es);

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

	sf::sort(names);

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

				if (es->editedPrefabDirtyTime != 0 && stm_sec(stm_since(es->editedPrefabDirtyTime)) >= 10.0 && es->editedPrefabInvalidated) {
					sf::reset(es->editedPrefab);
				}

				if (es->editedPrefab.name != es->selectedPrefab) {
					sf::reset(es->editedPrefab);
					es->prefabCopyBuffer.clear();
					sf::writeBinary(es->prefabCopyBuffer, *sourcePrefab);
                    sf::Slice<const char> input = es->prefabCopyBuffer.slice();
					sf::readBinary(input, es->editedPrefab);
				}

				ImguiStatus status;
				handleImguiPrefab(es, status, es->editedPrefab);

				if (status.modified) {
					es->editedPrefabDirtyTime = stm_now();
				}

				if (status.changed) {
					sf::Array<sf::Box<sv::Edit>> &edits = es->requests.edits.push();
					auto ed = sf::box<sv::ModifyPrefabEdit>();
					ed->prefab = es->editedPrefab;
					edits.push(ed);
					es->modifiedPrefabs.insert(es->selectedPrefab);
					es->editedPrefabDirtyTime = 0;
					es->editedPrefabInvalidated = false;
				}
			}
		}
		ImGui::End();
	}
}

static void prepareDragTransform(EditorState *es, int32_t &dragCos, int32_t &dragSin)
{
	if (es->dragRotation > 0) {
		if ((es->dragRotation & ((1<<6) - 1)) == 0 || true) {
			dragCos = sv::fixedCos360(es->dragRotation >> 6);
			dragSin = sv::fixedSin360(es->dragRotation >> 6);
		} else {
			double c = cos((double)es->dragRotation * (1.0/64.0 * sf::D_PI/180.0));
			double s = sin((double)es->dragRotation * (1.0/64.0 * sf::D_PI/180.0));
			dragCos = sf::clamp((int32_t)(c * 65536.0), -0x10000, 0x10000);
			dragSin = sf::clamp((int32_t)(s * 65536.0), -0x10000, 0x10000);
		}
	} else {
		dragCos = 0x10000;
		dragSin = 0;
	}
}

static bool applyDragTransform(EditorState *es, sv::PropTransform &transform, int32_t dragCos, int32_t dragSin, bool snap)
{
	sv::PropTransform prev = transform;

	transform.position += es->dragPrevOffset;
	transform.offsetY += es->dragPrevYOffset;
	transform.rotation = (transform.rotation + es->dragRotation) % (360<<6);
	transform.scale = (uint16_t)(sf::clamp(sv::fixedMul((int32_t)transform.scale << 8, es->dragScale) >> 8, 0, 0xffff));

	if (es->dragRotation != 0 || es->dragScale != 0x10000) {
		sf::Vec2i dragStartOrigin = sf::Vec2i(sf::Vec2(es->dragOrigin.x, es->dragOrigin.z) * 65536.0f);
		sf::Vec2i rotateOrigin = dragStartOrigin + es->dragPrevOffset;

		sf::Vec2i delta = transform.position - rotateOrigin;
		sf::Vec2i newDelta;
		newDelta.x = sv::fixedMul(delta.x, dragCos) + sv::fixedMul(delta.y, dragSin);
		newDelta.y = sv::fixedMul(delta.x, -dragSin) + sv::fixedMul(delta.y, dragCos);

		newDelta.x = sv::fixedMul(newDelta.x, es->dragScale);
		newDelta.y = sv::fixedMul(newDelta.y, es->dragScale);

		transform.position = rotateOrigin + newDelta;
	}

	if (snap) {
		transform.position.x = (transform.position.x + 0x7fff) & ~0xffff;
		transform.position.y = (transform.position.y + 0x7fff) & ~0xffff;
	}

	return memcmp(&transform, &prev, sizeof(sv::PropTransform)) != 0;
}

void editorUpdate(EditorState *es, const FrameArgs &frameArgs, const ClientInput &input, const EditorInput &editorInput)
{
	for (uint32_t i = 0; i < es->selectedSvIds.size(); i++) {
		uint32_t svId = es->selectedSvIds.data[i];
		if (!es->svState->isIdValid(svId)) {
			es->selectedSvIds.remove(svId);
			i--;
		}
	}

	if (editorInput.totalErrors > es->totalErrors) {
		es->totalErrors = editorInput.totalErrors;
		es->windowErrors = true;
	}

	es->prevMouseDown = es->mouseDown;
	es->mouseDown = ImGui::GetIO().MouseDown[0];

	bool shiftDown = ImGui::IsKeyDown(SAPP_KEYCODE_LEFT_SHIFT);
	bool ctrlDown = ImGui::IsKeyDown(SAPP_KEYCODE_LEFT_CONTROL);

	sf::Mat44 clipToWorld = sf::inverse(frameArgs.mainRenderArgs.worldToClip);
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
		es->dragPrevSmooth = es->dragSmooth;
		es->dragSmooth = !ImGui::GetIO().WantCaptureKeyboard && ImGui::IsKeyDown(SAPP_KEYCODE_F);
	}

	if (es->draggingSelection) {
		float dragT = (es->dragOrigin.y - mouseRay.origin.y) / mouseRay.direction.y;
		sf::Vec3 dragPos = rayOrigin + rayDirection * dragT;
		sf::Vec2 dragTile = sf::Vec2(dragPos.x, dragPos.z) - sf::Vec2(es->dragOrigin.x, es->dragOrigin.z);
		sf::Vec2i dragTileOffset;

		dragTileOffset = sf::Vec2i(sf::floor(dragTile * 65536.0f + sf::Vec2(0.5f)));

		if (es->dragSmoothRotate) {
			for (sapp_event &event : input.events) {
				if (event.type == SAPP_EVENTTYPE_MOUSE_MOVE) {
					if (event.mouse_dx) {
						int32_t rotation = (int32_t)es->dragRotation + (int32_t)(event.mouse_dx * 10.0f);
						while (rotation < 0) {
							rotation += (360 << 6);
						}
						es->dragRotation = (uint32_t)rotation % (360 << 6);
					}
				}
			}
		} else {
			if (ImGui::GetIO().MouseClicked[1]) {
				es->dragRotation = (es->dragRotation + (270<<6)) % (360<<6);
				es->dragStarted = true;
			}
		}

		if (ImGui::GetIO().MouseDownDuration[0] > 0.4f || sf::length(dragTile) > 0.2f) {
			es->dragStarted = true;
		}

		if ((dragTileOffset != es->dragPrevOffset || es->dragRotation != es->dragPrevRotation || es->dragSmooth != es->dragPrevSmooth) && es->dragStarted) {
			es->didDragSelection = true;
			es->dragPrevOffset = dragTileOffset;
			es->dragPrevRotation = es->dragRotation;

			int32_t dragCos, dragSin;
			prepareDragTransform(es, dragCos, dragSin);

			for (sv::Prop &prop : es->dragProps) {
				sv::PropTransform transform = prop.transform;

				applyDragTransform(es, transform, dragCos, dragSin, !es->dragSmooth);

				sv::Prop *svProp = es->svState->props.find(prop.id);
				if (!svProp || memcmp(&transform, &svProp->transform, sizeof(sv::PropTransform)) != 0) {
					es->svState->moveProp(es->editEvents, prop.id, transform);
				}
			}
		}
	} else {

		if (es->gizmoIndex > 0) {
			sf::Vec3 selectionMidpoint;

			sf::SmallArray<sv::Prop*, 64> gizmoProps;
			for (uint32_t svId : es->selectedSvIds) {
				if (sv::Prop *prop = es->svState->props.find(svId)) {
					sf::Vec3 pos = sf::Vec3((float)prop->transform.position.x, (float)prop->transform.offsetY, (float)prop->transform.position.y) * (1.0f/65536.0f);
					selectionMidpoint += pos;
					gizmoProps.push(prop);
				}
			}

			selectionMidpoint /= (float)gizmoProps.size;

			if (es->gizmoIndex != 1 && es->gizmoingSelection) {
				selectionMidpoint = es->dragOrigin;
			}

			sf::Mat44 gizmoToWorld = sf::mat::translate(selectionMidpoint);
			const sf::Mat44 &view = frameArgs.mainRenderArgs.worldToView;
			const sf::Mat44 &proj = frameArgs.mainRenderArgs.viewToClip;
			sf::Mat44 delta;
			sf::Mat44 identity;

			if (es->gizmoIndex == 1) {
				ImGuizmo::Manipulate(view.v, proj.v, ImGuizmo::TRANSLATE, ImGuizmo::WORLD, gizmoToWorld.v, delta.v);
			} else if (es->gizmoIndex == 2) {
				ImGuizmo::Manipulate(view.v, proj.v, ImGuizmo::ROTATE, ImGuizmo::WORLD, gizmoToWorld.v, delta.v);
			} else if (es->gizmoIndex == 3) {
				ImGuizmo::Manipulate(view.v, proj.v, ImGuizmo::SCALE, ImGuizmo::WORLD, gizmoToWorld.v, delta.v);
			}

			if (memcmp(&delta, &identity, sizeof(sf::Mat44)) != 0) {
				if (!es->gizmoingSelection) {
					es->dragProps.clear();
					for (sv::Prop *pProp : gizmoProps) {
						es->dragProps.push(*pProp);
					}
				}

				es->gizmoingSelection = true;
				es->didDragSelection = true;
				es->dragSmooth = true;
				es->dragOrigin = selectionMidpoint;

				sf::Vec3 translation, rotation, scale;
				ImGuizmo::DecomposeMatrixToComponents(delta.v, translation.v, rotation.v, scale.v);

				if (es->gizmoIndex == 1) {
					es->dragPrevOffset += sf::Vec2i(sf::Vec2(translation.x, translation.z) * 65536.0f);
					es->dragPrevYOffset += (int32_t)(delta.m13 * 65536.0f);
				} else if (es->gizmoIndex == 2) {
					int32_t rot = (int32_t)es->dragRotation + (int32_t)(rotation.y * 64.0f);
					while (rot < 0) {
						rot += (360 << 6);
					}
					es->dragRotation = (uint32_t)rot % (360 << 6);
				} else if (es->gizmoIndex == 3) {
					float scaleDelta = 1.0f;
					if (scale.x != 1.0f) {
						scaleDelta = scale.x;
					} else if (scale.y != 1.0f) {
						scaleDelta = scale.y;
					} else if (scale.z != 1.0f) {
						scaleDelta = scale.z;
					}

					es->dragScale = (uint32_t)(scaleDelta * 65536.0f);
				}

				es->dragPrevRotation = es->dragRotation;

				int32_t dragCos, dragSin;
				prepareDragTransform(es, dragCos, dragSin);

				for (sv::Prop &prop : es->dragProps) {
					sv::PropTransform transform = prop.transform;

					applyDragTransform(es, transform, dragCos, dragSin, !es->dragSmooth);

					es->svState->moveProp(es->editEvents, prop.id, transform);
				}

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
			} else if (event.key_code == SAPP_KEYCODE_1) {
				es->gizmoIndex = (es->gizmoIndex != 1) ? 1 : 0;
			} else if (event.key_code == SAPP_KEYCODE_2) {
				es->gizmoIndex = (es->gizmoIndex != 2) ? 2 : 0;
			} else if (event.key_code == SAPP_KEYCODE_3) {
				es->gizmoIndex = (es->gizmoIndex != 3) ? 3 : 0;
			}
		}
	}

	if (!es->mouseDown) {
		if (es->draggingSelection || es->gizmoingSelection) {
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
				int32_t dragCos, dragSin;
				prepareDragTransform(es, dragCos, dragSin);

				sf::Array<sf::Box<sv::Edit>> &edits = es->requests.edits.push();
				if (es->dragClone) {
					for (sv::Prop &prop : es->dragProps) {
						auto ed = sf::box<sv::ClonePropEdit>();
						ed->clientId = es->svState->localClientId;
						ed->localId = prop.id;
						ed->prop = prop;
						applyDragTransform(es, ed->prop.transform, dragCos, dragSin, !es->dragSmooth);
						edits.push(ed);
					}
				} else {
					for (sv::Prop &prop : es->dragProps) {
						auto ed = sf::box<sv::MovePropEdit>();
						ed->propId = prop.id;
						ed->transform = prop.transform;
						applyDragTransform(es, ed->transform, dragCos, dragSin, !es->dragSmooth);
						edits.push(ed);
					}
				}
			}
		}

		es->draggingSelection = false;
		es->gizmoingSelection = false;
		es->didDragSelection = false;
		es->dragSmooth = false;
		es->dragStarted = false;
		es->dragPrevOffset = sf::Vec2i();
		es->dragPrevYOffset = 0;
		es->dragRotation = 0;
		es->dragPrevRotation = 0;
		es->dragScale = 0x10000;
	}

	sf::Array<cl::EntityHit> hits;
	es->clState->editorPick(hits, mouseRay);

	if (!ImGui::IsKeyDown(SAPP_KEYCODE_G)) {
		for (uint32_t i = 0; i < hits.size; i++) {
			if (hits[i].approximate) {
				hits.removeSwap(i--);
			}
		}
	}

	sf::sortBy(hits, [](const cl::EntityHit &a) { return a.t; });

	bool draggingGhostProp = false;

	if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
		const ImGuiPayload *payload = ImGui::GetDragDropPayload();
		if (payload && !strcmp(payload->DataType, "prefab")) {
			sf::Symbol prefabName = sf::Symbol((const char*)payload->Data, payload->DataSize);
			if (sf::beginsWith(prefabName, "Prefabs/Props/")) {
				if (es->svState->prefabs.find(prefabName)) {

					if (es->mouseDown) {
						draggingGhostProp = true; 
						if (!es->ghostPropId) {
							sv::Prop prop;
							prop.prefabName = prefabName;
							prop.transform.position = tileToFixed(mouseTileInt);
							es->ghostPropId = es->svState->addProp(es->editEvents, prop, true);
							es->ghostPropPrevTile = mouseTileInt;
						} else if (es->ghostPropPrevTile != mouseTileInt) {
							es->ghostPropPrevTile = mouseTileInt;

							sv::PropTransform transform;
							transform.position = tileToFixed(mouseTileInt);
							es->svState->moveProp(es->editEvents, es->ghostPropId, transform);
						}
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
			} else if (sf::beginsWith(prefabName, "Prefabs/Characters/")) {
				if (es->svState->prefabs.find(prefabName)) {

					if (!es->mouseDown && es->prevMouseDown) {
						sf::Array<sf::Box<sv::Edit>> &edits = es->requests.edits.push();
						auto ed = sf::box<sv::AddCharacterEdit>();
						ed->character.tile = mouseTileInt;
						ed->character.prefabName = prefabName;
						edits.push(ed);
						es->ghostPropId = 0;
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

				if (entity.prefabId != ~0u) {
					es->selectedPrefab = es->clState->systems.entities.prefabs[entity.prefabId].svPrefab->name;
				}

				if (ImGui::IsMouseDoubleClicked(0) && !shiftDown) {
					es->selectedSvIds.clear();
					es->selectedSvIds.insert(entity.svId);
					es->windowProperties = true;
				} else {
					es->dragSvId = entity.svId;
					es->dragSvIdAlreadySelected = (es->selectedSvIds.find(entity.svId) != nullptr);
					es->dragOrigin = mouseRay.origin + mouseRay.direction * hit.t;
					es->draggingSelection = true;
					es->dragPrevSmooth = es->dragSmooth;
					es->didDragSelection = false;
					es->dragStarted = false;
					es->dragRotation = 0;
					es->dragPrevRotation = 0;
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

			Entity &entity = es->clState->systems.entities.entities[entityId];
			Prefab &prefab = es->clState->systems.entities.prefabs[entity.prefabId];
			for (const sv::Component *comp : prefab.svPrefab->components) {
				if (const auto *c = comp->as<sv::TileAreaComponent>()) {
					sf::Vec2 min = sf::Vec2(c->minCorner) * (1.0f / 65536.0f) - sf::Vec2(0.45f);
					sf::Vec2 max = sf::Vec2(c->maxCorner) * (1.0f / 65536.0f) + sf::Vec2(0.45f);
					Transform transform = entity.transform;
					transform.position.y = 0.0f;
					debugDrawBox(sf::Bounds3::minMax(sf::Vec3(min.x, 0.0f, min.y), sf::Vec3(max.x, 0.1f, max.y)),
						transform.asMatrix());
				}
			}
		}
	}

	for (uint32_t svId : es->selectedSvIds) {
		uint32_t packedTile;
		sf::UintFind find = es->svState->getEntityPackedTiles(svId);
		while (find.next(packedTile)) {
			sf::Vec2i tile = sv::unpackTile(packedTile);
			sf::Bounds3 bounds;
			bounds.origin = sf::Vec3((float)tile.x, 0.0f, (float)tile.y);
			bounds.extent = sf::Vec3(0.95f, 0.1f, 0.95f) / 2.0f;
			debugDrawBox(bounds, sf::Vec3(1.0f, 0.0f, 0.0f));
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
	handleImguiHelpWindow(es);
	handleImguiDirectoryBrowsers(es);
	handleImguiPropertiesWindow(es);
	handleImguiErrorsWindow(es, editorInput.errors);
	handleImguiDebugWindows(es);

	for (sv::Event *event : es->editEvents) {
		es->clState->applyEventImmediate(*event);
	}
	es->editEvents.clear();

	if (es->viewAreas) {
		sf::Array<cl::AreaBounds> areas;
		es->clState->systems.area->queryFrustumBounds(areas, ~0u, frameArgs.mainRenderArgs.frustum);

		const sf::Vec3 areaColors[] = {
			sp::srgbToLinearHex(0xffd5e5),
			sp::srgbToLinearHex(0xffffdd),
			sp::srgbToLinearHex(0xa0ffe6),
			sp::srgbToLinearHex(0x81f5ff),
		};

		for (cl::AreaBounds &bounds : areas) {
			sf::Vec3 color = sf::Vec3(1.0f);
			uint32_t group = (uint32_t)bounds.area.group;
			if (group < sf_arraysize(areaColors)) {
				color = areaColors[group];
			}
			debugDrawBox(bounds.bounds, color);
		}
	}
}

EditorRequests &editorPendingRequests(EditorState *es)
{
	return es->requests;
}

}
