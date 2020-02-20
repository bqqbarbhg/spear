#pragma once

#include "sf/Base.h"
#include "sf/Array.h"

namespace sf {

// -- Slice

struct ImpSortRange
{
	void *left, *right;
};

template <typename T, typename Cmp>
void impSort(T *data, size_t size, Cmp cmpFn)
{
	if (size <= 1) return;

	ImpSortRange stack[128], *top = stack;
	T *left = data, *right = data + size - 1;

	uint32_t rng = 0;

	for (;;) {
		sf_assert(left != right);

		if (right - left <= 32) {

			// Insertion sort
			T *hi = left + 1;
			while (hi <= right) {
				if (cmpFn(*hi, hi[-1])) {
					T *lo = hi;
					T tmp(std::move(*hi));
					do {
						lo->~T(); new (lo) T(std::move(lo[-1]));
						lo--;
					} while (lo != left && cmpFn(tmp, lo[-1]));
					lo->~T(); new (lo) T(std::move(tmp));
				}
				hi++;
			}

		} else {

			// Partition
			T *pivot = left + ((right - left) >> 1);
			if (rng) {
				// Xorshift32
				rng ^= rng << 13;
				rng ^= rng >> 17;
				rng ^= rng << 5;
				pivot = left + rng % (right - left);
			}

			T *lo = left - 1;
			T *hi = right + 1;

			for (;;) {
				do lo++; while (cmpFn(*lo, *pivot));
				do hi--; while (cmpFn(*pivot, *hi));
				if (lo >= hi) break;
				impSwap(*lo, *hi);

				if (lo == pivot) pivot = hi;
				else if (hi == pivot) pivot = lo;
			}

			lo = hi;
			if (!cmpFn(*lo, *pivot)) {
				while (lo > left && !cmpFn(lo[-1], *pivot)) --lo;
			}
			if (!cmpFn(*pivot, *hi)) {
				while (hi < right && !cmpFn(*pivot, hi[1])) ++hi;
			}

			size_t limit = ((right - left) >> 2) * 3;
			size_t numLeft = lo - left;
			size_t numRight = right - hi;
			if (rng == 0 && limit > 128 && (numLeft > limit || numRight > limit)) {
				rng = 1;
			}

			if (numLeft > 0) {
				if (numRight > 1) {
					top->left = hi + 1;
					top->right = right;
					top++;

					if (top - stack >= (sf_arraysize(stack) >> 1)) {
						// Transition to heap sort
						right = lo;
						break;
					}
				}
				right = lo;
				continue;
			} else if (numRight > 1) {
				left = hi + 1;
				right = right;
				continue;
			}
		}

		if (top != stack) {
			--top;
			left = (T*)top->left;
			right = (T*)top->right;
		} else {
			// Done!
			return;
		}
	}

	// Heap sort
	for (;;) {
		sf_assert(left != right);

		// No insertion sort here as this code should only be
		// executed for malicious inputs.

		size_t num = right - left + 1;
		size_t start = (num - 1) >> 1;
		size_t end = num - 1;
		for (;;) {

			size_t root = start;
			size_t child;
			while ((child = root*2 + 1) <= end) {
				size_t next = cmpFn(left[child], left[root]) ? root : child;
				if (child + 1 <= end && cmpFn(left[next], left[child + 1])) {
					next = child + 1;
				}
				if (next == root) break;
				impSwap(left[root], left[next]);
				root = next;
			}

			if (start > 0) {
				start--;
			} else if (end > 0) {
				impSwap(left[end], left[0]);
				end--;
			} else {
				break;
			}
		}

		if (top != stack) {
			--top;
			left = (T*)top->left;
			right = (T*)top->right;
		} else {
			// Done!
			return;
		}
	}
}

template <typename T>
sf_inline void sort(sf::Slice<T> data) {
	impSort(data.data, data.size, [](const T &a, const T &b) -> bool {
		return a < b;
	});
}
template <typename T>
sf_inline void sortRev(sf::Slice<T> data) {
	impSort(data.data, data.size, [](const T &a, const T &b) -> bool {
		return b < a;
	});
}

template <typename T, typename Cmp>
sf_inline void sort(sf::Slice<T> data, Cmp cmpFn) {
	impSort(data.data, data.size, [&](const T &a, const T &b) -> bool {
		return cmpFn(a, b);
	});
}
template <typename T, typename Cmp>
sf_inline void sortRev(sf::Slice<T> data, Cmp cmpFn) {
	impSort(data.data, data.size, [&](const T &a, const T &b) -> bool {
		return cmpFn(b, a);
	});
}

template <typename T, typename Key>
sf_inline void sortBy(sf::Slice<T> data, Key keyFn) {
	impSort(data.data, data.size, [&](const T &a, const T &b) -> bool {
		return keyFn(a) < keyFn(b);
	});
}
template <typename T, typename Key>
sf_inline void sortByRev(sf::Slice<T> data, Key keyFn) {
	impSort(data.data, data.size, [&](const T &a, const T &b) -> bool {
		return keyFn(b) < keyFn(a);
	});
}

// -- Array

template <typename T>
sf_inline void sort(sf::Array<T> &data) { return sort(data.slice()); }
template <typename T>
sf_inline void sortRev(sf::Array<T> &data) { return sortRev(data.slice()); }

template <typename T, typename Cmp>
sf_inline void sort(sf::Array<T> &data, Cmp cmpFn) { return sort(data.slice(), cmpFn); }
template <typename T, typename Cmp>
sf_inline void sortRev(sf::Array<T> &data, Cmp cmpFn) { return sortRev(data.slice(), cmpFn); }

template <typename T, typename Key>
sf_inline void sortBy(sf::Array<T> &data, Key keyFn) { return sortBy(data.slice(), keyFn); }
template <typename T, typename Key>
sf_inline void sortByRev(sf::Array<T> &data, Key keyFn) { return sortByRev(data.slice(), keyFn); }

}
