#pragma once

#include "sf/Base.h"

namespace sf {

void *boxAlloc(size_t size);
void boxFree(void *ptr, size_t size);

struct WeakBoxData;

struct BoxHeader
{
	uint32_t refCount;
	uint32_t size;
	DestructRangeFn dtor;
	WeakBoxData *weakData;
	uint64_t id;
};

void *impBoxAllocate(size_t size, DestructRangeFn dtor);
void impBoxIncRef(void *ptr);
void impBoxDecRef(void *ptr);
uint64_t impBoxGetId(void *ptr);
WeakBoxData *impWeakBoxMake(void *ptr);
uint64_t impWeakBoxGetId(WeakBoxData *data);
bool impWeakBoxValid(WeakBoxData *data);
void *impWeakBoxRetain(WeakBoxData *data);

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

	bool operator!() const { return ptr == nullptr; }
	explicit operator bool() const { return ptr != nullptr; }

	operator T*() const { return ptr; }
	T *operator->() const { return ptr; }
	std::add_lvalue_reference<T> operator*() const { return *ptr; }

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
struct WeakBox
{
	sf::Box<WeakBoxData> data;

	WeakBox() { }
	WeakBox(const Box<T> &box)
	{
		data.ptr = impWeakBoxMake(box.ptr);
	}

	uint64_t getId() const { return impWeakBoxGetid(data.ptr); }
	bool isValid() const { return impWeakBoxValid(data.ptr); }

	Box<T> retain() const {
		Box<T> box;
		box.ptr = (T*)impWeakBoxRetain(data.ptr);
		return box;
	}

	bool operator!() const { return data.ptr == nullptr; }
	explicit operator bool() const { return data.ptr != nullptr; }
};

void initBoxType(Type *t, const TypeInfo &info, Type *elemType);

template <typename T>
struct InitType<Box<T>> {
	static void init(Type *t) {
		return initBoxType(t, sf::getTypeInfo<Box<T>>(), typeOfRecursive<T>());
	}
};

}
