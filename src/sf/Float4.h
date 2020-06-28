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

	sf_forceinline bool anyGreaterThanZero() const { return wasm_i32x4_any_true(wasm_f32x4_gt(imp, wasm_f32x4_const(0.0f,0.0f,0.0f,0.0f))); }
	sf_forceinline bool allGreaterThanZero() const { return wasm_i32x4_all_true(wasm_f32x4_gt(imp, wasm_f32x4_const(0.0f,0.0f,0.0f,0.0f))); }

	sf_forceinline Vec3 asVec3() const {
		float a = wasm_f32x4_extract_lane(imp, 0);
		float b = wasm_f32x4_extract_lane(imp, 1);
		float c = wasm_f32x4_extract_lane(imp, 2);
		return sf::Vec3(a, b, c);
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
};

#elif SF_ARCH_X86 && !SF_FLOAT4_FORCE_SCALAR

struct Float4
{
	__m128 imp;

	static sf_forceinline Float4 zero() { return _mm_setzero_ps(); }
	static sf_forceinline Float4 loadu(const float *ptr) { return _mm_loadu_ps(ptr); }
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
	sf_forceinline Float4 rotateLeft() const { return _mm_shuffle_ps(imp, imp, _MM_SHUFFLE(0,3,2,1)); }
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

	sf_forceinline bool anyGreaterThanZero() const { return _mm_movemask_ps(_mm_cmpgt_ps(imp, _mm_setzero_ps())) != 0; }
	sf_forceinline bool allGreaterThanZero() const { return _mm_movemask_ps(_mm_cmpgt_ps(imp, _mm_setzero_ps())) == 0xf; }

	sf_forceinline Vec3 asVec3() const {
		float a = _mm_cvtss_f32(imp);
		float b = _mm_cvtss_f32(_mm_shuffle_ps(imp, imp, _MM_SHUFFLE(3,2,1,1)));
		float c = _mm_cvtss_f32(_mm_shuffle_ps(imp, imp, _MM_SHUFFLE(3,2,1,2)));
		return sf::Vec3(a, b, c);
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
};

#elif SF_ARCH_ARM && !SF_FLOAT4_FORCE_SCALAR

struct Float4
{
	float32x4_t imp;

	static sf_forceinline Float4 zero() { return vdupq_n_f32(0.0f); }
	static sf_forceinline Float4 loadu(const float *ptr) { return vld1q_f32(ptr); }
	sf_forceinline void storeu(float *ptr) const { vst1q_f32(ptr, imp); }

	sf_forceinline Float4() { }
	sf_forceinline Float4(float f) : imp(vdupq_n_f32(f)) { }
	sf_forceinline Float4(v128_t m) : imp(m) { }
	sf_forceinline Float4(float a, float b, float c, float d) {
		alignas(16) float v[4] = { a, b, c, d };
		imp = vld1q_f32(v);
	}
	sf_forceinline Float4 operator+(const Float4 &rhs) const { return vaddq_f32(imp, rhs.imp); }
	sf_forceinline Float4 operator-(const Float4 &rhs) const { return vsubq_f32(imp, rhs.imp); }
	sf_forceinline Float4 operator*(const Float4 &rhs) const { return vmulq_f32(imp, rhs.imp); }
	sf_forceinline Float4 operator/(const Float4 &rhs) const {
		float32x4_t e = vrecpeq_f32(rhs);
		e = vmulq_f32(vrecpsq_f32(rhs, e), e);
		e = vmulq_f32(vrecpsq_f32(rhs, e), e);
		return vmulq_f32(imp, e);
	}
	sf_forceinline Float4 &operator+=(const Float4 &rhs) { imp = vaddq_f32(imp, rhs.imp); return *this; }
	sf_forceinline Float4 &operator-=(const Float4 &rhs) { imp = vsubq_f32(imp, rhs.imp); return *this; }
	sf_forceinline Float4 &operator*=(const Float4 &rhs) { imp = vmulq_f32(imp, rhs.imp); return *this; }
	sf_forceinline Float4 &operator/=(const Float4 &rhs) {
		float32x4_t e = vrecpeq_f32(rhs);
		e = vmulq_f32(vrecpsq_f32(rhs, e), e);
		e = vmulq_f32(vrecpsq_f32(rhs, e), e);
		imp = vmulq_f32(imp, e);
		return *this;
	}
	sf_forceinline Float4 broadcastX() const { return vdupq_lane_f32(imp, 0); }
	sf_forceinline Float4 broadcastY() const { return vdupq_lane_f32(imp, 1); }
	sf_forceinline Float4 broadcastZ() const { return vdupq_lane_f32(imp, 2); }
	sf_forceinline Float4 broadcastW() const { return vdupq_lane_f32(imp, 3); }
	sf_forceinline Float4 rotateLeft() const { return vextq_f32(imp, imp, 1); }
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

	sf_forceinline bool anyGreaterThanZero() const {
		uint64x2_t b = vreinterpretq_u64_f32(vcgtq_f32(imp, vdupq_n_f32(0.0f)));
		return (vgetq_lane_u64(b, 0) | vgetq_lane_u64(b, 1)) != 0;
	}
	sf_forceinline bool allGreaterThanZero() const {
		uint64x2_t b = vreinterpretq_u64_f32(vcleq_f32(imp, vdupq_n_f32(0.0f)));
		return (vgetq_lane_u64(b, 0) | vgetq_lane_u64(b, 1)) == 0;
	}

	sf_forceinline Vec3 asVec3() const {
		float a = vgetq_lane_f32(imp, 0);
		float b = vgetq_lane_f32(imp, 1);
		float c = vgetq_lane_f32(imp, 2);
		return sf::Vec3(a, b, c);
	}

	static sf_forceinline void transpose4(Float4 &a, Float4 &b, Float4 &c, Float4 &d)
	{
		float32x4x2_t ab = vtrnq_f32(a, b);
		float32x4x2_t cd = vtrnq_f32(c, d);
		a.imp = vcombine_f32(vget_low_f32(ab.val[0]), vget_low_f32(cd.val[0]));
		b.imp = vcombine_f32(vget_low_f32(ab.val[1]), vget_low_f32(cd.val[1]));
		c.imp = vcombine_f32(vget_high_f32(ab.val[0]), vget_high_f32(cd.val[0]));
		d.imp = vcombine_f32(vget_high_f32(ab.val[1]), vget_high_f32(cd.val[1]));
	}

	// NEON optimizations
	sf_forceinline Float4 operator*(float rhs) const { return vmulq_n_f32(imp, rhs); }
	sf_forceinline Float4 &operator*=(float rhs) { imp = vmulq_n_f32(imp, rhs); return *this; }
};

#else

struct Float4
{
	float imp[4];

	static sf_forceinline Float4 zero() { return { 0.0f, 0.0f, 0.0f, 0.0f }; }
	static sf_forceinline Float4 loadu(const float *ptr) { return { ptr[0], ptr[1], ptr[2], ptr[3] }; }
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
	sf_forceinline Float4 rotateLeft() const { return { imp[1], imp[2], imp[3], imp[0] }; }

	#if SF_CC_GCC || SF_CC_CLANG
		sf_forceinline Float4 sqrt() const { return { __builtin_sqrtf(imp[0]), __builtin_sqrtf(imp[1]), __builtin_sqrtf(imp[2]), __builtin_sqrtf(imp[3]) }; }
		sf_forceinline Float4 rsqrt() const { return { 1.0f / __builtin_sqrtf(imp[0]), 1.0f / __builtin_sqrtf(imp[1]), 1.0f / __builtin_sqrtf(imp[2]), 1.0f / __builtin_sqrtf(imp[3]) }; }
		sf_forceinline Float4 abs() const { return { __builtin_fabsf(imp[0]), __builtin_fabsf(imp[1]), __builtin_fabsf(imp[2]), __builtin_fabsf(imp[3]) }; }
		sf_forceinline Float4 round() const { return { __builtin_roundf(imp[0]), __builtin_roundf(imp[1]), __builtin_roundf(imp[2]), __builtin_roundf(imp[3]) }; }
	#else
		sf_forceinline Float4 sqrt() const { return { sqrtf(imp[0]), sqrtf(imp[1]), sqrtf(imp[2]), sqrtf(imp[3]) }; }
		sf_forceinline Float4 rsqrt() const { return { 1.0f / sqrtf(imp[0]), 1.0f / sqrtf(imp[1]), 1.0f / sqrtf(imp[2]), 1.0f / sqrtf(imp[3]) }; }
		sf_forceinline Float4 abs() const { return { fabsf(imp[0]), fabsf(imp[1]), fabsf(imp[2]), fabsf(imp[3]) }; }
		sf_forceinline Float4 round() const { return { roundf(imp[0]), roundf(imp[1]), roundf(imp[2]), roundf(imp[3]) }; }
	#endif

	sf_forceinline Float4 min(const Float4 &rhs) const { return { imp[0]<rhs.imp[0]?imp[0]:rhs.imp[0], imp[1]<rhs.imp[1]?imp[1]:rhs.imp[1], imp[2]<rhs.imp[2]?imp[2]:rhs.imp[2], imp[3]<rhs.imp[3]?imp[3]:rhs.imp[3] }; }
	sf_forceinline Float4 max(const Float4 &rhs) const { return { imp[0]<rhs.imp[0]?rhs.imp[0]:imp[0], imp[1]<rhs.imp[1]?rhs.imp[1]:imp[1], imp[2]<rhs.imp[2]?rhs.imp[2]:imp[2], imp[3]<rhs.imp[3]?rhs.imp[3]:imp[3] }; }

	sf_forceinline bool anyGreaterThanZero() const { return imp[0]>0.0f || imp[1]>0.0f || imp[2]>0.0f || imp[3]>0.0f; }
	sf_forceinline bool allGreaterThanZero() const { return imp[0]>0.0f && imp[1]>0.0f && imp[2]>0.0f && imp[3]>0.0f; }

	sf_forceinline Vec3 asVec3() const {
		return sf::Vec3(imp[0], imp[1], imp[2]);
	}

	static sf_forceinline void transpose4(Float4 &a, Float4 &b, Float4 &c, Float4 &d)
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

sf_inline Float4 horizontalSumXYZ(const Float4 rhs) {
#if SF_FLOAT4_SCALAR
	return rhs.imp[0] + rhs.imp[1] + rhs.imp[2];
#else
	return rhs.broadcastX() + rhs.broadcastY() + rhs.broadcastZ();
#endif
}

sf_inline Float4 broadcastRcpLengthXYZ(const Float4 rhs) {
#if SF_FLOAT4_SCALAR
	return 1.0f / sf::sqrt(rhs.imp[0]*rhs.imp[0] + rhs.imp[1]*rhs.imp[1] + rhs.imp[2]*rhs.imp[2]);
#else
	return horizontalSumXYZ(rhs * rhs).rsqrt();
#endif
}

}
