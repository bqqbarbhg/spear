#pragma once

#include "Base.h"
#include "Vector.h"

namespace sf {

struct Quat
{
	union {
		struct {
			float x, y, z, w;
		};
		float v[4];
	};

	Quat() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) { }
	Quat(float x, float y, float z, float w): x(x), y(y), z(z), w(w) { }

	Quat operator+(const Quat &rhs) const { return Quat(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
	Quat operator-(const Quat &rhs) const { return Quat(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }

	Quat &operator+=(const Quat &rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
	Quat &operator-=(const Quat &rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }

	Quat operator*(float rhs) const { return Quat(x * rhs, y * rhs, z * rhs, w * rhs); }
	Quat operator/(float rhs) const { return Quat(x / rhs, y / rhs, z / rhs, w / rhs); }

	Quat &operator*=(float rhs) { x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; }
	Quat &operator/=(float rhs) { x /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; }

	bool operator==(const Quat &rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
	bool operator!=(const Quat &rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z || w == rhs.w; }

	Quat operator*(const Quat &rhs) const {
		Quat r;
		r.x = w*rhs.x + x*rhs.w + y*rhs.z - z*rhs.y;
		r.y = w*rhs.y - x*rhs.z + y*rhs.w + z*rhs.x;
		r.z = w*rhs.z + x*rhs.y - y*rhs.x + z*rhs.w;
		r.w = w*rhs.w - x*rhs.x - y*rhs.y - z*rhs.z;
		return r;
	}

	Quat &operator*=(const Quat &rhs) {
		*this = (*this * rhs);
		return *this;
	}

	Quat operator-() const {
		return Quat(-x, -y, -z, -w);
	}
};

sf_inline float dot(const Quat &a, const Quat &b) { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }
sf_inline float lengthSq(const Quat &a) { return a.x*a.x + a.y*a.y + a.z*a.z + a.w*a.w; }
sf_inline float length(const Quat &a) { return sqrt(a.x*a.x + a.y*a.y + a.z*a.z + a.w*a.w); }
sf_inline Quat normalize(const Quat &a) { return a * (1.0f / length(a)); }
sf_inline Quat normalizeOrIdentity(const Quat &a) {
	float len = length(a);
	return len > 1e-9 ? a * (1.0f / len) : Quat();
}
sf_inline Quat min(const Quat &a, const Quat &b) { return Quat(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z), min(a.w, b.w)); }
sf_inline Quat max(const Quat &a, const Quat &b) { return Quat(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z), max(a.w, b.w)); }
sf_inline Quat lerp(const Quat &a, const Quat &b, float t) { return a * (1.0f - t) + b * t; }

sf_inline sf::Vec3 rotate(const sf::Quat &q, const sf::Vec3 &v) {
	float xy = q.x*v.y - q.y*v.x;
	float xz = q.x*v.z - q.z*v.x;
	float yz = q.y*v.z - q.z*v.y;
	sf::Vec3 r;
	r.x = 2.0f * (+ q.w*yz + q.y*xy + q.z*xz) + v.x;
	r.y = 2.0f * (- q.x*xy - q.w*xz + q.z*yz) + v.y;
	r.z = 2.0f * (- q.x*xz - q.y*yz + q.w*xy) + v.z;
	return r;
}

enum class EulerOrder {
	XYZ, XZY, YZX, YXZ, ZXY, ZYX
};

Quat eulerAnglesToQuat(float x, float y, float z, EulerOrder order=EulerOrder::XYZ);
Quat eulerAnglesToQuat(const sf::Vec3 &v, EulerOrder order=EulerOrder::XYZ);

Quat axesToQuat(const sf::Vec3 &x, const sf::Vec3 &y, const sf::Vec3 &z);

}
