#include "Box.h"
#include "ext/mx/mx_platform.h"
#include "ext/mx/mx_sync.h"
#include "sf/Reflection.h"

namespace sf {

static constexpr const size_t BoxHeaderSize = 16;

#if SF_DEBUG
static constexpr const size_t BoxMagicSize = 16;

const char boxMagic[] = "\x0b\x1b\x2b\x3b\x4b\x5b\x6b\x7b\x8b\x9b\xab\xbb\xcb\xdb\xeb\xfb";
const char boxDeletedMagic[] = "\x0d\x1d\x2d\x3d\x4d\x5d\x6d\x7d\x8d\x9d\xad\xbd\xcd\xdd\xed\xfd";

static_assert(sizeof(boxMagic) == BoxMagicSize + 1, "boxMagic size");
static_assert(sizeof(boxDeletedMagic) == BoxMagicSize + 1, "boxDeletedMagic size");

sf_inline void boxInitMagic(BoxHeader *header) {
	char *magic = (char*)header + BoxHeaderSize + header->size;
	memcpy(magic, boxMagic, BoxMagicSize);
}
sf_inline void boxDeleteMagic(BoxHeader *header) {
	char *magic = (char*)header + BoxHeaderSize + header->size;
	memcpy(magic, boxDeletedMagic, BoxMagicSize);
}
sf_inline void boxCheckMagic(const BoxHeader *header) {
	const char *magic = (const char*)header + BoxHeaderSize + header->size;
	sf_assert(!memcmp(magic, boxMagic, BoxMagicSize));
}
#else
static constexpr const size_t BoxMagicSize = 0;
sf_inline void boxInitMagic(BoxHeader *header) { }
sf_inline void boxDeleteMagic(BoxHeader *header) { }
sf_inline void boxCheckMagic(const BoxHeader *header) { }
#endif

struct WeakBoxData
{
	mx_mutex mutex;
	void *ptr;
	uint64_t id;
};

void *boxAlloc(size_t size)
{
	return memAlloc(size);
}

void boxFree(void *ptr, size_t size)
{
	memFree(ptr);
}

static_assert(sizeof(BoxHeader) <= BoxHeaderSize, "BoxHeader is too large");

BoxHeader *getHeader(void *ptr)
{
	sf_assert(ptr);
	return (BoxHeader*)((char*)ptr - BoxHeaderSize);
}

void *impBoxAllocate(size_t size, DestructRangeFn dtor)
{
	void *data = boxAlloc(size + (BoxHeaderSize + BoxMagicSize));
	BoxHeader *header = (BoxHeader*)data;
	header->refCount = 1;
	header->size = (uint32_t)size;
	header->dtor = dtor;
	boxInitMagic(header);
	return (char*)data + BoxHeaderSize;
}

void impBoxIncRef(void *ptr)
{
	BoxHeader *header = getHeader(ptr);
	boxCheckMagic(header);
	uint32_t refs = mxa_inc32(&header->refCount);
	sf_assert(refs > 0);
}

void impBoxDecRef(void *ptr)
{
	BoxHeader *header = getHeader(ptr);
	boxCheckMagic(header);
	uint32_t left = mxa_dec32(&header->refCount) - 1;
	if (left == 0) {
		header->dtor(ptr, 1);
		boxFree(header, header->size + (BoxHeaderSize + BoxMagicSize));
	}
}

struct BoxType final : Type
{
	BoxType(const TypeInfo &info, Type *elemType)
		: Type("sf::Box", info, HasPointer | IsBox)
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
