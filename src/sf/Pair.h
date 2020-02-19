#pragma once

#include "sf/Base.h"

namespace sf {

template <typename A, typename B>
struct Pair
{
	A a;
	B b;

	Pair() { }
	Pair(const A &a, const B &b) : a(a), b(b) { }
	Pair(const Pair&) = default;
	Pair(Pair&&) = default;

	bool operator==(const Pair &rhs) const { return a == rhs.a && b == rhs.b; }
	bool operator!=(const Pair &rhs) const { return a != rhs.a && b != rhs.b; }
	bool operator<(const Pair &rhs) const {
		if (a < rhs.a) return true;
		if (rhs.a < a) return false;
		return b < rhs.b;
	}
};

template <typename A, typename B>
inline uint32_t hash(const Pair<A, B> &pair)
{
	return hashCombine(hash(pair.a), hash(pair.b));
}

template <typename A, typename B>
inline int compare(const Pair<A, B> &lhs, const Pair<A, B> &rhs)
{
	if (lhs.a < rhs.a) return -1;
	if (rhs.a < lhs.a) return +1;
	if (lhs.b < rhs.b) return -1;
	if (rhs.b < lhs.b) return +1;
	return 0;
}


template <typename A, typename B>
inline Pair<A, B> pair(const A &a, const B &b)
{
	return Pair<A, B>(a, b);
}

}
