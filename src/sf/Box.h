#pragma once

#include "sf/Base.h"

namespace sf {

void *boxAlloc(size_t size);
void boxFree(void *ptr, size_t size);

struct RcHeader
{
	uint32_t refCount;
	uint32_t size;
	DestructFn dtor;
};

void *impRcBoxAllocate(size_t size, DestructFn dtor);
void impRcBoxIncRef(void *ptr);
void impRcBoxDecRef(void *ptr);

template <typename T>
struct RcBox
{
	T *ptr;

	RcBox() : ptr(nullptr) { }

	template <typename U>
	RcBox(RcBox<U>&& rhs) : ptr(rhs.ptr) { rhs.ptr = nullptr; }

	template <typename U>
	RcBox &operator=(RcBox<U>&& rhs) {
		if (&rhs == this) return *this;
		if (ptr) impRcBoxDecRef(ptr);
		ptr = rhs.ptr;
		rhs.ptr = nullptr;
		return *this;
	}

	template <typename U>
	RcBox(const RcBox<U>& rhs) : ptr(rhs.ptr) {
		impRcBoxIncRef(rhs.ptr);
	}

	template <typename U>
	RcBox &operator=(const RcBox<U>& rhs) {
		impRcBoxIncRef(rhs.ptr);
		impRcBoxDecRef(ptr);
		ptr = rhs.ptr;
		return *this;
	}

	~RcBox() { if (ptr) { impRcBoxDecRef(ptr); } }

	void reset() {
		if (ptr) { impRcBoxDecRef(ptr); ptr = nullptr; }
	}

	T *operator->() { return ptr; }
	const T *operator->() const { return ptr; }
	T &operator*() { return *ptr; }
	const T &operator*() const { return *ptr; }
};

template <typename T>
RcBox<T> rcBox()
{
	RcBox<T> box;
	box.ptr = (T*)impRcBoxAllocate(sizeof(T), &destructImp<T>);
	new (box.ptr) T();
	return box;
}

void initRcBoxType(Type *t, Type *elemType, DestructFn dtor);

template <typename T>
struct InitType<RcBox<T>> {
	static void init(Type *t) {
		return initRcBoxType(t, typeOfRecursive<T>(), &destructImp<T>);
	}
};

}
