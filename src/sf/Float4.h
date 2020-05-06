#pragma once

#include "sf/Base.h"

#if SF_ARCH_WASM
	#include <wasm_simd128.h>
#elif SF_ARCH_X86
	#include <xmmintrin.h>
	#include <emmintrin.h>
#else
	#include <math.h>
#endif

namespace sf {

#if SF_ARCH_WASM

struct Float4
{
	v128_t imp;

	sf_forceinline static Float4 zero() { return wasm_f32x4_const(0.0f,0.0f,0.0f,0.0f)); }; }
	sf_forceinline static Float4 loadu(const float *ptr) { return wasm_v128_load(ptr); }
	sf_forceinline void storeu(float *ptr) const { wasm_v128_store(ptr, imp); }

	sf_forceinline Float4() { }
	sf_forceinline Float4(float f) : imp(wasm_f32x4_splat(f)) { }
	sf_forceinline Float4(v128_t m) : imp(m) { }
	sf_forceinline Float4(float a, float b, float c, float d) : imp(wasm_f32x4_make(a, b, c, d)) { }
	sf_forceinline Float4 operator+(const Float4 &rhs) const { return wasm_f32x4_add(imp, rhs.imp); }
	sf_forceinline Float4 operator-(const Float4 &rhs) const { return wasm_f32x4_sub(imp, rhs.imp); }
	sf_forceinline Float4 operator*(const Float4 &rhs) const { return wasm_f32x4_mul(imp, rhs.imp); }
	sf_forceinline Float4 operator/(const Float4 &rhs) const { return wasm_f32x4_div(imp, rhs.imp); }
	sf_forceinline Float4 &operator+=(const Float4 &rhs) { imp = wasm_f32x4_add(imp, rhs.imp); return *this; }
	sf_forceinline Float4 &operator-=(const Float4 &rhs) { imp = wasm_f32x4_sub(imp, rhs.imp); return *this; }
	sf_forceinline Float4 &operator*=(const Float4 &rhs) { imp = wasm_f32x4_mul(imp, rhs.imp); return *this; }
	sf_forceinline Float4 &operator/=(const Float4 &rhs) { imp = wasm_f32x4_div(imp, rhs.imp); return *this; }
	sf_forceinline Float4 broadcastX() const { return wasm_v32x4_shuffle(imp, imp, 0,0,0,0); }
	sf_forceinline Float4 broadcastY() const { return wasm_v32x4_shuffle(imp, imp, 1,1,1,1); }
	sf_forceinline Float4 broadcastZ() const { return wasm_v32x4_shuffle(imp, imp, 2,2,2,2); }
	sf_forceinline Float4 broadcastW() const { return wasm_v32x4_shuffle(imp, imp, 3,3,3,3); }
	sf_forceinline Float4 sqrt() const { return wasm_f32x4_sqrt(imp); }
	sf_forceinline Float4 abs() const { return wasm_f32x4_abs(imp); }
	sf_forceinline Float4 min(const Float4 &rhs) const { return wasm_f32x4_min(imp, rhs.imp); }
	sf_forceinline Float4 max(const Float4 &rhs) const { return wasm_f32x4_max(imp, rhs.imp); }
	sf_forceinline bool anyGreaterThanZero() const { return wasm_i32x4_any_true(wasm_f32x4_gt(imp, wasm_f32x4_const(0.0f,0.0f,0.0f,0.0f)); }
	sf_forceinline bool allGreaterThanZero() const { return wasm_i32x4_all_true(wasm_f32x4_gt(imp, wasm_f32x4_const(0.0f,0.0f,0.0f,0.0f)); }

	sf_forceinline static void transpose4(Float4 &a, Float4 &b, Float4 &c, Float4 &d)
	{
		v128_t t0 = wasm_v32x4_shuffle(a.imp, b.imp, 0, 4, 1, 5); // XXYY
		v128_t t1 = wasm_v32x4_shuffle(a.imp, b.imp, 2, 6, 3, 7); // ZZWW
		v128_t t2 = wasm_v32x4_shuffle(c.imp, d.imp, 0, 4, 1, 5); // XXYY
		v128_t t3 = wasm_v32x4_shuffle(c.imp, d.imp, 2, 6, 3, 7); // ZZWW
		a.imp = wasm_v32x4_shuffle(t0, t2, 0, 1, 4, 5);
		b.imp = wasm_v32x4_shuffle(t0, t2, 2, 3, 6, 7);
		c.imp = wasm_v32x4_shuffle(t1, t3, 0, 1, 4, 5);
		d.imp = wasm_v32x4_shuffle(t1, t3, 2, 3, 6, 7);
	}
};

#elif SF_ARCH_X86

struct Float4
{
	__m128 imp;

	sf_forceinline static Float4 zero() { return _mm_setzero_ps(); }
	sf_forceinline static Float4 loadu(const float *ptr) { return _mm_loadu_ps(ptr); }
	sf_forceinline void storeu(float *ptr) const { _mm_storeu_ps(ptr, imp); }

	sf_forceinline Float4() { }
	sf_forceinline Float4(float f) : imp(_mm_set1_ps(f)) { }
	sf_forceinline Float4(__m128 m) : imp(m) { }
	sf_forceinline Float4(float a, float b, float c, float d) : imp(_mm_set_ps(a, b, c, d)) { }
	sf_forceinline Float4 operator+(const Float4 &rhs) const { return _mm_add_ps(imp, rhs.imp); }
	sf_forceinline Float4 operator-(const Float4 &rhs) const { return _mm_sub_ps(imp, rhs.imp); }
	sf_forceinline Float4 operator*(const Float4 &rhs) const { return _mm_mul_ps(imp, rhs.imp); }
	sf_forceinline Float4 operator/(const Float4 &rhs) const { return _mm_div_ps(imp, rhs.imp); }
	sf_forceinline Float4 &operator+=(const Float4 &rhs) { imp = _mm_add_ps(imp, rhs.imp); return *this; }
	sf_forceinline Float4 &operator-=(const Float4 &rhs) { imp = _mm_sub_ps(imp, rhs.imp); return *this; }
	sf_forceinline Float4 &operator*=(const Float4 &rhs) { imp = _mm_mul_ps(imp, rhs.imp); return *this; }
	sf_forceinline Float4 &operator/=(const Float4 &rhs) { imp = _mm_div_ps(imp, rhs.imp); return *this; }
	sf_forceinline Float4 broadcastX() const { return _mm_shuffle_ps(imp, imp, _MM_SHUFFLE(0,0,0,0)); }
	sf_forceinline Float4 broadcastY() const { return _mm_shuffle_ps(imp, imp, _MM_SHUFFLE(1,1,1,1)); }
	sf_forceinline Float4 broadcastZ() const { return _mm_shuffle_ps(imp, imp, _MM_SHUFFLE(2,2,2,2)); }
	sf_forceinline Float4 broadcastW() const { return _mm_shuffle_ps(imp, imp, _MM_SHUFFLE(3,3,3,3)); }
	sf_forceinline Float4 sqrt() const { return _mm_sqrt_ps(imp); }
	sf_forceinline Float4 abs() const { return _mm_andnot_ps(_mm_set1_ps(-0.0f), imp); }
	sf_forceinline Float4 min(const Float4 &rhs) const { return _mm_min_ps(imp, rhs.imp); }
	sf_forceinline Float4 max(const Float4 &rhs) const { return _mm_max_ps(imp, rhs.imp); }
	sf_forceinline bool anyGreaterThanZero() const { return _mm_movemask_ps(_mm_cmpgt_ps(imp, _mm_setzero_ps())) != 0; }
	sf_forceinline bool allGreaterThanZero() const { return _mm_movemask_ps(_mm_cmpgt_ps(imp, _mm_setzero_ps())) == 0xf; }

	sf_forceinline static void transpose4(Float4 &a, Float4 &b, Float4 &c, Float4 &d)
	{
		__m128 t0 = _mm_unpacklo_ps(a.imp, b.imp); // XXYY
		__m128 t1 = _mm_unpackhi_ps(a.imp, b.imp); // ZZWW
		__m128 t2 = _mm_unpacklo_ps(c.imp, d.imp); // XXYY
		__m128 t3 = _mm_unpackhi_ps(c.imp, d.imp); // ZZWW
		a.imp = _mm_movelh_ps(t0, t2);
		b.imp = _mm_movehl_ps(t2, t0);
		c.imp = _mm_movelh_ps(t1, t3);
		d.imp = _mm_movehl_ps(t3, t1);
	}
};

#else

struct Float4
{
	float imp[4];

	sf_forceinline static Float4 zero() { return { 0.0f, 0.0f, 0.0f, 0.0f }; }
	sf_forceinline static Float4 loadu(const float *ptr) { return { ptr[0], ptr[1], ptr[2], ptr[3] }; }
	sf_forceinline void storeu(float *ptr) const { ptr[0] = imp[0]; ptr[1] = imp[1]; ptr[2] = imp[2]; ptr[3] = imp[3]; }

	sf_forceinline Float4() { }
	sf_forceinline Float4(float f) { imp[0] = f; imp[1] = f; imp[2] = f; imp[3] = f; }
	sf_forceinline Float4(float a, float b, float c, float d) { imp[0] = a; imp[1] = b; imp[2] = c; imp[3] = d; }
	sf_forceinline Float4 operator+(const Float4 &rhs) const { return { imp[0] + rhs.imp[0], imp[1] + rhs.imp[1], imp[2] + rhs.imp[2], imp[3] + rhs.imp[3] }; }
	sf_forceinline Float4 operator-(const Float4 &rhs) const { return { imp[0] - rhs.imp[0], imp[1] - rhs.imp[1], imp[2] - rhs.imp[2], imp[3] - rhs.imp[3] }; }
	sf_forceinline Float4 operator*(const Float4 &rhs) const { return { imp[0] * rhs.imp[0], imp[1] * rhs.imp[1], imp[2] * rhs.imp[2], imp[3] * rhs.imp[3] }; }
	sf_forceinline Float4 operator/(const Float4 &rhs) const { return { imp[0] / rhs.imp[0], imp[1] / rhs.imp[1], imp[2] / rhs.imp[2], imp[3] / rhs.imp[3] }; }
	sf_forceinline Float4 &operator+=(const Float4 &rhs) { imp[0] += rhs.imp[0]; imp[1] += rhs.imp[1]; imp[2] += rhs.imp[2]; imp[3] += rhs.imp[3]; return *this; }
	sf_forceinline Float4 &operator-=(const Float4 &rhs) { imp[0] -= rhs.imp[0]; imp[1] -= rhs.imp[1]; imp[2] -= rhs.imp[2]; imp[3] -= rhs.imp[3]; return *this; }
	sf_forceinline Float4 &operator*=(const Float4 &rhs) { imp[0] *= rhs.imp[0]; imp[1] *= rhs.imp[1]; imp[2] *= rhs.imp[2]; imp[3] *= rhs.imp[3]; return *this; }
	sf_forceinline Float4 &operator/=(const Float4 &rhs) { imp[0] /= rhs.imp[0]; imp[1] /= rhs.imp[1]; imp[2] /= rhs.imp[2]; imp[3] /= rhs.imp[3]; return *this; }
	sf_forceinline Float4 broadcastX() const { return { imp[0], imp[0], imp[0], imp[0] }; }
	sf_forceinline Float4 broadcastY() const { return { imp[1], imp[1], imp[1], imp[1] }; }
	sf_forceinline Float4 broadcastZ() const { return { imp[2], imp[2], imp[2], imp[2] }; }
	sf_forceinline Float4 broadcastW() const { return { imp[3], imp[3], imp[3], imp[3] }; }
	sf_forceinline Float4 sqrt() const { return { sqrtf(imp[0]), sqrtf(imp[1]), sqrtf(imp[2]), sqrtf(imp[3]) }; }
	sf_forceinline Float4 abs() const { return { fabsf(imp[0]), fabsf(imp[1]), fabsf(imp[2]), fabsf(imp[3]) }; }
	sf_forceinline Float4 min(const Float4 &rhs) const { return { imp[0]<rhs.imp[0]?imp[0]:rhs.imp[0], imp[1]<rhs.imp[1]?imp[1]:rhs.imp[1], imp[2]<rhs.imp[2]?imp[2]:rhs.imp[2], imp[3]<rhs.imp[3]?imp[3]:rhs.imp[3] }; }
	sf_forceinline Float4 max(const Float4 &rhs) const { return { imp[0]<rhs.imp[0]?rhs.imp[0]:imp[0], imp[1]<rhs.imp[1]?rhs.imp[1]:imp[1], imp[2]<rhs.imp[2]?rhs.imp[2]:imp[2], imp[3]<rhs.imp[3]?rhs.imp[3]:imp[3] }; }
	sf_forceinline bool anyGreaterThanZero() const { return imp[0]>0.0f || imp[1]>0.0f || imp[2]>0.0f || imp[3]>0.0f; }
	sf_forceinline bool allGreaterThanZero() const { return imp[0]>0.0f && imp[1]>0.0f && imp[2]>0.0f && imp[3]>0.0f; }

	sf_forceinline static void transpose4(Float4 &a, Float4 &b, Float4 &c, Float4 &d)
	{
		float t;
		t = a.imp[1]; a.imp[1] = b.imp[0]; b.imp[0] = t;
		t = a.imp[2]; a.imp[2] = c.imp[0]; c.imp[0] = t;
		t = a.imp[3]; a.imp[3] = d.imp[0]; d.imp[0] = t;
		t = b.imp[2]; b.imp[2] = c.imp[1]; c.imp[1] = t;
		t = b.imp[3]; b.imp[3] = d.imp[1]; d.imp[1] = t;
		t = c.imp[3]; c.imp[3] = d.imp[2]; d.imp[2] = t;
	}
};

#endif

}
