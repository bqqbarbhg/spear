#pragma once

#include "Box.h"
#include "ext/mx/mx_platform.h"
#include "sf/Reflection.h"

namespace sf {

void *boxAlloc(size_t size)
{
	return memAlloc(size);
}

void boxFree(void *ptr, size_t size)
{
	memFree(ptr);
}

RcHeader *getHeader(void *ptr)
{
	sf_assert(ptr);
	return (RcHeader*)((char*)ptr - 16);
}

void *impBoxAllocate(size_t size, DestructRangeFn dtor)
{
	void *data = boxAlloc(size + 16);
	RcHeader *header = (RcHeader*)data;
	header->refCount = 1;
	header->size = (uint32_t)size;
	header->dtor = dtor;
	return (char*)data + 16;
}

void impBoxIncRef(void *ptr)
{
	RcHeader *header = getHeader(ptr);
	mxa_inc32(&header->refCount);
}

void impBoxDecRef(void *ptr)
{
	RcHeader *header = getHeader(ptr);
	uint32_t left = mxa_dec32(&header->refCount) - 1;
	if (left == 0) {
		header->dtor(ptr, 1);
		boxFree(header, header->size + 16);
	}
}

struct BoxType final : Type
{
	BoxType(const TypeInfo &info, Type *elemType)
		: Type("sf::Box", info, HasPointer | HasArrayResize)
	{
		elementType = elemType;
	}

	virtual void init()
	{
		if (elementType->flags & PolymorphBase) {
			flags |= Polymorph;
		}
	}

	virtual void getName(sf::StringBuf &buf)
	{
		buf.append("sf::Box<");
		elementType->getName(buf);
		buf.append(">");
	}

	virtual PolymorphInstance instGetPolymorph(void *inst)
	{
		void *ptr = *(void**)inst;
		if (ptr) {
			return elementType->instGetPolymorph(ptr);
		} else {
			return { };
		}
	}

	virtual void *instSetPolymorph(void *inst, Type *type)
	{
		void *ptr = *(void**)inst;
		if (ptr) impBoxDecRef(ptr);
		ptr = impBoxAllocate(type->info.size, type->info.destructRange);
		*(void**)inst = ptr;
		type->info.constructRange(ptr, 1);
		return ptr;
	}

	virtual void *instSetPointer(void *inst)
	{
		void *ptr = *(void**)inst;
		if (ptr) impBoxDecRef(ptr);
		ptr = impBoxAllocate(elementType->info.size, elementType->info.destructRange);
		*(void**)inst = ptr;
		elementType->info.constructRange(ptr, 1);
		return ptr;
	}
};

void initBoxType(Type *t, const TypeInfo &info, Type *elemType)
{
	new (t) BoxType(info, elemType);
}

}
