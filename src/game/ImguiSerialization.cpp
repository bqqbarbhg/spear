#include "ImguiSerialization.h"

#include "ext/imgui/imgui.h"
#include "sf/Reflection.h"

void handleFieldsImgui(ImguiStatus &status, void *inst, sf::Type *type, ImguiCallback callback, void *user)
{
	char *base = (char*)inst;
	for (const sf::Field &field : type->fields) {
		handleInstImgui(status, base + field.offset, field.type, field.name, callback, user);
	}
}

void handleInstImgui(ImguiStatus &status, void *inst, sf::Type *type, const sf::CString &label, ImguiCallback callback, void *user)
{
	uint32_t flags = type->flags;
	char *base = (char*)inst;

	if (callback && callback(user, status, inst, type, label)) {
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

	} else if (flags & sf::Type::Polymorph) {
		sf::PolymorphInstance poly = type->instGetPolymorph(inst);

		if (poly.type) {
			sf::SmallStringBuf<128> typeLabel;
			typeLabel.append(label, " (", poly.type->name, ")");
			if (ImGui::TreeNode(typeLabel.data)) {
				char *polyBase = (char*)poly.inst;
				handleFieldsImgui(status, polyBase, poly.type->type, callback, user);
				ImGui::TreePop();
			}
		} else {
			ImGui::LabelText(label.data, "null");
		}

	} else if (flags & sf::Type::HasFields) {
		if (ImGui::TreeNode(label.data)) {
			handleFieldsImgui(status, base, type, callback, user);
			ImGui::TreePop();
		}
	} else if (flags & sf::Type::HasPointer) {
		void *ptr = type->instGetPointer(inst);
		if (ptr) {
			handleInstImgui(status, ptr, type->elementType, label, callback, user);
		} else {
			ImGui::LabelText(label.data, "null");
		}
	} else if (flags & sf::Type::HasArray) {
		sf::Type *elem = type->elementType;
		size_t elemSize = elem->info.size;
		sf::VoidSlice slice = type->instGetArray(inst);
		uint32_t size = (uint32_t)slice.size;

		if (ImGui::TreeNode(label.data)) {
			char *ptr = (char*)slice.data;
			sf::SmallStringBuf<16> indexLabel;
			for (uint32_t i = 0; i < size; i++) {
				indexLabel.clear();
				indexLabel.format("%u", i);
				handleInstImgui(status, ptr, elem, indexLabel, callback, user);
				ptr += elemSize;
			}

			if (flags & sf::Type::HasArrayResize) {

				bool doAdd = ImGui::Button("Add");
				ImGui::SameLine();
				bool doRemove = ImGui::Button("Remove");

				if (doAdd) {
					if (type->instArrayReserve(inst, size + 1).size) {
						type->instArrayResize(inst, size + 1);
					}
				} else if (doRemove && size > 0) {
					type->instArrayResize(inst, size - 1);
				}
			}

			ImGui::TreePop();
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
		}

		status.changed |= ImGui::IsItemDeactivatedAfterEdit();

	} else {
		// TODO: Binary serialization
	}
}
