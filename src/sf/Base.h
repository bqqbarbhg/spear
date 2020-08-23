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
sf_inline sf_malloc_like void *memAlloc(size_t size) { return sf_malloc(size); }
sf_inline void *memRealloc(void *ptr, size_t size) { return sf_realloc(ptr, size); }
sf_inline void memFree(void *ptr) { sf_free(ptr); }

sf_inline sf_malloc_like void *memAllocAligned(size_t size, size_t align) {
	if (align <= 8) {
		return sf_malloc(size);
	} else {
		return sf_malloc_aligned(size, align);
	}
}
sf_inline void memFreeAligned(void *ptr, size_t align) {
	if (align <= 8) {
		sf_free(ptr);
	} else {
		sf_free_aligned(ptr, align);
	}
}

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
sf_inline T clamp(const T &v, const T &minV, const T &maxV) {
    return min(max(v, minV), maxV); }

sf_inline float lerp(float a, float b, float t) { return a * (1.0f - t) + b * t; }

// -- Float intrinsics

#if SF_ARCH_X86
	sf_inline float sqrt(float a) { return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(a))); }
	sf_inline double sqrt(double a) { return _mm_cvtsd_f64(_mm_sqrt_sd(_mm_setzero_pd(), _mm_set_sd(a))); }
	sf_inline float abs(float a) { return _mm_cvtss_f32(_mm_andnot_ps(_mm_set_ss(-0.0f), _mm_set_ss(a))); }
	sf_inline double abs(double a) { return _mm_cvtsd_f64(_mm_andnot_pd(_mm_set_sd(-0.0), _mm_set_sd(a))); }
	sf_inline float copysign(float a, float b) {
		const __m128 sign = _mm_set_ss(-0.0f);
		return _mm_cvtss_f32(_mm_or_ps(_mm_andnot_ps(sign, _mm_set_ss(a)), _mm_and_ps(sign, _mm_set_ss(b))));
	}
	sf_inline double copysign(double a, double b) {
		const __m128d sign = _mm_set_sd(-0.0);
		return _mm_cvtsd_f64(_mm_or_pd(_mm_andnot_pd(sign, _mm_set_sd(a)), _mm_and_pd(sign, _mm_set_sd(b))));
	}
#elif SF_CC_GNU || SF_CC_CLANG
	sf_inline float sqrt(float a) { return __builtin_sqrtf(a); }
	sf_inline double sqrt(double a) { return __builtin_sqrt(a); }
	sf_inline float abs(float a) { return __builtin_fabsf(a); }
	sf_inline double abs(double a) { return __builtin_fabs(a); }
	sf_inline float copysign(float a, float b) { return __builtin_copysignf(a, b); }
	sf_inline double copysign(double a, double b) { return __builtin_copysign(a, b); }
#else
	sf_inline float sqrt(float a) { return ::sqrtf(a); }
	sf_inline double sqrt(double a) { return ::sqrt(a); }
	sf_inline float abs(float a) { return ::fabsf(a); }
	sf_inline double abs(double a) { return ::fabs(a); }
	sf_inline float copysign(float a, float b) { return ::copysignf(a, b); }
	sf_inline double copysign(double a, double b) { return ::copysign(a, b); }
#endif

// -- Misc intrinsics

#if SF_ARCH_X86
	sf_inline void prefetch(const void *ptr) { _mm_prefetch((const char*)ptr, _MM_HINT_T0); }
#elif SF_CC_GNU
	sf_inline void prefetch(const void *ptr) { __builtin_prefetch((const char*)ptr); }
#else
	sf_inline void prefetch(const void *ptr) { }
#endif

// -- Constants

static const constexpr float F_PI   = 3.14159265358979323846f;
static const constexpr float F_2PI  = 6.28318530717958647692f;
static const constexpr double D_PI  = 3.14159265358979323846;
static const constexpr double D_2PI = 6.28318530717958647692;

// -- Utilities

// Formatted print to a memory buffer allocated via sf::memAlloc()
char *memPrintf(const char *fmt, ...);

// Print to debugger or console window
void debugPrint(const char *fmt, ...);
void debugPrintLine(const char *fmt, ...);

struct String;
void debugPrintJson(const sf::String &label, const sf::String &json);

// Formatted assertions
#if SF_DEBUG
	#define sf_assertf(cond, ...) do { if (!(cond)) { ::sf::debugPrintLine("Assertion failed: " __VA_ARGS__); sf_debugbreak(); } } while (0)
	#define sf_failf(...) do { ::sf::debugPrintLine("Failed: " __VA_ARGS__); sf_debugbreak(); } while (0)
#else
	#define sf_assertf(cond, ...) (void)0
	#define sf_failf(...) (void)0
#endif

// Generic buffer hash
uint32_t hashBuffer(const void *data, size_t size);
sf_inline uint32_t hashCombine(uint32_t a, uint32_t b) {
	return  ((a << 5u | a >> 27u) ^ b) * UINT32_C(0x9e3779b9);
}
sf_inline uint32_t hash(bool val) { return (uint32_t)val; }
uint32_t hash(uint32_t val);
uint32_t hash(uint64_t val);
sf_inline uint32_t hash(int32_t val) { return hash((uint32_t)val); }
sf_inline uint64_t hash(int64_t val) { return hash((uint64_t)val); }
uint32_t hashReverse32(uint32_t hash);
uint32_t hash(const void *val) = delete;
sf_inline uint32_t hashPointer(const void *val) { return hash((uint64_t)(uintptr_t)val); }

template <typename T>
sf_inline void impSwap(T &a, T &b)
{
	T tmp(std::move(a));
	a.~T(); new (&a) T(std::move(b));
	b.~T(); new (&b) T(std::move(tmp));
}

template <typename T>
sf_inline void reset(T &t)
{
	t.~T();
	new (&t) T();
}

template <typename T>
sf_inline void memZero(T &t)
{
	memset(&t, 0, sizeof(t));
}

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

template <typename T> sf_forceinline typename std::enable_if<IsZeroInitializable<T>::value>::type
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

template <typename T> sf_forceinline typename std::enable_if<IsRelocatable<T>::value>::type
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

template <typename T> sf_forceinline typename std::enable_if<IsCopyable<T>::value>::type
copyRangeImp(void *dst, const void *src, size_t size) {
	memcpy(dst, src, size * sizeof(T));
}

template <typename T> sf_noinline inline typename std::enable_if<HasDestructor<T>::value>::type
destructRangeImp(void *data, size_t size) {
	for (T *t = (T*)data, *e = t + size; t != e; t++) {
		t->~T();
	}
}

template <typename T> sf_forceinline typename std::enable_if<!HasDestructor<T>::value>::type
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
	operator Slice<const T>() const { return Slice<const T>(data, size); }

	T *begin() const { return data; }
	T *end() const { return data + size; }

	Slice<T> take(size_t num) const {
		return { data, sf::min(num, size) };
	}

	Slice<T> drop(size_t num) const {
		sf_assert(num <= size);
		return { data + num, size - num };
	}

	Slice<T> dropRight(size_t num) const {
		sf_assert(num <= size);
		return { data, size - num };
	}

	T &operator[](size_t index) const {
		sf_assert(index < size);
		return data[index];
	}
};

struct VoidSlice
{
	void *data;
	size_t size;

	VoidSlice() : data(nullptr), size(0) { }
	VoidSlice(void *data, size_t size) : data(data), size(size) { }
	template <typename T>
	VoidSlice(sf::Slice<T> slice) : data((void*)slice.data), size(slice.size) { }

	template <typename T>
	Slice<T> cast() const { return Slice<T>((T*)data, size); }
};

template <typename T>
sf_inline Slice<T> slice(T *data, size_t size) { return Slice<T>(data, size); }

template <typename T, size_t N>
sf_inline Slice<T> slice(T (&data)[N]) { return Slice<T>(data, N); }

template <typename T, typename U>
static T *find(Slice<T> arr, const U &t)
{
	for (T &other : arr) {
		if (t == other) {
			return &other;
		}
	}
	return nullptr;
}

template <typename T>
static void reverse(Slice<T> arr)
{
	T *first = arr.data, *last = arr.data + arr.size;
	while (first > last) {
	}
}

template <typename T>
struct InsertResult
{
	T &entry;
	bool inserted;
};

enum UninitType { Uninit };
enum ConstType { Const };

struct TypeInfo
{
	uint32_t size;
	ConstructRangeFn constructRange;
	MoveRangeFn moveRange;
	DestructRangeFn destructRange;
};

template <typename T>
inline constexpr TypeInfo getTypeInfo() {
	return { sizeof(T), constructRangeImp<T>, moveRangeImp<T>, destructRangeImp<T> };
}

struct Type;

static const constexpr uint32_t MaxTypeStructSize = 128;

template <typename T>
void initType(Type *t);

void initCPointerType(Type *dst, Type *type);
void initCArrayType(Type *dst, const TypeInfo &info, Type *type, size_t size);

bool beginTypeInit(uint32_t *flag);
void endTypeInit(Type *type);
void waitForTypeInit();

template <typename T>
struct InitType {
	static void init(Type *t) { initType<T>(t); }
};

template <typename T>
inline Type *typeOfRecursive() {
	static uint32_t initFlag;
	alignas(16) static char storage[MaxTypeStructSize];
	if (beginTypeInit(&initFlag)) {
		InitType<T>::init((Type*)storage);
		endTypeInit((Type*)storage);
	}
	return (Type*)storage;
}

template <typename T>
inline Type *typeOf() {
	Type *t = typeOfRecursive<typename std::remove_const<T>::type>();
	waitForTypeInit();
	return t;
}

template <typename T>
struct InitType<T*> {
	static void init(Type *t) { initCPointerType(t, typeOf<T>()); }
};

template <typename T, size_t N>
struct InitType<T[N]> {
	static void init(Type *t) {
		TypeInfo info = {
			sizeof(T[N]),
			[](void *data, size_t size){ getTypeInfo<T>().constructRange(data, N * size); },
			[](void *dst, void *src, size_t size){ getTypeInfo<T>().moveRange(dst, src, N * size); },
			[](void *data, size_t size){ getTypeInfo<T>().destructRange(data, N * size); },
		};
		initCArrayType(t, info, typeOf<T>(), N);
	}
};

}

#if SF_CC_MSC
	#pragma warning(pop)
#endif
