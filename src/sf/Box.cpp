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

void *impRcBoxAllocate(size_t size, DestructFn dtor)
{
	void *data = boxAlloc(size + 16);
	RcHeader *header = (RcHeader*)data;
	header->refCount = 1;
	header->size = (uint32_t)size;
	header->dtor = dtor;
	return (char*)data + 16;
}

void impRcBoxIncRef(void *ptr)
{
	RcHeader *header = getHeader(ptr);
	mxa_inc32(&header->refCount);
}

void impRcBoxDecRef(void *ptr)
{
	RcHeader *header = getHeader(ptr);
	uint32_t left = mxa_dec32(&header->refCount) - 1;
	if (left == 0) {
		header->dtor(ptr);
		boxFree(header, header->size + 16);
	}
}

struct RcBoxType final : Type
{
	DestructFn dtor;

	RcBoxType(Type *elemType, DestructFn dtor)
		: Type("sf::RcBox", sizeof(void*), HasArray | HasArrayResize), dtor(dtor)
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
		buf.append("sf::RcBox<");
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
		if (ptr) impRcBoxDecRef(ptr);
		ptr = impRcBoxAllocate(type->size, dtor);
		*(void**)inst = ptr;
		type->instConstruct(ptr, 1);
		return ptr;
	}
};

void initRcBoxType(Type *t, Type *elemType, DestructFn dtor)
{
	new (t) RcBoxType(elemType, dtor);
}

}
