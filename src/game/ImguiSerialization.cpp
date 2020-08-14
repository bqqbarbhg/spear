#include "ImguiSerialization.h"

#include "ext/imgui/imgui.h"
#include "sf/Reflection.h"

#include "sp/Json.h"

void handleInstCopyPasteImgui(ImguiStatus &status, void *inst, sf::Type *type, const char *id)
{
	if (ImGui::BeginPopupContextItem(id)) {

		if (ImGui::MenuItem("Copy")) {
			sf::SmallArray<char, 2048> json;

			jso_stream s;
			sp::jsoInitArray(&s, json);
			s.pretty = true;

			sp::writeInstJson(s, inst, type);

			jso_close(&s);

			json.push('\0');
			ImGui::SetClipboardText(json.data);
		} else if (ImGui::MenuItem("Paste")) {

			const char *clipboard = ImGui::GetClipboardText();
			jsi_args args = { };
			args.dialect.allow_bare_keys = true;
			args.dialect.allow_comments = true;
			args.dialect.allow_control_in_string = true;
			args.dialect.allow_missing_comma = true;
			args.dialect.allow_trailing_comma = true;
			jsi_value *v = jsi_parse_string(clipboard, &args);
			if (v) {

				sf::SmallArray<char, 1024> copy;
				copy.resizeUninit(type->info.size);
				type->info.constructRange(copy.data, 1);
				if (sp::readInstJson(v, copy.data, type)) {
					type->info.destructRange(inst, 1);
					type->info.moveRange(inst, copy.data, 1);
					status.modified = true;
					status.changed = true;
				}

				jsi_free(v);
			}

		} else if (ImGui::MenuItem("Reset")) {
			type->info.destructRange(inst, 1);
			type->info.constructRange(inst, 1);
			status.modified = true;
			status.changed = true;
		}

		ImGui::EndPopup();
	}
}

void handleFieldsImgui(ImguiStatus &status, void *inst, sf::Type *type, ImguiCallback callback, void *user)
{
	char *base = (char*)inst;
	for (const sf::Field &field : type->fields) {
		handleInstImgui(status, base + field.offset, field.type, field.name, type, callback, user);
	}
}

void handleInstImgui(ImguiStatus &status, void *inst, sf::Type *type, const sf::CString &label, sf::Type *parentType, ImguiCallback callback, void *user)
{
	uint32_t flags = type->flags;
	char *base = (char*)inst;

	if (callback && callback(user, status, inst, type, label, parentType)) {
		return;
	}

	if (flags & sf::Type::HasString) {
		sf::VoidSlice slice = type->instGetArray(inst);

		sf::SmallStringBuf<4096> textBuf;
		textBuf.reserve(slice.size * 2);
		textBuf.append(sf::String((const char*)slice.data, slice.size));

		if (ImGui::InputText(label.data, textBuf.data, textBuf.capacity)) {
			textBuf.resize(strlen(textBuf.data));
			if (ImGui::IsItemDeactivatedAfterEdit() && (flags & sf::Type::HasSetString) != 0) {
				type->instSetString(inst , textBuf);
				status.changed = true;
			}
		}
		handleInstCopyPasteImgui(status, inst, type);

	} else if (flags & sf::Type::Polymorph) {
		sf::PolymorphInstance poly = type->instGetPolymorph(inst);

		if (poly.type) {
			sf::SmallStringBuf<128> typeLabel;
			typeLabel.append(label, " (", poly.type->name, ")");

			bool open = ImGui::TreeNode(typeLabel.data);
			handleInstCopyPasteImgui(status, inst, type);
			if (open) {
				char *polyBase = (char*)poly.inst;
				handleFieldsImgui(status, polyBase, poly.type->type, callback, user);
				ImGui::TreePop();
			}
		} else {
			ImGui::LabelText(label.data, "null");
		}

	} else if (flags & sf::Type::HasFields) {
		bool open = ImGui::TreeNode(label.data);
		handleInstCopyPasteImgui(status, inst, type);
		if (open) {
			handleFieldsImgui(status, base, type, callback, user);
			ImGui::TreePop();
		}
	} else if (flags & sf::Type::HasPointer) {
		void *ptr = type->instGetPointer(inst);
		if (ptr) {
			handleInstImgui(status, ptr, type->elementType, label, parentType, callback, user);
		} else {
			ImGui::LabelText(label.data, "null");
		}
	} else if (flags & sf::Type::HasArray) {
		sf::Type *elem = type->elementType;
		size_t elemSize = elem->info.size;

		sf::Array<char> scratch;
		sf::VoidSlice slice = type->instGetArray(inst, &scratch);
		uint32_t size = (uint32_t)slice.size;

		bool open = ImGui::TreeNode(label.data);
		handleInstCopyPasteImgui(status, inst, type);
		if (open) {
			char *ptr = (char*)slice.data;

			uint32_t removeIndex = ~0u;
			uint32_t swapIndex = ~0u;
			bool add = false;

			sf::SmallStringBuf<16> indexLabel;
			for (uint32_t i = 0; i < size; i++) {
				ImGui::PushID((int)i);

				if (flags & sf::Type::HasArrayResize) {
					if (ImGui::Button("x")) {
						removeIndex = i;
					}
					ImGui::SameLine();
					if (ImGui::Button("^")) {
						if (i > 0) swapIndex = i - 1;
					}
					ImGui::SameLine();
					if (ImGui::Button("v")) {
						if (i + 1 < size) swapIndex = i;
					}
					ImGui::SameLine();
				}

				indexLabel.clear();
				indexLabel.format("%u", i);
				handleInstImgui(status, ptr, elem, indexLabel, parentType, callback, user);
				ptr += elemSize;

				ImGui::PopID();
			}

			if (flags & sf::Type::HasArrayResize) {
				if (ImGui::Button("+")) {
					add = true;
				}
			}

			if (removeIndex != ~0u || swapIndex != ~0u || add) {

				size_t esz = elem->info.size;
				sf::Array<char> copy;
				uint32_t newSize = (uint32_t)slice.size;
				if (add) newSize += 1;
				else if (removeIndex != ~0u) newSize -= 1;

				copy.resizeUninit(newSize * elem->info.size);

				if (add) {
					elem->info.moveRange(copy.data, slice.data, slice.size);
					elem->info.constructRange((char*)copy.data + slice.size*esz, 1);
				} else if (removeIndex != ~0u) {
					elem->info.moveRange(copy.data, slice.data, removeIndex);
					elem->info.destructRange((char*)slice.data + removeIndex*esz, 1);
					elem->info.moveRange(copy.data + removeIndex*esz, (char*)slice.data + (removeIndex+1)*esz, slice.size - removeIndex - 1);
				} else if (swapIndex != ~0u) {
					elem->info.moveRange(copy.data, slice.data, swapIndex);
					elem->info.moveRange(copy.data + (swapIndex+1)*esz, (char*)slice.data + swapIndex*esz, 1);
					elem->info.moveRange(copy.data + swapIndex*esz, (char*)slice.data + (swapIndex+1)*esz, 1);
					elem->info.moveRange(copy.data + (swapIndex+2)*esz, (char*)slice.data + (swapIndex+2)*esz, slice.size - swapIndex - 2);
				}
				elem->info.constructRange(slice.data, slice.size);

				if (scratch.size > 0) {
					elem->info.destructRange(scratch.data, scratch.size / elem->info.size);
					scratch.clear();
				}

				sf::VoidSlice newSlice = type->instArrayReserve(inst, newSize, &scratch);
				elem->info.destructRange(newSlice.data, newSlice.size);
				elem->info.moveRange(newSlice.data, copy.data, newSize);
				type->instArrayResize(inst, newSize, newSlice);

				status.modified = true;
				status.changed = true;
			}

			ImGui::TreePop();
		}

		if (scratch.size > 0) {
			elem->info.destructRange(scratch.data, scratch.size / elem->info.size);
		}

	} else if (flags & sf::Type::IsPrimitive) {
		ImGuiDataType dataType = -1;
		switch (type->primitive) {
		case sf::Type::Bool: ImGui::Checkbox(label.data, (bool*)inst); break;
		case sf::Type::Char: dataType = ImGuiDataType_U8; break;
		case sf::Type::I8: dataType = ImGuiDataType_S8; break;
		case sf::Type::I16: dataType = ImGuiDataType_S16; break;
		case sf::Type::I32: dataType = ImGuiDataType_S32; break;
		case sf::Type::I64: dataType = ImGuiDataType_S64; break;
		case sf::Type::U8: dataType = ImGuiDataType_U8; break;
		case sf::Type::U16: dataType = ImGuiDataType_U16; break;
		case sf::Type::U32: dataType = ImGuiDataType_U32; break;
		case sf::Type::U64: dataType = ImGuiDataType_U64; break;
		case sf::Type::F32: dataType = ImGuiDataType_Float; break;
		case sf::Type::F64: dataType = ImGuiDataType_Double; break;
		}

		if (dataType >= 0) {
			ImGui::InputScalar(label.data, dataType, inst);
			handleInstCopyPasteImgui(status, inst, type);
		}

		status.changed |= ImGui::IsItemDeactivatedAfterEdit();

	} else {
		// TODO: Binary serialization
	}
}
