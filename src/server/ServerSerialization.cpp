#include "ServerSerialization.h"

namespace sv2 {

bool isObjectType(sf::Type *type)
{
	static sf::Type *objectType = sf::typeOf<sv2::Object>();
	while (type) {
		if (type == objectType) return true;
		type = type->baseType;
	}
	return false;
}

void serializeInstBinary(sf::Array<char> &dst, SerializationState &state, void *inst, sf::Type *type)
{
	if (type->baseType) {
		serializeInstBinary(dst, state, inst, type->baseType);
	}

	static sf::Type *objectType = sf::typeOf<sv2::Object>();
	uint32_t flags = type->flags;
	char *base = (char*)inst;
	if (flags & sf::Type::IsPod) {
		dst.push(base, type->info.size);
	} else if ((flags & sf::Type::HasPointer) && isObjectType(type->elementType)) {
		sf::Box<sv2::Object> &box = *(sf::Box<sv2::Object>*)inst;
		if (!box.ptr) {
			uint64_t nullId = 0;
			dst.push((char*)&nullId, sizeof(uint64_t));
		} else {
			sf::PolymorphInstance poly = objectType->instGetPolymorph(box.ptr);
			uint64_t &rev = state[box->objectId];
			if (box->objectRevision > rev) {
				rev = box->objectRevision;
				uint64_t newId = 1;
				dst.push((char*)&newId, sizeof(uint64_t));

				uint32_t tagValue = (uint32_t)poly.type->value;
				dst.push((char*)&tagValue, sizeof(uint32_t));

				serializeInstBinary(dst, state, poly.inst, poly.type->type);

			} else {
				dst.push((char*)&box->objectId, sizeof(uint64_t));
			}
		}
	} else if (flags & sf::Type::Polymorph) {

		sf::PolymorphInstance poly = type->instGetPolymorph(inst);

		if (poly.type) {
			uint32_t tagValue = (uint32_t)poly.type->value;
			dst.push((char*)&tagValue, sizeof(uint32_t));
			serializeInstBinary(dst, state, poly.inst, poly.type->type);
		} else {
			uint32_t tagValue = ~0u;
			dst.push((char*)&tagValue, sizeof(uint32_t));
		}

	} else if (flags & sf::Type::HasFields) {
		for (const sf::Field &field : type->fields) {
			serializeInstBinary(dst, state, base + field.offset, field.type);
		}
	} else if (flags & sf::Type::HasArray) {
		sf::Type *elem = type->elementType;
		size_t elemSize = elem->info.size;
		sf::VoidSlice slice = type->instGetArray(inst);
		uint32_t size = (uint32_t)slice.size;
		dst.push((char*)&size, sizeof(uint32_t));
		if (elem->flags & sf::Type::IsPod) {
			dst.push((char*)slice.data, slice.size * elemSize);
		} else {
			char *ptr = (char*)slice.data;
			for (uint32_t i = 0; i < size; i++) {
				serializeInstBinary(dst, state, ptr, elem);
				ptr += elemSize;
			}
		}
	} else {
		// TODO: Serialization callback
		sf_assert(0 && "Cannot serialize type");
	}
}

bool deserializeInstBinary(sf::Slice<char> &src, DeserializationState &state, void *inst, sf::Type *type)
{
	static sf::Type *objectType = sf::typeOf<sv2::Object>();

	if (type->baseType) {
		deserializeInstBinary(src, state, inst, type->baseType);
	}

	uint32_t flags = type->flags;
	char *base = (char*)inst;
	if (flags & sf::Type::IsPod) {
		if (src.size < type->info.size) return false;
		memcpy(inst, src.data, type->info.size);
		src = src.drop(type->info.size);
	} else if ((flags & sf::Type::HasPointer) && isObjectType(type->elementType)) {

		uint64_t idValue;
		if (src.size < sizeof(uint64_t)) return false;
		memcpy(&idValue, src.data, sizeof(uint64_t));
		src = src.drop(sizeof(uint64_t));

		sf::Box<sv2::Object> &box = *(sf::Box<sv2::Object>*)inst;
		if (idValue == 0) {
			box.reset();
		} else if (idValue == 1) {
			uint32_t tagValue;
			if (src.size < sizeof(uint32_t)) return false;
			memcpy(&tagValue, src.data, sizeof(uint32_t));
			src = src.drop(sizeof(uint32_t));

			const sf::PolymorphType *polyType = objectType->getPolymorphTypeByValue(tagValue);
			sf::Box<sv2::Object> &ref = state[box->objectId];
			if (!ref) {
				type->instSetPolymorph(inst, polyType->type);
				ref = box;
			} else {
				box = ref;
			}

			if (!deserializeInstBinary(src, state, ref, polyType->type)) return false;
		} else {
			sf::Box<sv2::Object> *ref = state.findValue(idValue);
			if (!ref) return false;
			box = *ref;
		}

	} else if (flags & sf::Type::Polymorph) {

		uint32_t tagValue;
		if (src.size < sizeof(uint32_t)) return false;
		memcpy(&tagValue, src.data, sizeof(uint32_t));
		src = src.drop(sizeof(uint32_t));

		const sf::PolymorphType *polyType = type->elementType->getPolymorphTypeByValue(tagValue);
		void *ptr = type->instSetPolymorph(inst, polyType->type);
		return deserializeInstBinary(src, state, ptr, polyType->type);

	} else if (flags & sf::Type::HasFields) {
		for (const sf::Field &field : type->fields) {
			if (!deserializeInstBinary(src, state, base + field.offset, field.type)) return false;
		}
	} else if (flags & sf::Type::HasArrayResize) {
		sf::Type *elem = type->elementType;
		size_t elemSize = elem->info.size;
		uint32_t size;
		if (src.size < sizeof(uint32_t)) return false;
		memcpy(&size, src.data,	sizeof(uint32_t));
		src = src.drop(sizeof(uint32_t));

		sf::VoidSlice slice = type->instArrayReserve(inst, size);
		if (elem->flags & sf::Type::IsPod) {
			size_t copySize = slice.size * elemSize;
			if (src.size < copySize) return false;
			memcpy(slice.data, src.data, copySize);
			src = src.drop(copySize);
		} else {
			char *ptr = (char*)slice.data;
			for (uint32_t i = 0; i < size; i++) {
				if (!deserializeInstBinary(src, state, ptr, elem)) return false;
				ptr += elemSize;
			}
		}

		type->instArrayResize(inst, size);
	} else if (flags & sf::Type::HasSetString) {
		uint32_t size;

		if (src.size < sizeof(uint32_t)) return false;
		memcpy(&size, src.data, sizeof(uint32_t));
		src = src.drop(sizeof(uint32_t));

		if (src.size < size) return false;
		type->instSetString(inst, src.take(size));
		src = src.drop(size);
	} else {
		// TODO: Serialization callback
		sf_assert(0 && "Cannot serialize type");
	}

	return true;
}

}
