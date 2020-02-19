#pragma once

#include "sf/Base.h"
#include "sf/Array.h"

extern int LOOPCOUNT;

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

		if (right - left <= 16 && false) {

			// Insertion sort
			T *it = left + 1;
			while (it <= right) {
				T *jt = it;
				while (jt != left && cmpFn(jt[0], jt[-1])) {
					swap(jt[0], jt[-1]);
					jt--;
				}
				it++;
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
			LOOPCOUNT += (int)(hi - lo);

			for (;;) {
				do lo++; while (cmpFn(*lo, *pivot));
				do hi--; while (cmpFn(*pivot, *hi));
				if (lo >= hi) break;
				swap(*lo, *hi);

				if (lo == pivot) pivot = hi;
				else if (hi == pivot) pivot = lo;
			}

			size_t limit = (right - left) >> 2;
			size_t numLeft = hi - left;
			size_t numRight = right - hi;
			if (rng == 0 && limit > 128 && (numLeft < limit || numRight < limit)) {
				rng = 1;
			}

			if (numLeft > 0) {
				if (numRight > 1) {
					sf_assert(top - stack < sf_arraysize(stack));
					top->left = hi + 1;
					top->right = right;
					top++;
				}
				right = hi;
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
			break;
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
