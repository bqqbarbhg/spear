#pragma once

#include "sf/Base.h"

namespace sf {

void *boxAlloc(size_t size);
void boxFree(void *ptr, size_t size);

struct BoxHeader
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

	Box(Box&& rhs) : ptr(rhs.ptr) { rhs.ptr = nullptr; }

	Box &operator=(Box&& rhs) {
		if ((void*)&rhs == (void*)this) return *this;
		if (ptr) impBoxDecRef(ptr);
		ptr = rhs.ptr;
		rhs.ptr = nullptr;
		return *this;
	}

	Box(const Box& rhs) : ptr(rhs.ptr) {
		if (rhs.ptr) impBoxIncRef(rhs.ptr);
	}

	Box &operator=(const Box& rhs) {
		if (rhs.ptr) impBoxIncRef(rhs.ptr);
		if (ptr) impBoxDecRef(ptr);
		ptr = rhs.ptr;
		return *this;
	}

	template <typename U>
	Box(Box<U>&& rhs) : ptr(rhs.ptr) { rhs.ptr = nullptr; }

	template <typename U>
	Box &operator=(Box<U>&& rhs) {
		if ((void*)&rhs == (void*)this) return *this;
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

	template <typename U> const Box<U> &cast() const { return *(Box<U>*)this; };
	template <typename U> Box<U> &cast() { return *(Box<U>*)this; };

	sf_forceinline bool operator!() const { return ptr == nullptr; }
	sf_forceinline explicit operator bool() const { return ptr != nullptr; }

	sf_forceinline operator T*() const { return ptr; }
	sf_forceinline T *operator->() const { return ptr; }
	sf_forceinline typename std::add_lvalue_reference<T>::type operator*() const { return *ptr; }

	uint64_t getId() const { return impBoxGetId(ptr); }
};

template <typename T, typename... Args>
Box<T> box(Args&&... args)
{
	Box<T> box;
	box.ptr = (T*)impBoxAllocate(sizeof(T), &destructRangeImp<T>);
	new (box.ptr) T(std::forward<Args>(args)...);
	return box;
}

template <typename T>
Box<T> boxFromPointer(T *t) {
	Box<T> box;
	impBoxIncRef(t);
	box.ptr = t;
	return box;
}

void initBoxType(Type *t, const TypeInfo &info, Type *elemType);

template <typename T>
struct InitType<Box<T>> {
	static void init(Type *t) {
		return initBoxType(t, sf::getTypeInfo<Box<T>>(), typeOfRecursive<T>());
	}
};

template <typename T> struct IsZeroInitializable<Box<T>> { enum { value = 1 }; };
template <typename T> struct IsRelocatable<Box<T>> { enum { value = 1 }; };

}
