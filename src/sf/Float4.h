#pragma once

#include "sf/Base.h"
#include "sf/Vector.h"

#ifndef SF_FLOAT4_FORCE_SCALAR
#define SF_FLOAT4_FORCE_SCALAR 1
#endif

#if SF_ARCH_WASM && SF_WASM_USE_SIMD && !SF_FLOAT4_FORCE_SCALAR
	#include <wasm_simd128.h>
#elif SF_ARCH_X86 && !SF_FLOAT4_FORCE_SCALAR
	#include <xmmintrin.h>
	#include <emmintrin.h>
	#include <nmmintrin.h>
#elif SF_ARCH_ARM && !SF_FLOAT4_FORCE_SCALAR
	#include <arm_neon.h>
#else
	#define SF_FLOAT4_SCALAR 1
	#include <math.h>
#endif

#ifndef SF_FLOAT4_SCALAR
#define SF_FLOAT4_SCALAR 0
#endif

namespace sf {

#if SF_ARCH_WASM && SF_WASM_USE_SIMD && !SF_FLOAT4_FORCE_SCALAR

struct Float4
{
	v128_t imp;

	static sf_forceinline Float4 zero() { return wasm_f32x4_const(0.0f,0.0f,0.0f,0.0f); }
	static sf_forceinline Float4 loadu(const float *ptr) { return wasm_v128_load(ptr); }
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
	sf_forceinline Float4 rotateLeft() const { return wasm_v32x4_shuffle(imp, imp, 1,2,3,0); }
	sf_forceinline Float4 clearW() const { return wasm_v32x4_replace_lane(imp, 0, 0.0f); }

	sf_forceinline float getX() const { return wasm_f32x4_extract_lane(imp, 0); }
	sf_forceinline float getY() const { return wasm_f32x4_extract_lane(imp, 1); }
	sf_forceinline float getZ() const { return wasm_f32x4_extract_lane(imp, 2); }
	sf_forceinline float getW() const { return wasm_f32x4_extract_lane(imp, 3); }

	sf_forceinline Float4 sqrt() const { return wasm_f32x4_sqrt(imp); }
	sf_forceinline Float4 rsqrt() const { return Float4(1.0f) / wasm_f32x4_sqrt(imp); }
	sf_forceinline Float4 abs() const { return wasm_f32x4_abs(imp); }
	sf_forceinline Float4 round() const {
		float a = __builtin_rintf(wasm_f32x4_extract_lane(imp, 0));
		float b = __builtin_rintf(wasm_f32x4_extract_lane(imp, 1));
		float c = __builtin_rintf(wasm_f32x4_extract_lane(imp, 2));
		float d = __builtin_rintf(wasm_f32x4_extract_lane(imp, 3));
		return wasm_f32x4_make(a, b, c, d);
	}
	sf_forceinline Float4 min(const Float4 &rhs) const { return wasm_f32x4_min(imp, rhs.imp); }
	sf_forceinline Float4 max(const Float4 &rhs) const { return wasm_f32x4_max(imp, rhs.imp); }

	sf_forceinline float reduceMin() const {
		__m128 t0 = wasm_f32x4_min(imp, wasm_v32x4_shuffle(imp, imp, 2,3,0,1));
		return wasm_f32x4_extract_lane(wasm_f32x4_min(t0, wasm_v32x4_shuffle(t0, t0, 1,0,3,2)));
	}
	sf_forceinline float reduceMax() const {
		__m128 t0 = wasm_f32x4_max(imp, wasm_v32x4_shuffle(imp, imp, 2,3,0,1));
		return wasm_f32x4_extract_lane(wasm_f32x4_max(t0, wasm_v32x4_shuffle(t0, t0, 1,0,3,2)));
	}

	sf_forceinline bool anyGreaterThanZero() const { return wasm_i32x4_any_true(wasm_f32x4_gt(imp, wasm_f32x4_const(0.0f,0.0f,0.0f,0.0f))); }
	sf_forceinline bool allGreaterThanZero() const { return wasm_i32x4_all_true(wasm_f32x4_gt(imp, wasm_f32x4_const(0.0f,0.0f,0.0f,0.0f))); }

	sf_forceinline Vec3 asVec3() const {
		float a = wasm_f32x4_extract_lane(imp, 0);
		float b = wasm_f32x4_extract_lane(imp, 1);
		float c = wasm_f32x4_extract_lane(imp, 2);
		return sf::Vec3(a, b, c);
	}

	static sf_forceinline void interleave2(Float4 &da, Float4 &db, const Float4 &a, const Float4 &b)
	{
		da = wasm_v32x4_shuffle(a.imp, b.imp, 0, 4, 1, 5); // XXYY
		db = wasm_v32x4_shuffle(a.imp, b.imp, 2, 6, 3, 7); // ZZWW
	}

	static sf_forceinline void transpose22(Float4 &a, Float4 &b)
	{
		v128_t t0 = wasm_v32x4_shuffle(a.imp, b.imp, 0, 1, 4, 5);
		v128_t t1 = wasm_v32x4_shuffle(a.imp, b.imp, 6, 7, 2, 3);
		a.imp = t0;
		b.imp = t1;
	}

	static sf_forceinline void transpose4(Float4 &a, Float4 &b, Float4 &c, Float4 &d)
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

	static sf_forceinline void load8xi16(Float4 &a, Float4 &b, const int16_t *src)
	{
		v128_t s16 = wasm_v128_load(src);
		v128_t lo32 = wasm_i32x4_widen_low_i16x8(src);
		v128_t hi32 = wasm_i32x4_widen_high_i16x8(src);
		a = wasm_f32x4_convert_i32x4(lo32);
		b = wasm_f32x4_convert_i32x4(hi32);
	}
};

using ScalarFloat4 = Float4;
using ScalarAddFloat4 = Float4;

#elif SF_ARCH_X86 && !SF_FLOAT4_FORCE_SCALAR

struct Mask4
{
	__m128 imp;

	sf_forceinline Mask4(__m128 m) : imp(m) { }
};

struct Float4
{
	__m128 imp;

	static sf_forceinline Float4 zero() { return _mm_setzero_ps(); }
	static sf_forceinline Float4 loadu(const float *ptr) { return _mm_loadu_ps(ptr); }
	sf_forceinline void storeu(float *ptr) const { _mm_storeu_ps(ptr, imp); }

	sf_forceinline Float4() { }
	sf_forceinline Float4(float f) : imp(_mm_set1_ps(f)) { }
	sf_forceinline Float4(__m128 m) : imp(m) { }
	sf_forceinline Float4(float a, float b, float c, float d) : imp(_mm_set_ps(d, c, b, a)) { }

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
	sf_forceinline Float4 rotateLeft() const { return _mm_shuffle_ps(imp, imp, _MM_SHUFFLE(0,3,2,1)); }
	sf_forceinline Float4 clearW() const { return _mm_blend_ps(imp, _mm_setzero_ps(), 0x8); }

	sf_forceinline float getX() const { return _mm_cvtss_f32(imp); }
	sf_forceinline float getY() const { return _mm_cvtss_f32(_mm_shuffle_ps(imp, imp, _MM_SHUFFLE(3,2,1,1))); }
	sf_forceinline float getZ() const { return _mm_cvtss_f32(_mm_shuffle_ps(imp, imp, _MM_SHUFFLE(3,2,1,2))); }
	sf_forceinline float getW() const { return _mm_cvtss_f32(_mm_shuffle_ps(imp, imp, _MM_SHUFFLE(3,2,1,3))); }

	sf_forceinline Float4 sqrt() const { return _mm_sqrt_ps(imp); }
	sf_forceinline Float4 rsqrt() const {
		const __m128 mm3 = _mm_set1_ps(3.0f), mmRcp2 = _mm_set1_ps(0.5f);
		__m128 e = _mm_rsqrt_ps(imp);
		e = _mm_mul_ps(_mm_mul_ps(mmRcp2, e), _mm_sub_ps(mm3, _mm_mul_ps(_mm_mul_ps(e, e), imp)));
		// e = _mm_mul_ps(_mm_mul_ps(mmRcp2, e), _mm_sub_ps(mm3, _mm_mul_ps(_mm_mul_ps(e, e), imp)));
		return e;
	}
	sf_forceinline Float4 abs() const { return _mm_andnot_ps(_mm_set1_ps(-0.0f), imp); }
	sf_forceinline Float4 round() const { return _mm_round_ps(imp, _MM_FROUND_TO_NEAREST_INT |_MM_FROUND_NO_EXC); }
	sf_forceinline Float4 min(const Float4 &rhs) const { return _mm_min_ps(imp, rhs.imp); }
	sf_forceinline Float4 max(const Float4 &rhs) const { return _mm_max_ps(imp, rhs.imp); }

	sf_forceinline float reduceMin() const {
		__m128 t0 = _mm_min_ps(imp, _mm_movehl_ps(imp, imp));
		return _mm_cvtss_f32(_mm_min_ps(t0, _mm_shuffle_ps(t0, t0, _MM_SHUFFLE(3,2,1,1))));
	}

	sf_forceinline float reduceMax() const {
		__m128 t0 = _mm_max_ps(imp, _mm_movehl_ps(imp, imp));
		return _mm_cvtss_f32(_mm_max_ps(t0, _mm_shuffle_ps(t0, t0, _MM_SHUFFLE(3,2,1,1))));
	}

	sf_forceinline bool anyGreaterThanZero() const { return _mm_movemask_ps(_mm_cmpgt_ps(imp, _mm_setzero_ps())) != 0; }
	sf_forceinline bool allGreaterThanZero() const { return _mm_movemask_ps(_mm_cmpgt_ps(imp, _mm_setzero_ps())) == 0xf; }

	sf_forceinline Mask4 compareLess(const Float4 &rhs) const { return _mm_cmplt_ps(imp, rhs.imp); }
	sf_forceinline Float4 selectOrZero(const Mask4 &rhs) const { return _mm_and_ps(imp, rhs.imp); }

	sf_forceinline Vec3 asVec3() const {
		float a = _mm_cvtss_f32(imp);
		float b = _mm_cvtss_f32(_mm_shuffle_ps(imp, imp, _MM_SHUFFLE(3,2,1,1)));
		float c = _mm_cvtss_f32(_mm_shuffle_ps(imp, imp, _MM_SHUFFLE(3,2,1,2)));
		return sf::Vec3(a, b, c);
	}

	static sf_forceinline void interleave2(Float4 &da, Float4 &db, const Float4 &a, const Float4 &b)
	{
		da = _mm_unpacklo_ps(a.imp, b.imp); // XXYY
		db = _mm_unpackhi_ps(a.imp, b.imp); // ZZWW
	}

	static sf_forceinline void transpose22(Float4 &a, Float4 &b)
	{
		__m128 t0 = a.imp;
		__m128 t1 = b.imp;
		a.imp = _mm_movelh_ps(t0, t1);
		b.imp = _mm_movehl_ps(t1, t0);
	}

	static sf_forceinline void transpose4(Float4 &a, Float4 &b, Float4 &c, Float4 &d)
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

	static sf_forceinline void load8xi16(Float4 &a, Float4 &b, const int16_t *src)
	{
		__m128i s16 = _mm_loadu_si128((const __m128i*)src);
		__m128i lo32 = _mm_cvtepi16_epi32(s16);
		__m128i hi32 = _mm_cvtepi16_epi32(_mm_srli_si128(s16, 8));
		a = _mm_cvtepi32_ps(lo32);
		b = _mm_cvtepi32_ps(hi32);
	}
};

using ScalarFloat4 = Float4;
using ScalarAddFloat4 = Float4;

#elif SF_ARCH_ARM && !SF_FLOAT4_FORCE_SCALAR

struct Float4
{
	float32x4_t imp;

	static sf_forceinline Float4 zero() { return vdupq_n_f32(0.0f); }
	static sf_forceinline Float4 loadu(const float *ptr) { return vld1q_f32(ptr); }
	sf_forceinline void storeu(float *ptr) const { vst1q_f32(ptr, imp); }

	sf_forceinline Float4() { }
	sf_forceinline Float4(float f) : imp(vdupq_n_f32(f)) { }
	sf_forceinline Float4(float32x4_t m) : imp(m) { }
	sf_forceinline Float4(float a, float b, float c, float d) {
		alignas(16) float v[4] = { a, b, c, d };
		imp = vld1q_f32(v);
	}
	sf_forceinline Float4 operator+(const Float4 &rhs) const { return vaddq_f32(imp, rhs.imp); }
	sf_forceinline Float4 operator-(const Float4 &rhs) const { return vsubq_f32(imp, rhs.imp); }
	sf_forceinline Float4 operator*(const Float4 &rhs) const { return vmulq_f32(imp, rhs.imp); }
	sf_forceinline Float4 operator/(const Float4 &rhs) const {
		float32x4_t e = vrecpeq_f32(rhs.imp);
		e = vmulq_f32(vrecpsq_f32(rhs.imp, e), e);
		e = vmulq_f32(vrecpsq_f32(rhs.imp, e), e);
		return vmulq_f32(imp, e);
	}
	sf_forceinline Float4 &operator+=(const Float4 &rhs) { imp = vaddq_f32(imp, rhs.imp); return *this; }
	sf_forceinline Float4 &operator-=(const Float4 &rhs) { imp = vsubq_f32(imp, rhs.imp); return *this; }
	sf_forceinline Float4 &operator*=(const Float4 &rhs) { imp = vmulq_f32(imp, rhs.imp); return *this; }
	sf_forceinline Float4 &operator/=(const Float4 &rhs) {
		float32x4_t e = vrecpeq_f32(rhs.imp);
		e = vmulq_f32(vrecpsq_f32(rhs.imp, e), e);
		e = vmulq_f32(vrecpsq_f32(rhs.imp, e), e);
		imp = vmulq_f32(imp, e);
		return *this;
	}
	sf_forceinline Float4 broadcastX() const { return vdupq_laneq_f32(imp, 0); }
	sf_forceinline Float4 broadcastY() const { return vdupq_laneq_f32(imp, 1); }
	sf_forceinline Float4 broadcastZ() const { return vdupq_laneq_f32(imp, 2); }
	sf_forceinline Float4 broadcastW() const { return vdupq_laneq_f32(imp, 3); }
	sf_forceinline Float4 rotateLeft() const { return vextq_f32(imp, imp, 1); }
	sf_forceinline Float4 clearW() const { return vsetq_lane_f32(0.0f, imp, 3); }

	sf_forceinline float getX() const { return vgetq_lane_f32(imp, 0); }
	sf_forceinline float getY() const { return vgetq_lane_f32(imp, 1); }
	sf_forceinline float getZ() const { return vgetq_lane_f32(imp, 2); }
	sf_forceinline float getW() const { return vgetq_lane_f32(imp, 3); }

	sf_forceinline Float4 rsqrt() const {
		float32x4_t e = vrsqrteq_f32(imp);
		e = vmulq_f32(vrsqrtsq_f32(vmulq_f32(e, e), imp), e);
		// e = vmulq_f32(vrsqrtsq_f32(vmulq_f32(e, e), imp), e);
		return e;
	}
	sf_forceinline Float4 sqrt() const {
		float32x4_t e = vrsqrteq_f32(imp);
		e = vmulq_f32(vrsqrtsq_f32(vmulq_f32(e, e), imp), e);
		e = vmulq_f32(vrsqrtsq_f32(vmulq_f32(e, e), imp), e);
		float32x4_t e2 = vrecpeq_f32(e);
		e2 = vmulq_f32(vrecpsq_f32(e, e2), e2);
		e2 = vmulq_f32(vrecpsq_f32(e, e2), e2);
		return e2;
	}
	sf_forceinline Float4 abs() const { return vabsq_f32(imp); }
	sf_forceinline Float4 round() const { return vrndnq_f32(imp); }
	sf_forceinline Float4 min(const Float4 &rhs) const { return vminq_f32(imp, rhs.imp); }
	sf_forceinline Float4 max(const Float4 &rhs) const { return vmaxq_f32(imp, rhs.imp); }

	sf_forceinline float reduceMin() const { return vminvq_f32(imp); }
	sf_forceinline float reduceMax() const { return vmaxvq_f32(imp); }

	sf_forceinline bool anyGreaterThanZero() const {
		uint64x2_t b = vreinterpretq_u64_u32(vcgtq_f32(imp, vdupq_n_f32(0.0f)));
		return (vgetq_lane_u64(b, 0) | vgetq_lane_u64(b, 1)) != 0;
	}
	sf_forceinline bool allGreaterThanZero() const {
		uint64x2_t b = vreinterpretq_u64_u32(vcleq_f32(imp, vdupq_n_f32(0.0f)));
		return (vgetq_lane_u64(b, 0) | vgetq_lane_u64(b, 1)) == 0;
	}

	sf_forceinline Vec3 asVec3() const {
		float a = vgetq_lane_f32(imp, 0);
		float b = vgetq_lane_f32(imp, 1);
		float c = vgetq_lane_f32(imp, 2);
		return sf::Vec3(a, b, c);
	}

	static sf_forceinline void interleave2(Float4 &da, Float4 &db, Float4 &a, Float4 &b)
	{
		float32x4x2_t ab = vzipq_f32(a.imp, b.imp);
		da.imp = ab.val[0];
		db.imp = ab.val[1];
	}

	static sf_forceinline void transpose22(Float4 &a, Float4 &b)
	{
		float32x4_t t0 = vcombine_f32(vget_low_f32(a), vget_low_f32(b)));
		float32x4_t t1 = vcombine_f32(vget_high_f32(a), vget_high_f32(b)));
		a = t0;
		b = t1;
	}

	static sf_forceinline void transpose4(Float4 &a, Float4 &b, Float4 &c, Float4 &d)
	{
		float32x4x2_t ab = vtrnq_f32(a.imp, b.imp);
		float32x4x2_t cd = vtrnq_f32(c.imp, d.imp);
		a.imp = vcombine_f32(vget_low_f32(ab.val[0]), vget_low_f32(cd.val[0]));
		b.imp = vcombine_f32(vget_low_f32(ab.val[1]), vget_low_f32(cd.val[1]));
		c.imp = vcombine_f32(vget_high_f32(ab.val[0]), vget_high_f32(cd.val[0]));
		d.imp = vcombine_f32(vget_high_f32(ab.val[1]), vget_high_f32(cd.val[1]));
	}

	// NEON optimizations
	sf_forceinline Float4 operator*(float rhs) const { return vmulq_n_f32(imp, rhs); }
	sf_forceinline Float4 &operator*=(float rhs) { imp = vmulq_n_f32(imp, rhs); return *this; }

	static sf_forceinline void load8xi16(Float4 &a, Float4 &b, const int16_t *src)
	{
		int16x8_t s16 = vld1q_s16(src);
		__m128i lo32 = vmovl_s16(s16);
		__m128i hi32 = vmovl_s16(vextq_s16(s16, s16, 4));
		a = vcvt_s32_f32(lo32);
		b = vcvt_s32_f32(hi32);
	}
};

using ScalarFloat4 = float;
using ScalarAddFloat4 = Float4;

#else

struct Float4
{
	float a, b, c, d;

	static sf_forceinline Float4 zero() { return { 0.0f, 0.0f, 0.0f, 0.0f }; }
	static sf_forceinline Float4 loadu(const float *ptr) { return { ptr[0], ptr[1], ptr[2], ptr[3] }; }
	sf_forceinline void storeu(float *ptr) const { ptr[0] = a; ptr[1] = b; ptr[2] = c; ptr[3] = d; }

	sf_forceinline Float4() { }
	sf_forceinline Float4(float f) : a(f), b(f), c(f), d(f) { }
	sf_forceinline Float4(float a, float b, float c, float d) : a(a), b(b), c(c), d(d) { }
	sf_forceinline Float4 operator+(const Float4 &rhs) const { return { a + rhs.a, b + rhs.b, c + rhs.c, d + rhs.d }; }
	sf_forceinline Float4 operator-(const Float4 &rhs) const { return { a - rhs.a, b - rhs.b, c - rhs.c, d - rhs.d }; }
	sf_forceinline Float4 operator*(const Float4 &rhs) const { return { a * rhs.a, b * rhs.b, c * rhs.c, d * rhs.d }; }
	sf_forceinline Float4 operator*(float rhs) const { return { a * rhs, b * rhs, c * rhs, d * rhs }; }
	sf_forceinline Float4 operator/(const Float4 &rhs) const { return { a / rhs.a, b / rhs.b, c / rhs.c, d / rhs.d }; }
	sf_forceinline Float4 &operator+=(const Float4 &rhs) { a += rhs.a; b += rhs.b; c += rhs.c; d += rhs.d; return *this; }
	sf_forceinline Float4 &operator-=(const Float4 &rhs) { a -= rhs.a; b -= rhs.b; c -= rhs.c; d -= rhs.d; return *this; }
	sf_forceinline Float4 &operator*=(const Float4 &rhs) { a *= rhs.a; b *= rhs.b; c *= rhs.c; d *= rhs.d; return *this; }
	sf_forceinline Float4 &operator/=(const Float4 &rhs) { a /= rhs.a; b /= rhs.b; c /= rhs.c; d /= rhs.d; return *this; }

	sf_forceinline Float4 broadcastX() const { return { a, a, a, a }; }
	sf_forceinline Float4 broadcastY() const { return { b, b, b, b }; }
	sf_forceinline Float4 broadcastZ() const { return { c, c, c, c }; }
	sf_forceinline Float4 broadcastW() const { return { d, d, d, d }; }
	sf_forceinline Float4 rotateLeft() const { return { b, c, d, a }; }
	sf_forceinline Float4 clearW() const { return { a, b, c, 0.0f }; }

	sf_forceinline float getX() const { return a; }
	sf_forceinline float getY() const { return b; }
	sf_forceinline float getZ() const { return c; }
	sf_forceinline float getW() const { return d; }

	#if SF_CC_GNU || SF_CC_CLANG
		sf_forceinline Float4 sqrt() const { return { __builtin_sqrtf(a), __builtin_sqrtf(b), __builtin_sqrtf(c), __builtin_sqrtf(d) }; }
		sf_forceinline Float4 rsqrt() const { return { 1.0f / __builtin_sqrtf(a), 1.0f / __builtin_sqrtf(b), 1.0f / __builtin_sqrtf(c), 1.0f / __builtin_sqrtf(d) }; }
		sf_forceinline Float4 abs() const { return { __builtin_fabsf(a), __builtin_fabsf(b), __builtin_fabsf(c), __builtin_fabsf(d) }; }
		#if SF_ARCH_WASM
			sf_forceinline Float4 round() const { return { __builtin_rintf(a), __builtin_rintf(b), __builtin_rintf(c), __builtin_rintf(d) }; }
		#else
			sf_forceinline Float4 round() const { return { __builtin_roundf(a), __builtin_roundf(b), __builtin_roundf(c), __builtin_roundf(d) }; }
		#endif
	#else
		sf_forceinline Float4 sqrt() const { return { sqrtf(a), sqrtf(b), sqrtf(c), sqrtf(d) }; }
		sf_forceinline Float4 rsqrt() const { return { 1.0f / sqrtf(a), 1.0f / sqrtf(b), 1.0f / sqrtf(c), 1.0f / sqrtf(d) }; }
		sf_forceinline Float4 abs() const { return { fabsf(a), fabsf(b), fabsf(c), fabsf(d) }; }
		sf_forceinline Float4 round() const { return { roundf(a), roundf(b), roundf(c), roundf(d) }; }
	#endif

	sf_forceinline Float4 min(const Float4 &rhs) const { return { a<rhs.a?a:rhs.a, b<rhs.b?b:rhs.b, c<rhs.c?c:rhs.c, d<rhs.d?d:rhs.d }; }
	sf_forceinline Float4 max(const Float4 &rhs) const { return { a<rhs.a?rhs.a:a, b<rhs.b?rhs.b:b, c<rhs.c?rhs.c:c, d<rhs.d?rhs.d:d }; }

	sf_forceinline float reduceMin() const { return sf::min(sf::min(a, b), sf::min(c, d)); }
	sf_forceinline float reduceMax() const { return sf::max(sf::max(a, b), sf::max(c, d)); }

	sf_forceinline bool anyGreaterThanZero() const { return a>0.0f || b>0.0f || c>0.0f || d>0.0f; }
	sf_forceinline bool allGreaterThanZero() const { return a>0.0f && b>0.0f && c>0.0f && d>0.0f; }

	sf_forceinline Vec3 asVec3() const {
		return sf::Vec3(a, b, c);
	}

	static sf_forceinline void interleave2(Float4 &da, Float4 &db, const Float4 &a, const Float4 &b)
	{
		da.a = a.a; da.b = b.a; da.c = a.b; da.d = b.b;
		db.a = a.c; db.b = b.c; db.c = a.d; db.d = b.d;
	}

	static sf_forceinline void transpose22(Float4 &a, Float4 &b)
	{
		float t;
		t = a.c; a.c = b.a; b.a = t;
		t = a.d; a.d = b.b; b.b = t;
	}

	static sf_forceinline void transpose4(Float4 &a, Float4 &b, Float4 &c, Float4 &d)
	{
		float t;
		t = a.b; a.b = b.a; b.a = t;
		t = a.c; a.c = c.a; c.a = t;
		t = a.d; a.d = d.a; d.a = t;
		t = b.c; b.c = c.b; c.b = t;
		t = b.d; b.d = d.b; d.b = t;
		t = c.d; c.d = d.c; d.c = t;
	}

	static sf_forceinline void load8xi16(Float4 &a, Float4 &b, const int16_t *src)
	{
		a.a = (float)src[0];
		a.b = (float)src[1];
		a.c = (float)src[2];
		a.d = (float)src[3];
		b.a = (float)src[4];
		b.b = (float)src[5];
		b.c = (float)src[6];
		b.d = (float)src[7];
	}
};

using ScalarFloat4 = float;
using ScalarAddFloat4 = float;

#endif

sf_inline Float4 horizontalSumXYZ(const Float4 rhs) {
#if SF_FLOAT4_SCALAR
	return rhs.a + rhs.b + rhs.c;
#else
	return rhs.broadcastX() + rhs.broadcastY() + rhs.broadcastZ();
#endif
}

sf_inline Float4 broadcastRcpLengthXYZ(const Float4 rhs) {
#if SF_FLOAT4_SCALAR
	return 1.0f / sf::sqrt(rhs.a*rhs.a + rhs.b*rhs.b + rhs.c*rhs.c);
#else
	return horizontalSumXYZ(rhs * rhs).rsqrt();
#endif
}

sf_inline void setLaneInMemory(Float4 &dst, size_t index, float value) {
	((float*)&dst)[index] = value;
}

}
