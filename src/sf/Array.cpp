#include "Array.h"
#include "Reflection.h"

namespace sf {

struct ArrayType final : Type
{
	ArrayType(const TypeInfo &info, Type *elemType)
		: Type("sf::Array", info, HasArray | HasArrayResize)
	{
		elementType = elemType;
	}

	virtual void getName(sf::StringBuf &buf) override
	{
		buf.append("sf::Array<");
		elementType->getName(buf);
		buf.append(">");
	}

	virtual VoidSlice instGetArray(void *inst, sf::Array<char> *scratch) override
	{
		ArrayBase *arr = (ArrayBase*)inst;
		return { arr->data, arr->size };
	}

	virtual VoidSlice instArrayReserve(void *inst, size_t size, sf::Array<char> *scratch) override
	{
		ArrayBase *arr = (ArrayBase*)inst;
		if (arr->capacity < size) {
			sf_assert(size <= UINT32_MAX);
			arr->capacity = (uint32_t)size;
			void *newData = memAlloc(size * elementType->info.size);
			if (arr->size) {
				elementType->info.moveRange(newData, arr->data, arr->size);
			}
			arr->data = newData;
		}
		if (size > arr->size) {
			char *base = (char*)arr->data + arr->size * elementType->info.size;
			elementType->info.constructRange(base, size - arr->size);
		}
		return { arr->data, arr->capacity };
	}

	virtual void instArrayResize(void *inst, size_t size, VoidSlice elements) override
	{
		ArrayBase *arr = (ArrayBase*)inst;
		sf_assert(arr->capacity >= size);
		if (size < arr->size) {
			char *base = (char*)arr->data + size * elementType->info.size;
			elementType->info.destructRange(base, arr->size - size);
		}
		arr->size = (uint32_t)size;
	}

};

struct SmallArrayType final : Type
{
	uint32_t n;

	SmallArrayType(const TypeInfo &info, Type *elemType, uint32_t n)
		: Type("sf::SmallArray", info, HasArray | HasArrayResize), n(n)
	{
		elementType = elemType;
	}

	virtual void getName(sf::StringBuf &buf) override
	{
		buf.append("sf::SmallArray<");
		elementType->getName(buf);
		buf.format(", %u>", n);
	}

	virtual VoidSlice instGetArray(void *inst, sf::Array<char> *scratch) override
	{
		ArrayBase *arr = (ArrayBase*)inst;
		return { arr->data, arr->size };
	}

	virtual VoidSlice instArrayReserve(void *inst, size_t size, sf::Array<char> *scratch) override
	{
		ArrayBase *arr = (ArrayBase*)inst;
		if (arr->capacity < size) {
			sf_assert(size <= UINT32_MAX);
			arr->capacity = (uint32_t)size;
			void *newData = memAlloc(size * elementType->info.size);
			if (arr->size) {
				elementType->info.moveRange(newData, arr->data, arr->size);
			}
			arr->data = newData;
		}
		if (size > arr->size) {
			char *base = (char*)arr->data + arr->size * elementType->info.size;
			elementType->info.constructRange(base, size - arr->size);
		}
		return { arr->data, arr->capacity };
	}

	virtual void instArrayResize(void *inst, size_t size, VoidSlice elements) override
	{
		ArrayBase *arr = (ArrayBase*)inst;
		sf_assert(arr->capacity >= size);
		if (size < arr->size) {
			char *base = (char*)arr->data + size * elementType->info.size;
			elementType->info.destructRange(base, arr->size - size);
		}
		arr->size = (uint32_t)size;
	}

};

void initArrayType(Type *t, const TypeInfo &info, Type *elemType)
{
	new (t) ArrayType(info, elemType);
}

void initSmallArrayType(Type *t, const TypeInfo &info, Type *elemType, uint32_t n)
{
	new (t) SmallArrayType(info, elemType, n);
}

}
