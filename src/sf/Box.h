#pragma once

#include "sf/Base.h"

namespace sf {

void *boxAlloc(size_t size);
void boxFree(void *ptr, size_t size);

struct RcHeader
{
	uint32_t refCount;
	uint32_t size;
	DestructRangeFn dtor;
};

void *impBoxAllocate(size_t size, DestructRangeFn dtor);
void impBoxIncRef(void *ptr);
void impBoxDecRef(void *ptr);

template <typename T>
struct Box
{
	T *ptr;

	Box() : ptr(nullptr) { }

	template <typename U>
	Box(Box<U>&& rhs) : ptr(rhs.ptr) { rhs.ptr = nullptr; }

	template <typename U>
	Box &operator=(Box<U>&& rhs) {
		if (&rhs == this) return *this;
		if (ptr) impBoxDecRef(ptr);
		ptr = rhs.ptr;
		rhs.ptr = nullptr;
		return *this;
	}

	template <typename U>
	Box(const Box<U>& rhs) : ptr(rhs.ptr) {
		if (rhs.ptr) impBoxIncRef(rhs.ptr);
	}

	template <typename U>
	Box &operator=(const Box<U>& rhs) {
		if (rhs.ptr) impBoxIncRef(rhs.ptr);
		if (ptr) impBoxDecRef(ptr);
		ptr = rhs.ptr;
		return *this;
	}

	~Box() { if (ptr) { impBoxDecRef(ptr); } }

	void reset() {
		if (ptr) { impBoxDecRef(ptr); ptr = nullptr; }
	}

	T *operator->() { return ptr; }
	const T *operator->() const { return ptr; }
	T &operator*() { return *ptr; }
	const T &operator*() const { return *ptr; }
};

template <typename T>
Box<T> box()
{
	Box<T> box;
	box.ptr = (T*)impBoxAllocate(sizeof(T), &destructRangeImp<T>);
	new (box.ptr) T();
	return box;
}

void initBoxType(Type *t, const TypeInfo &info, Type *elemType);

template <typename T>
struct InitType<Box<T>> {
	static void init(Type *t) {
		return initBoxType(t, sf::getTypeInfo<Box<T>>(), typeOfRecursive<T>());
	}
};

}
