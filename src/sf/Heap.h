#pragma once

#include "sf/Array.h"

namespace sf {

template <typename T, typename Cmp>
void upHeapImp(T *values, uint32_t index, Cmp cmpFn)
{
	if (index == 0) return;
	uint32_t parent = (index - 1) >> 1;
	if (!cmpFn(values[index], values[parent])) return;
	T t(std::move(values[index]));
	values[index].~T();
	new (&values[index]) T(std::move(values[parent]));
	index = parent;
	while (index > 0) {
		if (cmpFn(values[index], values[parent])) {
			values[index].~T();
			new (&values[index]) T(std::move(values[parent]));
			index = parent;
		} else {
			break;
		}
	}
	new (&values[index]) T(std::move(t));
}

template <typename T, typename Cmp>
void downHeapImp(T *values, uint32_t size, uint32_t index, Cmp cmpFn)
{
	uint32_t child = index * 2 + 1;
	if (child >= size) return;
	if (child + 1 < size && cmpFn(values[child + 1], values[child])) {
		child++;
	}
	if (!cmpFn(values[child], values[index])) return;
	T t(std::move(values[index]));
	values[index].~T();
	new (&values[index]) T(std::move(values[child]));
	index = child;
	child = index * 2 + 1;
	while (child < size) {
		if (child + 1 < size && cmpFn(values[child + 1], values[child])) {
			child++;
		}
		if (cmpFn(values[child], values[index])) {
			values[index].~T();
			new (&values[index]) T(std::move(values[child]));
			index = child;
		} else {
			break;
		}
	}
	new (&values[index]) T(std::move(t));
}

template <typename T>
sf_inline void upHeap(sf::Slice<T> values, uint32_t index) {
	sf_assert(index < values.size);
	upHeapImp(values.data, index, [](const T &a, const T &b) -> bool {
		return a < b;
	});
}

template <typename T, typename Cmp>
sf_inline void upHeap(sf::Slice<T> values, uint32_t index, Cmp cmpFn) {
	sf_assert(index < values.size);
	upHeapImp(values.data, index, cmpFn);
}

template <typename T, typename Key>
sf_inline void upHeapBy(sf::Slice<T> values, uint32_t index, Key keyFn) {
	sf_assert(index < values.size);
	upHeapImp(values.data, index, [=](const T &a, const T &b) -> bool {
		return keyFn(a) < keyFn(b);
	});
}

template <typename T>
sf_inline void downHeap(sf::Slice<T> values, uint32_t index) {
	sf_assert(index < values.size);
	downHeapImp(values.data, index, [=](const T &a, const T &b) -> bool {
		return a < b;
	});
}

template <typename T, typename Cmp>
sf_inline void downHeap(sf::Slice<T> values, uint32_t index, Cmp cmpFn) {
	sf_assert(index < values.size);
	downHeapImp(values.data, index, cmpFn);
}

template <typename T, typename Key>
sf_inline void downHeapBy(sf::Slice<T> values, uint32_t index, Key keyFn) {
	sf_assert(index < values.size);
	downHeapImp(values.data, index, [=](const T &a, const T &b) -> bool {
		return keyFn(a) < keyFn(b);
	});
}

// -- Priority queue

template <typename T, typename U, typename Cmp>
sf_inline void priorityEnqueueImp(sf::Array<T> &arr, U &&t, Cmp cmpFn) {
	arr.push(std::forward<U>(t));
	upHeapImp(arr.data, arr.size - 1, cmpFn);
}

template <typename T, typename Cmp>
sf_inline T priorityDequeueImp(sf::Array<T> &arr, Cmp cmpFn) {
	sf_assert(arr.size > 0);
	T &top = arr.data[0];
	T t(std::move(top));
	if (arr.size > 1) {
		top.~T();
		new (&top) T(std::move(arr.data[arr.size - 1]));
		downHeapImp(arr.data, arr.size, 0, cmpFn);
	}
	arr.pop();
	return t;
}

template <typename T, typename U>
sf_inline void priorityEnqueue(sf::Array<T> &arr, U &&t) {
	priorityEnqueueImp(arr, std::forward<U>(t), [](const T &a, const T &b) {
		return a < b;
	});
}

template <typename T, typename U, typename Cmp>
sf_inline void priorityEnqueue(sf::Array<T> &arr, U &&t, Cmp cmpFn) {
	priorityEnqueueImp(arr, std::forward<U>(t), cmpFn);
}

template <typename T, typename U, typename Key>
sf_inline void priorityEnqueueBy(sf::Array<T> &arr, U &&t, Key keyFn) {
	sf_assert(index < arr.size);
	priorityEnqueueImp(arr, std::forward<U>(t), [=](const T &a, const T &b) -> bool {
		return keyFn(a) < keyFn(b);
	});
}

template <typename T>
sf_inline T priorityDequeue(sf::Array<T> &arr) {
	return priorityDequeueImp(arr, [](const T &a, const T &b) -> bool {
		return a < b;
	});
}

template <typename T, typename Cmp>
sf_inline T priorityDequeue(sf::Array<T> &arr, Cmp cmpFn) {
	return priorityDequeueImp(arr, cmpFn);
}

template <typename T, typename Key>
sf_inline void priorityDequeueBy(sf::Array<T> &arr, uint32_t index, Key keyFn) {
	return priorityDequeueImp(arr, [=](const T &a, const T &b) -> bool {
		return keyFn(a) < keyFn(b);
	});
}

}
