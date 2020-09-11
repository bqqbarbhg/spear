#pragma once

#include "Vector.h"
#include <math.h>

namespace sf {

struct Mat34_3;
struct Mat33;
struct Mat34;
struct Mat44;
struct Quat;

struct Mat34_3
{
	union {
		struct {
			float m03;
			float m13;
			float m23;
		};
		Vec3 col3;
		float v[3];
	};

	Mat34_3(UninitType) { }

	Mat34_3()
		: m03(0.0f), m13(0.0f), m23(0.0f)
	{ }

	Mat34_3(float m03, float m13, float m23)
		: m03(m03), m13(m13), m23(m23)
	{ }
};

struct Mat33_D
{
	union {
		struct {
			float m00;
			float m11;
			float m22;
		};
		float v[3];
	};

	Mat33_D(UninitType) { }

	Mat33_D()
		: m00(0.0f), m11(0.0f), m22(0.0f)
	{ }

	Mat33_D(float m00, float m11, float m22)
		: m00(m00), m11(m11), m22(m22)
	{ }
};

struct Mat33
{
	union {
		struct {
			float m00, m10, m20;
			float m01, m11, m21;
			float m02, m12, m22;
		};
		Vec3 cols[3];
		float v[9];
	};

	Mat33(UninitType) { }

	Mat33()
		: m00(1.0f), m01(0.0f), m02(0.0f)
		, m10(0.0f), m11(1.0f), m12(0.0f)
		, m20(0.0f), m21(0.0f), m22(1.0f)
	{ }

	Mat33(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22)
		: m00(m00), m01(m01), m02(m02)
		, m10(m10), m11(m11), m12(m12)
		, m20(m20), m21(m21), m22(m22)
	{ }
};

struct Mat34
{
	union {
		struct {
			float m00, m10, m20;
			float m01, m11, m21;
			float m02, m12, m22;
			float m03, m13, m23;
		};
		Vec3 cols[4];
		float v[12];
	};

	Mat34(UninitType) { }

	Mat34()
		: m00(1.0f), m01(0.0f), m02(0.0f), m03(0.0f)
		, m10(0.0f), m11(1.0f), m12(0.0f), m13(0.0f)
		, m20(0.0f), m21(0.0f), m22(1.0f), m23(0.0f)
	{ }

	Mat34(const Mat33_D &rhs)
		: m00(rhs.m00), m01(0.0f), m02(0.0f), m03(0.0f)
		, m10(0.0f), m11(rhs.m11), m12(0.0f), m13(0.0f)
		, m20(0.0f), m21(0.0f), m22(rhs.m22), m23(0.0f)
	{ } 

	Mat34(const Mat34_3 &rhs)
		: m00(1.0f), m01(0.0f), m02(0.0f), m03(rhs.m03)
		, m10(0.0f), m11(1.0f), m12(0.0f), m13(rhs.m13)
		, m20(0.0f), m21(0.0f), m22(1.0f), m23(rhs.m23)
	{ } 

	Mat34(const Mat33 &rhs)
		: m00(rhs.m00), m01(rhs.m01), m02(rhs.m02), m03(0.0f)
		, m10(rhs.m10), m11(rhs.m11), m12(rhs.m12), m13(0.0f)
		, m20(rhs.m20), m21(rhs.m21), m22(rhs.m22), m23(0.0f)
	{ } 

	Mat34(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23)
		: m00(m00), m01(m01), m02(m02), m03(m03)
		, m10(m10), m11(m11), m12(m12), m13(m13)
		, m20(m20), m21(m21), m22(m22), m23(m23)
	{ }

	sf_forceinline Vec4 getRow(int index) { return Vec4(cols[0].v[index], cols[1].v[index], cols[2].v[index], cols[3].v[index]); }

	void writeColMajor44(float *dst) const {
		memcpy(dst +  0, &cols[0], sizeof(Vec3)); dst[ 3] = 0.0f;
		memcpy(dst +  4, &cols[1], sizeof(Vec3)); dst[ 7] = 0.0f;
		memcpy(dst +  8, &cols[2], sizeof(Vec3)); dst[11] = 0.0f;
		memcpy(dst + 12, &cols[3], sizeof(Vec3)); dst[15] = 1.0f;
	}

	Mat33 get33() const {
		Mat33 r = Uninit;
		r.cols[0] = cols[0];
		r.cols[1] = cols[1];
		r.cols[2] = cols[2];
		return r;
	}
};

struct Mat44
{
	union {
		struct {
			float m00, m10, m20, m30;
			float m01, m11, m21, m31;
			float m02, m12, m22, m32;
			float m03, m13, m23, m33;
		};
		Vec4 cols[4];
		float v[16];
	};

	Mat44(UninitType) { }

	Mat44()
		: m00(1.0f), m01(0.0f), m02(0.0f), m03(0.0f)
		, m10(0.0f), m11(1.0f), m12(0.0f), m13(0.0f)
		, m20(0.0f), m21(0.0f), m22(1.0f), m23(0.0f)
		, m30(0.0f), m31(0.0f), m32(0.0f), m33(1.0f)
	{ }

	Mat44(const Mat33 &rhs)
		: m00(rhs.m00), m01(rhs.m01), m02(rhs.m02), m03(0.0f)
		, m10(rhs.m10), m11(rhs.m11), m12(rhs.m12), m13(0.0f)
		, m20(rhs.m20), m21(rhs.m21), m22(rhs.m22), m23(0.0f)
		, m30(0.0f), m31(0.0f), m32(0.0f), m33(1.0f)
	{ } 

	Mat44(const Mat34 &rhs)
		: m00(rhs.m00), m01(rhs.m01), m02(rhs.m02), m03(rhs.m03)
		, m10(rhs.m10), m11(rhs.m11), m12(rhs.m12), m13(rhs.m13)
		, m20(rhs.m20), m21(rhs.m21), m22(rhs.m22), m23(rhs.m23)
		, m30(0.0f), m31(0.0f), m32(0.0f), m33(1.0f)
	{ }
    
    Mat44(const Mat34_3 &rhs)
    : m00(0.0f), m01(0.0f), m02(0.0f), m03(rhs.m03)
    , m10(0.0f), m11(0.0f), m12(0.0f), m13(rhs.m13)
    , m20(0.0f), m21(0.0f), m22(0.0f), m23(rhs.m23)
    , m30(0.0f), m31(0.0f), m32(0.0f), m33(1.0f)
    { }

	Mat44(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
		: m00(m00), m01(m01), m02(m02), m03(m03)
		, m10(m10), m11(m11), m12(m12), m13(m13)
		, m20(m20), m21(m21), m22(m22), m23(m23)
		, m30(m30), m31(m31), m32(m32), m33(m33)
	{ }

	sf_forceinline Vec4 getRow(int index) { return Vec4(cols[0].v[index], cols[1].v[index], cols[2].v[index], cols[3].v[index]); }

	void writeColMajor44(float *dst) const {
		memcpy(dst, v, sizeof(v));
	}
};

struct Mat23
{
	union {
		struct {
			float m00, m10;
			float m01, m11;
			float m02, m12;
		};
		Vec2 cols[3];
		float v[6];
	};

	Mat23(UninitType) { }

	Mat23()
		: m00(1.0f), m01(0.0f), m02(0.0f)
		, m10(0.0f), m11(1.0f), m12(0.0f)
	{ }

	Mat23(float m00, float m10, float m01, float m11, float m02, float m12)
		: m00(m00), m10(m10)
		, m01(m01), m11(m11)
		, m02(m02), m12(m12)
	{ }

};

// Composition

Mat34_3 operator*(const Mat34_3 &l, const Mat34_3 &r);
Mat34 operator*(const Mat34_3 &l, const Mat33_D &r);
Mat34 operator*(const Mat34_3 &l, const Mat33 &r);
Mat34 operator*(const Mat34_3 &l, const Mat34 &r);
Mat44 operator*(const Mat34_3 &l, const Mat44 &r);
Mat34 operator*(const Mat33_D &l, const Mat34_3 &r);
Mat33_D operator*(const Mat33_D &l, const Mat33_D &r);
Mat33 operator*(const Mat33_D &l, const Mat33 &r);
Mat34 operator*(const Mat33_D &l, const Mat34 &r);
Mat44 operator*(const Mat33_D &l, const Mat44 &r);
Mat34 operator*(const Mat33 &l, const Mat34_3 &r);
Mat33 operator*(const Mat33 &l, const Mat33_D &r);
Mat33 operator*(const Mat33 &l, const Mat33 &r);
Mat34 operator*(const Mat33 &l, const Mat34 &r);
Mat44 operator*(const Mat33 &l, const Mat44 &r);
Mat34 operator*(const Mat34 &l, const Mat34_3 &r);
Mat34 operator*(const Mat34 &l, const Mat33_D &r);
Mat34 operator*(const Mat34 &l, const Mat33 &r);
Mat34 operator*(const Mat34 &l, const Mat34 &r);
Mat44 operator*(const Mat34 &l, const Mat44 &r);
Mat44 operator*(const Mat44 &l, const Mat34_3 &r);
Mat44 operator*(const Mat44 &l, const Mat33_D &r);
Mat44 operator*(const Mat44 &l, const Mat33 &r);
Mat44 operator*(const Mat44 &l, const Mat34 &r);
Mat44 operator*(const Mat44 &l, const Mat44 &r);

Mat23 operator*(const Mat23 &l, const Mat23 &r);

// Multiply by scalar

Mat33 operator*(const Mat33 &l, float r);
Mat33_D operator*(const Mat33_D &l, float r);
Mat34_3 operator*(const Mat34_3 &l, float r);
Mat34 operator*(const Mat34 &l, float r);
Mat44 operator*(const Mat44 &l, float r);

Mat33 lerpFromIdentity(const Mat33 &l, float r);
Mat33_D lerpFromIdentity(const Mat33_D &l, float r);
Mat34_3 lerpFromIdentity(const Mat34_3 &l, float r);
Mat34 lerpFromIdentity(const Mat34 &l, float r);
Mat44 lerpFromIdentity(const Mat44 &l, float r);


// Multiply by vector

Vec4 operator*(const Mat34_3 &l, const Vec4 &r);
Vec4 operator*(const Mat33_D &l, const Vec4 &r);
Vec4 operator*(const Mat33 &l, const Vec4 &r);
Vec4 operator*(const Mat34 &l, const Vec4 &r);
Vec4 operator*(const Mat44 &l, const Vec4 &r);

Vec3 operator*(const Mat23 &l, const Vec3 &r);

// Transform point

Vec3 transformPoint(const Mat33 &l, const Vec3 &r);
Vec3 transformPoint(const Mat34 &l, const Vec3 &r);

Vec2 transformPoint(const Mat23 &l, const Vec2 &r);

// Transform direction

Vec3 transformDirection(const Mat33 &l, const Vec3 &r);
Vec3 transformDirection(const Mat34 &l, const Vec3 &r);
Vec3 transformDirectionAbs(const Mat33 &l, const Vec3 &r);
Vec3 transformDirectionAbs(const Mat34 &l, const Vec3 &r);

// Determinant

float determinant(const Mat34_3 &m);
float determinant(const Mat33_D &m);
float determinant(const Mat33 &m);
float determinant(const Mat34 &m);
float determinant(const Mat44 &m);

float determinant(const Mat23 &m);

// Transpose

Mat44 transpose(const Mat34_3 &m);
Mat33_D transpose(const Mat33_D &m);
Mat33 transpose(const Mat33 &m);
Mat44 transpose(const Mat34 &m);
Mat44 transpose(const Mat44 &m);

// Inverse

Mat34_3 inverse(const Mat34_3 &m);
Mat33_D inverse(const Mat33_D &m);
Mat33 inverse(const Mat33 &m);
Mat34 inverse(const Mat34 &m);
Mat44 inverse(const Mat44 &m);

Mat23 inverse(const Mat23 &m);

// Initializers

namespace mat {

	Mat34_3 translateX(float x);
	Mat34_3 translateY(float y);
	Mat34_3 translateZ(float z);
	Mat34_3 translate(float x, float y, float z);
	Mat34_3 translate(const Vec3 &amount);

	Mat33_D scaleX(float x);
	Mat33_D scaleY(float y);
	Mat33_D scaleZ(float z);
	Mat33_D scale(float s);
	Mat33_D scale(float x, float y, float z);
	Mat33_D scale(const Vec3 &amount);

	Mat33 rotateX(float angle);
	Mat33 rotateY(float angle);
	Mat33 rotateZ(float angle);

	Mat33 rotateX(float angle);
	Mat33 rotateY(float angle);
	Mat33 rotateZ(float angle);

	Mat34 inverseBasis(const sf::Vec3 &x, const sf::Vec3 &y, const sf::Vec3 &z, const sf::Vec3 &origin=sf::Vec3());

	Mat34 look(const sf::Vec3 &eye, const sf::Vec3 &dir, const sf::Vec3 &up=sf::Vec3(0.0f,1.0f,0.0f));
	Mat34 world(const sf::Vec3 &translation, const sf::Quat &rotation, float scale);
	Mat34 world(const sf::Vec3 &translation, const sf::Quat &rotation, const sf::Vec3 &scale);

	Mat44 perspectiveD3D(float fov, float aspect, float near, float far);
	Mat44 perspectiveGL(float fov, float aspect, float near, float far);

	Mat44 orthoSkewedD3D(const sf::Vec2 &extent, const sf::Vec2 &skew, float near, float far);
}

namespace mat2D {

	Mat23 translateX(float x);
	Mat23 translateY(float y);
	Mat23 translate(float x, float y);
	Mat23 translate(const Vec2 &amount);

	Mat23 scaleX(float x);
	Mat23 scaleY(float y);
	Mat23 scale(float s);
	Mat23 scale(float x, float y);
	Mat23 scale(const Vec2 &amount);

	Mat23 rotate(float angle);

}

}
