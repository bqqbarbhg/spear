#pragma once

#include "Base.h"

namespace sf {

struct Vec2; struct Vec3; struct Vec4;
struct Vec2i; struct Vec3i; struct Vec4i;

struct Vec2
{
	union {
		struct {
			float x, y;
		};
		float v[2];
	};

	Vec2(): x(0.0f), y(0.0f) { }
	explicit Vec2(float s): x(s), y(s) { }
	Vec2(float x, float y): x(x), y(y) { }
	explicit Vec2(const float *ptr): x(ptr[0]), y(ptr[1]) { }
	explicit Vec2(const Vec2i &rhs);

	Vec2 operator+(const Vec2 &rhs) const { return Vec2(x + rhs.x, y + rhs.y); }
	Vec2 operator-(const Vec2 &rhs) const { return Vec2(x - rhs.x, y - rhs.y); }
	Vec2 operator*(const Vec2 &rhs) const { return Vec2(x * rhs.x, y * rhs.y); }
	Vec2 operator/(const Vec2 &rhs) const { return Vec2(x / rhs.x, y / rhs.y); }

	Vec2 &operator+=(const Vec2 &rhs) { x += rhs.x; y += rhs.y; return *this; }
	Vec2 &operator-=(const Vec2 &rhs) { x -= rhs.x; y -= rhs.y; return *this; }
	Vec2 &operator*=(const Vec2 &rhs) { x *= rhs.x; y *= rhs.y; return *this; }
	Vec2 &operator/=(const Vec2 &rhs) { x /= rhs.x; y /= rhs.y; return *this; }

	Vec2 operator*(float rhs) const { return Vec2(x * rhs, y * rhs); }
	Vec2 operator/(float rhs) const { return Vec2(x / rhs, y / rhs); }

	Vec2 &operator*=(float rhs) { x *= rhs; y *= rhs; return *this; }
	Vec2 &operator/=(float rhs) { x /= rhs; y /= rhs; return *this; }

	Vec2 operator-() const { return Vec2(-x, -y); }

	bool operator==(const Vec2 &rhs) const { return x == rhs.x && y == rhs.y; }
	bool operator!=(const Vec2 &rhs) const { return x != rhs.x || y != rhs.y; }
};

sf_inline float dot(const Vec2 &a, const Vec2 &b) { return a.x*b.x + a.y*b.y; }
sf_inline float lengthSq(const Vec2 &a) { return a.x*a.x + a.y*a.y; }
sf_inline float length(const Vec2 &a) { return sqrt(a.x*a.x + a.y*a.y); }
sf_inline Vec2 normalize(const Vec2 &a) { return a * (1.0f / length(a)); }
sf_inline Vec2 normalizeOrZero(const Vec2 &a) {
	float len = length(a);
	return len > 1e-9 ? a * (1.0f / len) : Vec2();
}
sf_inline Vec2 min(const Vec2 &a, const Vec2 &b) { return Vec2(min(a.x, b.x), min(a.y, b.y)); }
sf_inline Vec2 max(const Vec2 &a, const Vec2 &b) { return Vec2(max(a.x, b.x), max(a.y, b.y)); }
sf_inline Vec2 lerp(const Vec2 &a, const Vec2 &b, float t) { return a * (1.0f - t) + b * t; }
sf_inline Vec2 floor(const Vec2 &a) { return { floorf(a.x), floorf(a.y) }; }
sf_inline Vec2 abs(const Vec2 &a) { return { sf::abs(a.x), sf::abs(a.y) }; }

struct Vec3
{
	union {
		struct {
			float x, y, z;
		};
		float v[3];
	};

	Vec3(): x(0.0f), y(0.0f), z(0.0f) { }
	explicit Vec3(float s): x(s), y(s), z(s) { }
	Vec3(float x, float y, float z): x(x), y(y), z(z) { }
	explicit Vec3(const float *ptr): x(ptr[0]), y(ptr[1]), z(ptr[2]) { }
	explicit Vec3(const Vec3i &rhs);

	Vec3 operator+(const Vec3 &rhs) const { return Vec3(x + rhs.x, y + rhs.y, z + rhs.z); }
	Vec3 operator-(const Vec3 &rhs) const { return Vec3(x - rhs.x, y - rhs.y, z - rhs.z); }
	Vec3 operator*(const Vec3 &rhs) const { return Vec3(x * rhs.x, y * rhs.y, z * rhs.z); }
	Vec3 operator/(const Vec3 &rhs) const { return Vec3(x / rhs.x, y / rhs.y, z / rhs.z); }

	Vec3 &operator+=(const Vec3 &rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
	Vec3 &operator-=(const Vec3 &rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
	Vec3 &operator*=(const Vec3 &rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
	Vec3 &operator/=(const Vec3 &rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }

	Vec3 operator*(float rhs) const { return Vec3(x * rhs, y * rhs, z * rhs); }
	Vec3 operator/(float rhs) const { return Vec3(x / rhs, y / rhs, z / rhs); }

	Vec3 &operator*=(float rhs) { x *= rhs; y *= rhs; z *= rhs; return *this; }
	Vec3 &operator/=(float rhs) { x /= rhs; y /= rhs; z /= rhs; return *this; }

	Vec3 operator-() const { return Vec3(-x, -y, -z); }

	bool operator==(const Vec3 &rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
	bool operator!=(const Vec3 &rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z; }
};

sf_inline float dot(const Vec3 &a, const Vec3 &b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
sf_inline float lengthSq(const Vec3 &a) { return a.x*a.x + a.y*a.y + a.z*a.z; }
sf_inline float length(const Vec3 &a) { return sqrt(a.x*a.x + a.y*a.y + a.z*a.z); }
sf_inline Vec3 normalize(const Vec3 &a) { return a * (1.0f / length(a)); }
sf_inline Vec3 normalizeOrZero(const Vec3 &a) {
	float len = length(a);
	return len > 1e-9 ? a * (1.0f / len) : Vec3();
}
sf_inline Vec3 min(const Vec3 &a, const Vec3 &b) { return Vec3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z)); }
sf_inline Vec3 max(const Vec3 &a, const Vec3 &b) { return Vec3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z)); }
sf_inline Vec3 lerp(const Vec3 &a, const Vec3 &b, float t) { return a * (1.0f - t) + b * t; }
sf_inline Vec3 floor(const Vec3 &a) { return { floorf(a.x), floorf(a.y), floorf(a.z) }; }
sf_inline Vec3 abs(const Vec3 &a) { return { sf::abs(a.x), sf::abs(a.y), sf::abs(a.z) }; }

sf_inline Vec3 cross(const Vec3 &a, const Vec3 &b) {
	return Vec3(
		a.y*b.z - a.z*b.y,
		a.z*b.x - a.x*b.z,
		a.x*b.y - a.y*b.x);
}

struct Vec4
{
	union {
		struct {
			float x, y, z, w;
		};
		float v[4];
	};

	Vec4(): x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }
	explicit Vec4(float s): x(s), y(s), z(s), w(s) { }
	Vec4(float x, float y, float z, float w): x(x), y(y), z(z), w(w) { }
	explicit Vec4(const float *ptr): x(ptr[0]), y(ptr[1]), z(ptr[2]), w(ptr[3]) { }
	explicit Vec4(const Vec4i &rhs);
	Vec4(const Vec3 &xyz, float w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) { }

	Vec4 operator+(const Vec4 &rhs) const { return Vec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
	Vec4 operator-(const Vec4 &rhs) const { return Vec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
	Vec4 operator*(const Vec4 &rhs) const { return Vec4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
	Vec4 operator/(const Vec4 &rhs) const { return Vec4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }

	Vec4 &operator+=(const Vec4 &rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
	Vec4 &operator-=(const Vec4 &rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
	Vec4 &operator*=(const Vec4 &rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
	Vec4 &operator/=(const Vec4 &rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }

	Vec4 operator*(float rhs) const { return Vec4(x * rhs, y * rhs, z * rhs, w * rhs); }
	Vec4 operator/(float rhs) const { return Vec4(x / rhs, y / rhs, z / rhs, w / rhs); }

	Vec4 &operator*=(float rhs) { x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; }
	Vec4 &operator/=(float rhs) { x /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; }

	Vec4 operator-() const { return Vec4(-x, -y, -z, -w); }

	bool operator==(const Vec4 &rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
	bool operator!=(const Vec4 &rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z || w == rhs.w; }
};

sf_inline float dot(const Vec4 &a, const Vec4 &b) { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }
sf_inline float lengthSq(const Vec4 &a) { return a.x*a.x + a.y*a.y + a.z*a.z + a.w*a.w; }
sf_inline float length(const Vec4 &a) { return sqrt(a.x*a.x + a.y*a.y + a.z*a.z + a.w*a.w); }
sf_inline Vec4 normalize(const Vec4 &a) { return a * (1.0f / length(a)); }
sf_inline Vec4 normalizeOrZero(const Vec4 &a) {
	float len = length(a);
	return len > 1e-9 ? a * (1.0f / len) : Vec4();
}
sf_inline Vec4 min(const Vec4 &a, const Vec4 &b) { return Vec4(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z), min(a.w, b.w)); }
sf_inline Vec4 max(const Vec4 &a, const Vec4 &b) { return Vec4(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z), max(a.w, b.w)); }
sf_inline Vec4 lerp(const Vec4 &a, const Vec4 &b, float t) { return a * (1.0f - t) + b * t; }
sf_inline Vec4 floor(const Vec4 &a) { return { floorf(a.x), floorf(a.y), floorf(a.z), floorf(a.w) }; }
sf_inline Vec4 abs(const Vec4 &a) { return { sf::abs(a.x), sf::abs(a.y), sf::abs(a.z), sf::abs(a.w) }; }

sf_inline Vec3 divideW(const Vec4 &v) {
	return Vec3(v.x, v.y, v.z) * (1.0f / v.w);
}


struct Vec2i
{
	union {
		struct {
			int32_t x, y;
		};
		int32_t v[2];
	};

	Vec2i(): x(0), y(0) { }
	explicit Vec2i(int32_t s): x(s), y(s) { }
	Vec2i(int32_t x, int32_t y): x(x), y(y) { }
	explicit Vec2i(const int32_t *ptr): x(ptr[0]), y(ptr[1]) { }
	explicit Vec2i(const Vec2 &rhs);

	Vec2i operator+(const Vec2i &rhs) const { return Vec2i(x + rhs.x, y + rhs.y); }
	Vec2i operator-(const Vec2i &rhs) const { return Vec2i(x - rhs.x, y - rhs.y); }
	Vec2i operator*(const Vec2i &rhs) const { return Vec2i(x * rhs.x, y * rhs.y); }
	Vec2i operator/(const Vec2i &rhs) const { return Vec2i(x / rhs.x, y / rhs.y); }

	Vec2i &operator+=(const Vec2i &rhs) { x += rhs.x; y += rhs.y; return *this; }
	Vec2i &operator-=(const Vec2i &rhs) { x -= rhs.x; y -= rhs.y; return *this; }
	Vec2i &operator*=(const Vec2i &rhs) { x *= rhs.x; y *= rhs.y; return *this; }
	Vec2i &operator/=(const Vec2i &rhs) { x /= rhs.x; y /= rhs.y; return *this; }

	Vec2i operator*(int32_t rhs) const { return Vec2i(x * rhs, y * rhs); }
	Vec2i operator/(int32_t rhs) const { return Vec2i(x / rhs, y / rhs); }

	Vec2i &operator*=(int32_t rhs) { x *= rhs; y *= rhs; return *this; }
	Vec2i &operator/=(int32_t rhs) { x /= rhs; y /= rhs; return *this; }

	Vec2i operator-() const { return Vec2i(-x, -y); }

	bool operator==(const Vec2i &rhs) const { return x == rhs.x && y == rhs.y; }
	bool operator!=(const Vec2i &rhs) const { return x != rhs.x || y != rhs.y; }
};

sf_inline int32_t dot(const Vec2i &a, const Vec2i &b) { return a.x*b.x + a.y*b.y; }
sf_inline Vec2i min(const Vec2i &a, const Vec2i &b) { return Vec2i(min(a.x, b.x), min(a.y, b.y)); }
sf_inline Vec2i max(const Vec2i &a, const Vec2i &b) { return Vec2i(max(a.x, b.x), max(a.y, b.y)); }

struct Vec3i
{
	union {
		struct {
			int32_t x, y, z;
		};
		int32_t v[3];
	};

	Vec3i(): x(0), y(0), z(0) { }
	explicit Vec3i(int32_t s): x(s), y(s), z(s) { }
	Vec3i(int32_t x, int32_t y, int32_t z): x(x), y(y), z(z) { }
	explicit Vec3i(const int32_t *ptr): x(ptr[0]), y(ptr[1]), z(ptr[2]) { }
	explicit Vec3i(const Vec3 &rhs);

	Vec3i operator+(const Vec3i &rhs) const { return Vec3i(x + rhs.x, y + rhs.y, z + rhs.z); }
	Vec3i operator-(const Vec3i &rhs) const { return Vec3i(x - rhs.x, y - rhs.y, z - rhs.z); }
	Vec3i operator*(const Vec3i &rhs) const { return Vec3i(x * rhs.x, y * rhs.y, z * rhs.z); }
	Vec3i operator/(const Vec3i &rhs) const { return Vec3i(x / rhs.x, y / rhs.y, z / rhs.z); }

	Vec3i &operator+=(const Vec3i &rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
	Vec3i &operator-=(const Vec3i &rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
	Vec3i &operator*=(const Vec3i &rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
	Vec3i &operator/=(const Vec3i &rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }

	Vec3i operator*(int32_t rhs) const { return Vec3i(x * rhs, y * rhs, z * rhs); }
	Vec3i operator/(int32_t rhs) const { return Vec3i(x / rhs, y / rhs, z / rhs); }

	Vec3i &operator*=(int32_t rhs) { x *= rhs; y *= rhs; z *= rhs; return *this; }
	Vec3i &operator/=(int32_t rhs) { x /= rhs; y /= rhs; z /= rhs; return *this; }

	Vec3i operator-() const { return Vec3i(-x, -y, -z); }

	bool operator==(const Vec3i &rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
	bool operator!=(const Vec3i &rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z; }
};

sf_inline int32_t dot(const Vec3i &a, const Vec3i &b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
sf_inline Vec3i min(const Vec3i &a, const Vec3i &b) { return Vec3i(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z)); }
sf_inline Vec3i max(const Vec3i &a, const Vec3i &b) { return Vec3i(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z)); }

struct Vec4i
{
	union {
		struct {
			int32_t x, y, z, w;
		};
		int32_t v[4];
	};

	Vec4i(): x(0), y(0), z(0), w(0) { }
	explicit Vec4i(int32_t s): x(s), y(s), z(s), w(s) { }
	Vec4i(int32_t x, int32_t y, int32_t z, int32_t w): x(x), y(y), z(z), w(w) { }
	explicit Vec4i(const int32_t *ptr): x(ptr[0]), y(ptr[1]), z(ptr[2]), w(ptr[3]) { }
	explicit Vec4i(const Vec4 &rhs);

	Vec4i operator+(const Vec4i &rhs) const { return Vec4i(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
	Vec4i operator-(const Vec4i &rhs) const { return Vec4i(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
	Vec4i operator*(const Vec4i &rhs) const { return Vec4i(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
	Vec4i operator/(const Vec4i &rhs) const { return Vec4i(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }

	Vec4i &operator+=(const Vec4i &rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
	Vec4i &operator-=(const Vec4i &rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
	Vec4i &operator*=(const Vec4i &rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
	Vec4i &operator/=(const Vec4i &rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }

	Vec4i operator*(int32_t rhs) const { return Vec4i(x * rhs, y * rhs, z * rhs, w * rhs); }
	Vec4i operator/(int32_t rhs) const { return Vec4i(x / rhs, y / rhs, z / rhs, w / rhs); }

	Vec4i &operator*=(int32_t rhs) { x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; }
	Vec4i &operator/=(int32_t rhs) { x /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; }

	Vec4i operator-() const { return Vec4i(-x, -y, -z, -w); }

	bool operator==(const Vec4i &rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
	bool operator!=(const Vec4i &rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z || w == rhs.w; }
};

sf_inline int32_t dot(const Vec4i &a, const Vec4i &b) { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }
sf_inline Vec4i min(const Vec4i &a, const Vec4i &b) { return Vec4i(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z), min(a.w, b.w)); }
sf_inline Vec4i max(const Vec4i &a, const Vec4i &b) { return Vec4i(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z), max(a.w, b.w)); }

inline Vec2::Vec2(const Vec2i &rhs) : x((float)rhs.x), y((float)rhs.y) { }
inline Vec3::Vec3(const Vec3i &rhs) : x((float)rhs.x), y((float)rhs.y), z((float)rhs.z) { }
inline Vec4::Vec4(const Vec4i &rhs) : x((float)rhs.x), y((float)rhs.y), z((float)rhs.z), w((float)rhs.w) { }

inline Vec2i::Vec2i(const Vec2 &rhs) : x((int32_t)rhs.x), y((int32_t)rhs.y) { }
inline Vec3i::Vec3i(const Vec3 &rhs) : x((int32_t)rhs.x), y((int32_t)rhs.y), z((int32_t)rhs.z) { }
inline Vec4i::Vec4i(const Vec4 &rhs) : x((int32_t)rhs.x), y((int32_t)rhs.y), z((int32_t)rhs.z), w((int32_t)rhs.w) { }

template <> struct IsZeroInitializable<Vec2> { enum { value = 1 }; };
template <> struct IsZeroInitializable<Vec3> { enum { value = 1 }; };
template <> struct IsZeroInitializable<Vec4> { enum { value = 1 }; };
template <> struct IsZeroInitializable<Vec2i> { enum { value = 1 }; };
template <> struct IsZeroInitializable<Vec3i> { enum { value = 1 }; };
template <> struct IsZeroInitializable<Vec4i> { enum { value = 1 }; };

sf_inline uint32_t hash(const Vec2 &v) { return hashBuffer(&v, sizeof(v)); }
sf_inline uint32_t hash(const Vec3 &v) { return hashBuffer(&v, sizeof(v)); }
sf_inline uint32_t hash(const Vec4 &v) { return hashBuffer(&v, sizeof(v)); }
sf_inline uint32_t hash(const Vec2i &v) { return hashBuffer(&v, sizeof(v)); }
sf_inline uint32_t hash(const Vec3i &v) { return hashBuffer(&v, sizeof(v)); }
sf_inline uint32_t hash(const Vec4i &v) { return hashBuffer(&v, sizeof(v)); }

}
