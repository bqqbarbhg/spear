#include "Array.h"
#include "Reflection.h"

namespace sf {

struct ArrayType final : Type
{
	ArrayType(Type *elemType)
		: Type("sf::Array", sizeof(ArrayBase), HasArray | HasArrayResize)
	{
		elementType = elemType;
	}

	virtual void getName(sf::StringBuf &buf)
	{
		buf.append("sf::Array<");
		elementType->getName(buf);
		buf.append(">");
	}

	virtual VoidSlice instGetArray(void *inst)
	{
		ArrayBase *arr = (ArrayBase*)inst;
		return { arr->data, arr->size };
	}

	virtual VoidSlice instArrayReserve(void *inst, size_t size)
	{
		ArrayBase *arr = (ArrayBase*)inst;
		if (arr->capacity < size) {
			sf_assert(size <= UINT32_MAX);
			arr->capacity = (uint32_t)size;
			void *newData = memAlloc(size * elementType->size);
			if (arr->size) {
				elementType->instMove(newData, arr->data, arr->size);
			}
			arr->data = newData;
		}
		if (size > arr->size) {
			char *base = (char*)arr->data + arr->size * elementType->size;
			elementType->instConstruct(base, size - arr->size);
		}
		return { arr->data, arr->capacity };
	}

	virtual void instArrayResize(void *inst, size_t size)
	{
		ArrayBase *arr = (ArrayBase*)inst;
		sf_assert(arr->capacity >= size);
		if (size < arr->size) {
			char *base = (char*)arr->data + size * elementType->size;
			elementType->instDestruct(base, arr->size - size);
		}
		arr->size = (uint32_t)size;
	}

};

void initArrayType(Type *t, Type *elemType)
{
	new (t) ArrayType(elemType);
}

}
