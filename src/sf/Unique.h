#pragma once

#include "sf/Base.h"

namespace sf {

template <typename T>
struct Unique
{
	T *ptr;

	Unique() : ptr(nullptr) { }
	Unique(Unique&& rhs) : ptr(rhs.ptr) { rhs.ptr = nullptr; }
	Unique &operator=(Unique&& rhs) {
		if ((void*)&rhs == (void*)this) return *this;
		ptr = rhs.ptr;
		rhs.ptr = nullptr;
		return *this;
	}

	Unique(const Unique& rhs) = delete;
	Unique &operator=(const Unique& rhs) = delete;

	template <typename U>
	Unique(Unique<U>&& rhs) : ptr(rhs.ptr) { rhs.ptr = nullptr; }

	template <typename U>
	Unique &operator=(Unique<U>&& rhs) {
		if ((void*)&rhs == (void*)this) return *this;
		ptr = rhs.ptr;
		rhs.ptr = nullptr;
		return *this;
	}

	~Unique() { delete ptr; }

	void reset() {
		if (ptr) { delete ptr; ptr = nullptr; }
	}

	sf_forceinline bool operator!() const { return ptr == nullptr; }
	sf_forceinline explicit operator bool() const { return ptr != nullptr; }

	sf_forceinline operator T*() const { return ptr; }
	sf_forceinline T *operator->() const { return ptr; }
	sf_forceinline typename std::add_lvalue_reference<T>::type operator*() const { return *ptr; }
};

template <typename T, typename... Args>
Unique<T> unique(Args&&... args)
{
	Unique<T> unique;
	unique.ptr = new T(std::forward<Args>(args)...);
	return unique;
}

}
