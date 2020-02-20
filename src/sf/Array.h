#pragma once

#include "Base.h"

namespace sf {

template <typename T>
struct Array
{
	T *data;
	uint32_t size;
	uint32_t capacity;

	Array() : data(nullptr), size(0), capacity(0) { }

	Array(const Array &rhs) {
		uint32_t sz = rhs.size;
		capacity = size = sz;
		if (sz > 0) {
			data = (T*)memAlloc(sz * sizeof(T));
			copyRangeImp<T>(data, rhs.data, sz);
		} else {
			data = nullptr;
		}
	}

	Array(Array &&rhs) : data(rhs.data), size(rhs.size), capacity(rhs.capacity) {
		rhs.data = nullptr;
		rhs.capacity = rhs.size = 0;
	}

	Array &operator=(const Array &rhs) {
		sf_assert(&rhs != this);
		if (size > 0) destructRangeImp<T>(data, size);
		uint32_t sz = rhs.size;
		if (sz > 0) {
			if (sz > size) impGrowTo(sz);
			copyRangeImp<T>(data, rhs.data, sz);
		}
		size = sz;
		return *this;
	}

	Array &operator=(Array &&rhs) {
		sf_assert(&rhs != this);
		if (size > 0) destructRangeImp<T>(data, size);
		if (data != nullptr && data != (T*)(this + 1)) {
			memFree(data);
		}
		data = rhs.data;
		size = rhs.size;
		capacity = rhs.capacity;
		rhs.data = nullptr;
		rhs.capacity = rhs.size = 0;
		return *this;
	}


	~Array() {
		if (size > 0) destructRangeImp<T>(data, size);
		if (data != nullptr && data != (T*)(this + 1)) {
			memFree(data);
		}
	}

	operator Slice<T>() { return Slice<T>(data, size); }
	operator Slice<const T>() const { return Slice<const T>(data, size); }
	Slice<T> slice() { return Slice<T>(data, size); }
	Slice<const T> slice() const { return Slice<const T>(data, size); }

	T *begin() { return data; }
	T *end() { return data + size; }
	const T *begin() const { return data; }
	const T *end() const { return data + size; }
	size_t byteSize() const { return (size_t)size * sizeof(T); }

	T &operator[](size_t index) {
		sf_assert(index < size);
		return data[index];
	}
	const T &operator[](size_t index) const {
		sf_assert(index < size);
		return data[index];
	}

	void push(const T &t) {
		if (size == capacity) impGrowOne();
		new (&data[size++]) T(t);
	}

	void push(T &&t) {
		if (size == capacity) impGrowOne();
		new (&data[size++]) T(std::move(t));
	}

	T &push() {
		if (size == capacity) impGrowOne();
		T *ptr = &data[size++];
		new (ptr) T();
		return *ptr;
	}

	T &pushUninit() {
		static_assert(!HasDestructor<T>::value, "Trying to pushUninit() on type with a destructor!");
		if (size == capacity) impGrowOne();
		return data[size++];
	}

	T *pushUninit(size_t num) {
		static_assert(!HasDestructor<T>::value, "Trying to pushUninit() on type with a destructor!");
		if (size + num > capacity) impGrowToGeometric(size + num);
		T *ptr = &data[size];
		size += (uint32_t)num;
		return ptr;
	}

	void push(const T *ts, size_t num) {
		size_t newSize = size + num;
		if (newSize > capacity) impGrowToGeometric(newSize);
		copyRangeImp<T>(data + size, ts, num);
		size = (uint32_t)newSize;
	}

	void push(Slice<const T> slice) {
		size_t newSize = size + slice.size;
		if (newSize > capacity) impGrowToGeometric(newSize);
		copyRangeImp<T>(data + size, slice.data, slice.size);
		size = (uint32_t)newSize;
	}

	void pop() {
		sf_assert(size > 0);
		data[--size].~T();
	}

	T popValue() {
		sf_assert(size > 0);
		T &src = data[--size];
		T dst(std::move(src));
		src.~T();
		return std::move(dst);
	}

	T &back() {
		sf_assert(size > 0);
		return data[size - 1];
	}

	T &back() const {
		sf_assert(size > 0);
		return data[size - 1];
	}

	void clear() {
		destructRangeImp<T>(data, size);
		size = 0;
	}

	void trim() {
		if (capacity == size) return;
		capacity = size;
		if (size == 0) {
			destructRangeImp<T>(data, size);
		} else {
			T *newData = (T*)memAlloc(capacity * sizeof(T));
			moveRangeImp<T>(newData, data, size);
			if (data != (T*)(this + 1)) memFree(data);
			data = newData;
		}
	}

	void reserve(size_t size) {
		if (size > this->capacity) impGrowTo(size);
	}

	void resize(size_t size) {
		if (size > this->capacity) impGrowTo(size);
		if (size > this->size) {
			constructRangeImp<T>(data + this->size, size - this->size);
		} else if (size < this->size) {
			destructRangeImp<T>(data + size, this->size - size);
		}
		this->size = (uint32_t)size;
	}

	void resizeUninit(size_t size) {
		static_assert(!HasDestructor<T>::value, "Trying to resizeUninit() on type with a destructor!");
		if (size > this->capacity) impGrowTo(size);
		this->size = (uint32_t)size;
	}

	void removeSwap(size_t index) {
		sf_assert(index < size);
		data[index].~T();
		if (index < --size) {
			new (&data[index]) T(std::move(data[size]));
			data[size].~T();
		}
	}

	void removeSwapPtr(T *t) { removeSwap((size_t)(t - data)); }

	void removeOrdered(size_t index) {
		sf_assert(index < size);
		uint32_t end = --size;
		while (index < end) {
			data[index].~T();
			new (&data[index]) T(std::move(data[index + 1]));
			index++;
		}
		data[index].~T();
	}

	void removeOrderedPtr(T *t) { removeOrdered((size_t)(t - data)); }

protected:
	Array(T *data, size_t capacity) : data(data), size(0), capacity((uint32_t)capacity) { }

	sf_noinline void impGrowOne() {
		capacity = max(capacity * 2, (uint32_t)(sizeof(T) < 128 ? 128 / sizeof(T) : 1));
		T *newData = (T*)memAlloc(capacity * sizeof(T));
		moveRangeImp<T>(newData, data, size);
		if (data != (T*)(this + 1)) memFree(data);
		data = newData;
	}

	sf_noinline void impGrowTo(size_t sz) {
		sf_assert(sz <= UINT32_MAX);
		capacity = (uint32_t)sz;
		T *newData = (T*)memAlloc(sz * sizeof(T));
		moveRangeImp<T>(newData, data, size);
		if (data != (T*)(this + 1)) memFree(data);
		data = newData;
	}

	sf_noinline void impGrowToGeometric(size_t sz) {
		sf_assert(sz <= UINT32_MAX);
		if (sz < capacity * 2) sz = capacity * 2;
		capacity = (uint32_t)sz;
		T *newData = (T*)memAlloc(sz * sizeof(T));
		moveRangeImp<T>(newData, data, size);
		if (data != (T*)(this + 1)) memFree(data);
		data = newData;
	}


};

template <typename T, uint32_t N>
struct SmallArray : Array<T>
{
	alignas(T) char localData[N * sizeof(T)];

	SmallArray() : Array<T>((T*)localData, N) { }

	SmallArray(SmallArray &&rhs) : Array<T>((T*)localData, N) {
		uint32_t sz = rhs.size;
		this->size = sz;
		if (sz > 0) {
			if (rhs.data == (T*)rhs.localData) {
				moveRangeImp<T>(this->data, rhs.data, sz);
			} else {
				if (sz > N) {
					this->data = rhs.data;
					this->capacity = rhs.capacity;
				} else {
					moveRangeImp<T>(this->data, rhs.data, sz);
				}
			}
		}
		rhs.data = (T*)rhs.localData;
		rhs.size = 0;
		rhs.capacity = N;
	}

	SmallArray(const SmallArray &rhs) : Array<T>((T*)localData, N) {
		uint32_t sz = rhs.size;
		this->size = sz;
		if (sz > N) {
			this->data = (T*)memAlloc(sz * sizeof(T));
			this->capacity = sz;
		}
		copyRange(this->data, rhs.data, sz);
	}

	SmallArray &operator=(const SmallArray &rhs) {
		sf_assert(&rhs != this);
		if (this->size > 0) destructRangeImp<T>(this->data, this->size);
		uint32_t sz = rhs.size;
		if (sz > 0) {
			if (sz > this->size) this->impGrowTo(sz);
			copyRangeImp<T>(this->data, rhs.data, sz);
		}
		this->size = sz;
		return *this;
	}

	SmallArray &operator=(SmallArray &&rhs) {
		sf_assert(&rhs != this);
		if (this->size > 0) destructRangeImp<T>(this->data, this->size);
		uint32_t sz = rhs.size;
		this->size = sz;
		this->data = (T*)localData;
		this->capacity = N;
		if (sz > 0) {
			if (rhs.data == (T*)rhs.localData) {
				moveRangeImp<T>(this->data, rhs.data, sz);
			} else {
				if (sz > N) {
					this->data = rhs.data;
					this->capacity = rhs.capacity;
				} else {
					moveRangeImp<T>(this->data, rhs.data, sz);
				}
			}
		}
		rhs.data = (T*)rhs.localData;
		rhs.size = 0;
		rhs.capacity = N;
	}

};

template <typename T> struct IsZeroInitializable<Array<T>> { enum { value = 1 }; };

template <typename T>
static bool findRemoveSwap(Array<T> &arr, const T &t)
{
	for (T &other : arr) {
		if (t == other) {
			arr.removeSwapPtr(&other);
			return true;
		}
	}
	return false;
}

}
