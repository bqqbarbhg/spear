#include "Box.h"
#include "ext/mx/mx_platform.h"
#include "ext/mx/mx_sync.h"
#include "sf/Reflection.h"

namespace sf {

thread_local uint64_t t_boxCounter;
uint32_t g_boxThreadCounter = 0;

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

static_assert(sizeof(BoxHeader) <= 32, "BoxHeader is too large");

BoxHeader *getHeader(void *ptr)
{
	sf_assert(ptr);
	return (BoxHeader*)((char*)ptr - 32);
}

void *impBoxAllocate(size_t size, DestructRangeFn dtor)
{
	void *data = boxAlloc(size + 32);
	BoxHeader *header = (BoxHeader*)data;
	header->refCount = 1;
	header->size = (uint32_t)size;
	header->dtor = dtor;
	header->weakData = nullptr;
	uint64_t id = t_boxCounter;
	if (id == 0) {
		id = (mxa_inc32(g_boxThreadCounter) + 1) << 32u + 1;
	} else {
		id++;
	}
	t_boxCounter = id;
	header->id = id;
	return (char*)data + 32;
}

void impBoxIncRef(void *ptr)
{
	BoxHeader *header = getHeader(ptr);
	uint32_t refs = mxa_inc32(&header->refCount);
	sf_assert(refs > 0);
}

void impBoxDecRef(void *ptr)
{
	BoxHeader *header = getHeader(ptr);
	uint32_t left = mxa_dec32(&header->refCount) - 1;
	if (left == 0) {
		WeakBoxData *weak = (WeakBoxData*)mxa_load_ptr_acq(&header->weakData);
		if (weak) {
			mx_mutex_lock(&weak->mutex);
			weak->ptr = nullptr;
			mx_mutex_unlock(&weak->mutex);
			impBoxDecRef(weak);
		}

		header->dtor(ptr, 1);
		boxFree(header, header->size + 32);
	}
}

uint64_t impBoxGetId(void *ptr)
{
	if (!ptr) return 0;
	BoxHeader *header = getHeader(ptr);
	return header->id;
}

uint64_t impWeakBoxGetId(WeakBoxData *data)
{
	return data->id;
}

WeakBoxData *impWeakBoxMake(void *ptr)
{
	BoxHeader *header = getHeader(ptr);
	sf_assert(header->refCount > 0);

	WeakBoxData *weak = (WeakBoxData*)mxa_load_ptr_acq(&header->weakData);
	if (weak) {
		impBoxIncRef(weak);
	} else {
		weak = (WeakBoxData*)impBoxAllocate(sizeof(WeakBoxData), &sf::destructRangeImp<WeakBoxData>);
		weak->mutex.state = 0;
		weak->ptr = ptr;
		weak->id = header->id;
		if (!mxa_cas_ptr_rel(&header->weakData, nullptr, weak)) {
			impBoxDecRef(weak);
			weak = (WeakBoxData*)mxa_load_ptr_acq(&header->weakData);
			impBoxIncRef(weak);
		}
	}
	return weak;
}

bool impWeakBoxValid(WeakBoxData *data)
{
	return data->ptr != nullptr;
}

void *impWeakBoxRetain(WeakBoxData *data)
{
	mx_mutex_lock(&data->mutex);
	void *ptr = data->ptr;
	if (ptr) {
		BoxHeader *header = getHeader(ptr);
		uint32_t left;
		do {
			left = mxa_load32_nf(&header->refCount);
			if (left == 0) {
				ptr = nullptr;
				break;
			}
		} while (!mxa_cas32_nf(&header->refCount, left, left + 1));
	}
	mx_mutex_unlock(&data->mutex);
	return ptr;
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
