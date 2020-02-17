#pragma once

// Base header included by almost everything (except 3rd party libraries).
// Should contain things that go into a precompiled header.

#include "Platform.h"

#if SF_CC_MSC
	#pragma warning(push)
	// inline used more than once on forceinlined `inline` functions
	#pragma warning(disable: 4141) 
#endif

// -- Common headers

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// These are pretty heavy but also sadly necessary.
// TODO: Should look into reimplementing these>
#include <new>
#include <utility>
#include <type_traits>

// Required by intrinsics

#if SF_CC_MSC
	#include <intrin.h>
#endif

#if SF_ARCH_X86
	#include <xmmintrin.h>
	#include <emmintrin.h>
#endif

namespace sf {

// -- Memory allocation

// Generic malloc()-like allocation
#if SF_DEBUG
	void *memAlloc(size_t size);
	void *memRealloc(void *ptr, size_t size);
	void memFree(void *ptr);

	void *alignedAlloc(size_t size, size_t align);
	void *alignedRealloc(void *ptr, size_t size, size_t align);
	void alignedFree(void *ptr);
#else
	sf_inline void *memAlloc(size_t size) { return malloc(size); }
	sf_inline void *memRealloc(void *ptr, size_t size) { return realloc(ptr, size); }
	sf_inline void memFree(void *ptr) { free(ptr); }

	sf_inline void *alignedAlloc(size_t size, size_t align) { return _aligned_malloc(size, align); }
	sf_inline void *alignedRealloc(void *ptr, size_t size, size_t align) { return _aligned_realloc(ptr, size, align); }
	sf_inline void alignedFree(void *ptr) { _aligned_free(ptr); }
#endif

// -- Useful inline functions

sf_inline uint32_t alignUp(uint32_t v, uint32_t align) {
	sf_assert((align & (align - 1)) == 0);
	return (v + align - 1) & ~(align - 1);
}
sf_inline uint64_t alignUp(uint64_t v, uint64_t align) {
	sf_assert((align & (align - 1)) == 0);
	return (v + align - 1) & ~(align - 1);
}

template <typename T>
sf_inline T min(const T &a, const T &b) { return a < b ? a : b; }
template <typename T>
sf_inline T max(const T &a, const T &b) { return a < b ? b : a; }

template <typename T>
sf_inline T min(const T &a, const T &b, const T &c) { return min(min(a, b), c); }
template <typename T>
sf_inline T max(const T &a, const T &b, const T &c) { return max(max(a, b), c); }

sf_inline uint32_t min(uint32_t a, uint32_t b) { return a < b ? a : b; }
sf_inline uint32_t max(uint32_t a, uint32_t b) { return a < b ? b : a; }
sf_inline uint64_t min(uint64_t a, uint64_t b) { return a < b ? a : b; }
sf_inline uint64_t max(uint64_t a, uint64_t b) { return a < b ? b : a; }
sf_inline int32_t min(int32_t a, int32_t b) { return a < b ? a : b; }
sf_inline int32_t max(int32_t a, int32_t b) { return a < b ? b : a; }
sf_inline int64_t min(int64_t a, int64_t b) { return a < b ? a : b; }
sf_inline int64_t max(int64_t a, int64_t b) { return a < b ? b : a; }

#if SF_ARCH_X86
	sf_inline float min(float a, float b) { return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(a), _mm_set_ss(b))); }
	sf_inline float max(float a, float b) { return _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(a), _mm_set_ss(b))); }
	sf_inline double min(double a, double b) { return _mm_cvtsd_f64(_mm_min_sd(_mm_set_sd(a), _mm_set_sd(b))); }
	sf_inline double max(double a, double b) { return _mm_cvtsd_f64(_mm_max_sd(_mm_set_sd(a), _mm_set_sd(b))); }
#else
	sf_inline float min(float a, float b) { return a < b ? a : b; }
	sf_inline float max(float a, float b) { return b < a ? a : b; }
	sf_inline double min(double a, double b) { return a < b ? a : b; }
	sf_inline double max(double a, double b) { return b < a ? a : b; }
#endif

template <typename T>
sf_inline T clamp(const T &v, const T &min, const T &max) { return ::sf::min(::sf::max(v, min), max); }

// -- Float intrinsics

#if SF_ARCH_X86
	sf_inline float sqrt(float a) { return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(a))); }
	sf_inline double sqrt(double a) { return _mm_cvtsd_f64(_mm_sqrt_sd(_mm_setzero_pd(), _mm_set_sd(a))); }
	sf_inline float abs(float a) { return _mm_cvtss_f32(_mm_andnot_ps(_mm_set_ss(-0.0f), _mm_set_ss(a))); }
	sf_inline double abs(double a) { return _mm_cvtsd_f64(_mm_andnot_pd(_mm_set_sd(-0.0), _mm_set_sd(a))); }
#else
	sf_inline float sqrt(float a) { return ::sqrtf(a); }
	sf_inline double sqrt(double a) { return ::sqrt(a); }
	sf_inline float abs(float a) { return ::fabsf(a); }
	sf_inline double abs(double a) { return ::fabs(a); }
#endif

// -- Constants

constexpr float F_PI   = 3.14159265358979323846f;
constexpr float F_2PI  = 6.28318530717958647692f;
constexpr double D_PI  = 3.14159265358979323846;
constexpr double D_2PI = 6.28318530717958647692;

// -- Utilities

// Formatted print to a memory buffer allocated via sf::memAlloc()
char *memPrintf(const char *fmt, ...);

// Print to debugger or console window
void debugPrint(const char *fmt, ...);
void debugPrintLine(const char *fmt, ...);

// Formatted assertions
#if SF_DEBUG
	#define sf_assertf(cond, ...) do { if (!(cond)) { ::sf::debugPrintLine("Assertion failed: " __VA_ARGS__); sf_debugbreak(); } } while (0)
	#define sf_failf(...) do { ::sf::debugPrintLine("Failed: " __VA_ARGS__); sf_debugbreak(); } while (0)
#else
	#define sf_assertf(cond, ...) (void)0
	#define sf_failf(...) (void)0
#endif

// -- Type traits

// Can the type be initialized by calling `memset(0)`
template <typename T> struct IsZeroInitializable {
	enum { value = std::is_trivially_default_constructible<T>::value };
};

// Can the type be moved by calling `memcpy()` and not destructing the previous value.
template <typename T> struct IsRelocatable {
	enum { value = std::is_trivially_move_constructible<T>::value && std::is_trivially_destructible<T>::value };
};

// Can the type be copied by calling `memcpy()`.
template <typename T> struct IsCopyable {
	enum { value = std::is_trivially_copyable<T>::value };
};

// Is calling the destructor necessary.
template <typename T> struct HasDestructor {
	enum { value = !std::is_trivially_destructible<T>::value };
};

// -- Centralized template operations

// Template operations for calling constructors/destructors,
// they all operate for a range (call with `size=1` for a single one).
// The *Imp variants can be casted into a generic function pointer.

// constructRange(): Calls default constructor
// moveRange(): If necessary, calls move constructor and destructs the old value, otherwise memcpy()
// copyRange(): If necessary, calls copy constructor, otherwise memcpy()
// destructRange(): If necessary, calls destructor

typedef void (*ConstructRangeFn)(void *data, size_t size);
typedef void (*MoveRangeFn)(void *dst, void *src, size_t size);
typedef void (*CopyRangeFn)(void *dst, const void *src, size_t size);
typedef void (*DestructRangeFn)(void *data, size_t size);

template <typename T> sf_noinline inline typename std::enable_if<!IsZeroInitializable<T>::value>::type
constructRangeImp(void *data, size_t size) {
	for (T *t = (T*)data, *e = t + size; t != e; t++) {
		new (t) T();
	}
}

template <typename T> sf_forceinline inline typename std::enable_if<IsZeroInitializable<T>::value>::type
constructRangeImp(void *data, size_t size) {
	memset(data, 0, sizeof(T) * size);
}


template <typename T> sf_noinline inline typename std::enable_if<!IsRelocatable<T>::value>::type
moveRangeImp(void *dst, void *src, size_t size) {
	for (T *d = (T*)dst, *s = (T*)src, *e = d + size; d != e; d++, s++) {
		new (d) T(std::move(*s));
		s->~T();
	}
}

template <typename T> sf_forceinline inline typename std::enable_if<IsRelocatable<T>::value>::type
moveRangeImp(void *dst, void *src, size_t size) {
	memcpy(dst, src, size * sizeof(T));
}

template <typename T> sf_noinline inline typename std::enable_if<!IsCopyable<T>::value>::type
copyRangeImp(void *dst, const void *src, size_t size) {
	const T *s = (const T*)src;
	for (T *d = (T*)dst, *e = d + size; d != e; d++, s++) {
		new (d) T(*s);
	}
}

template <typename T> sf_forceinline inline typename std::enable_if<IsCopyable<T>::value>::type
copyRangeImp(void *dst, const void *src, size_t size) {
	memcpy(dst, src, size * sizeof(T));
}

template <typename T> sf_noinline inline typename std::enable_if<HasDestructor<T>::value>::type
destructRangeImp(void *data, size_t size) {
	for (T *t = (T*)data, *e = t + size; t != e; t++) {
		t->~T();
	}
}

template <typename T> sf_forceinline inline typename std::enable_if<!HasDestructor<T>::value>::type
destructRangeImp(void *, size_t) {
	// Nop
}

template <typename T> sf_inline void constructRange(T *data, size_t size) { constructRangeImp<T>(data, size); }
template <typename T> sf_inline void moveRange(T *dst, T *src, size_t size) { moveRangeImp<T>(dst, src, size); }
template <typename T> sf_inline void copyRange(T *dst, const T *src, size_t size) { copyRangeImp<T>(dst, src, size); }
template <typename T> sf_inline void destructRange(T *data, size_t size) { destructRangeImp<T>(data, size); }

// -- Slice type for ranges

template <typename T>
struct Slice
{
	T *data;
	size_t size;

	Slice() : data(nullptr), size(0) { }
	Slice(T *data, size_t size) : data(data), size(size) { }
	template <size_t N>
	Slice(T (&arr)[N]) : data(arr), size(N) { }
	operator Slice<const T>() { return Slice<const T>(data, size); }

	T *begin() const { return data; }
	T *end() const { return data + size; }

	Slice<T> takeRight(size_t num) const {
		if (num > size) num = size;
		return { data + size - num, num };
	}

	T &operator[](size_t index) const {
		sf_assert(index < size);
		return data[index];
	}
};

template <typename T>
Slice<T> slice(T *data, size_t size) { return Slice<T>(data, size); }

}

#if SF_CC_MSC
	#pragma warning(pop)
#endif