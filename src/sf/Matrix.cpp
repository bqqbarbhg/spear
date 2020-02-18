#include "Matrix.h"

namespace sf {

// Composition

Mat34_3 operator*(const Mat34_3 &l, const Mat34_3 &r) {
	return {
		r.m03 + l.m03,
		r.m13 + l.m13,
		r.m23 + l.m23,
	};
}
Mat34 operator*(const Mat34_3 &l, const Mat33_D &r) {
	return {
		r.m00,
		0.0f,
		0.0f,
		l.m03,
		0.0f,
		r.m11,
		0.0f,
		l.m13,
		0.0f,
		0.0f,
		r.m22,
		l.m23,
	};
}
Mat34 operator*(const Mat34_3 &l, const Mat33 &r) {
	return {
		r.m00,
		r.m01,
		r.m02,
		l.m03,
		r.m10,
		r.m11,
		r.m12,
		l.m13,
		r.m20,
		r.m21,
		r.m22,
		l.m23,
	};
}
Mat34 operator*(const Mat34_3 &l, const Mat34 &r) {
	return {
		r.m00,
		r.m01,
		r.m02,
		r.m03 + l.m03,
		r.m10,
		r.m11,
		r.m12,
		r.m13 + l.m13,
		r.m20,
		r.m21,
		r.m22,
		r.m23 + l.m23,
	};
}
Mat44 operator*(const Mat34_3 &l, const Mat44 &r) {
	return {
		r.m00 + l.m03*r.m30,
		r.m01 + l.m03*r.m31,
		r.m02 + l.m03*r.m32,
		r.m03 + l.m03*r.m33,
		r.m10 + l.m13*r.m30,
		r.m11 + l.m13*r.m31,
		r.m12 + l.m13*r.m32,
		r.m13 + l.m13*r.m33,
		r.m20 + l.m23*r.m30,
		r.m21 + l.m23*r.m31,
		r.m22 + l.m23*r.m32,
		r.m23 + l.m23*r.m33,
		r.m30,
		r.m31,
		r.m32,
		r.m33,
	};
}
Mat34 operator*(const Mat33_D &l, const Mat34_3 &r) {
	return {
		l.m00,
		0.0f,
		0.0f,
		l.m00*r.m03,
		0.0f,
		l.m11,
		0.0f,
		l.m11*r.m13,
		0.0f,
		0.0f,
		l.m22,
		l.m22*r.m23,
	};
}
Mat33_D operator*(const Mat33_D &l, const Mat33_D &r) {
	return {
		l.m00*r.m00,
		l.m11*r.m11,
		l.m22*r.m22,
	};
}
Mat33 operator*(const Mat33_D &l, const Mat33 &r) {
	return {
		l.m00*r.m00,
		l.m00*r.m01,
		l.m00*r.m02,
		l.m11*r.m10,
		l.m11*r.m11,
		l.m11*r.m12,
		l.m22*r.m20,
		l.m22*r.m21,
		l.m22*r.m22,
	};
}
Mat34 operator*(const Mat33_D &l, const Mat34 &r) {
	return {
		l.m00*r.m00,
		l.m00*r.m01,
		l.m00*r.m02,
		l.m00*r.m03,
		l.m11*r.m10,
		l.m11*r.m11,
		l.m11*r.m12,
		l.m11*r.m13,
		l.m22*r.m20,
		l.m22*r.m21,
		l.m22*r.m22,
		l.m22*r.m23,
	};
}
Mat44 operator*(const Mat33_D &l, const Mat44 &r) {
	return {
		l.m00*r.m00,
		l.m00*r.m01,
		l.m00*r.m02,
		l.m00*r.m03,
		l.m11*r.m10,
		l.m11*r.m11,
		l.m11*r.m12,
		l.m11*r.m13,
		l.m22*r.m20,
		l.m22*r.m21,
		l.m22*r.m22,
		l.m22*r.m23,
		r.m30,
		r.m31,
		r.m32,
		r.m33,
	};
}
Mat34 operator*(const Mat33 &l, const Mat34_3 &r) {
	return {
		l.m00,
		l.m01,
		l.m02,
		l.m00*r.m03 + l.m01*r.m13 + l.m02*r.m23,
		l.m10,
		l.m11,
		l.m12,
		l.m10*r.m03 + l.m11*r.m13 + l.m12*r.m23,
		l.m20,
		l.m21,
		l.m22,
		l.m20*r.m03 + l.m21*r.m13 + l.m22*r.m23,
	};
}
Mat33 operator*(const Mat33 &l, const Mat33_D &r) {
	return {
		l.m00*r.m00,
		l.m01*r.m11,
		l.m02*r.m22,
		l.m10*r.m00,
		l.m11*r.m11,
		l.m12*r.m22,
		l.m20*r.m00,
		l.m21*r.m11,
		l.m22*r.m22,
	};
}
Mat33 operator*(const Mat33 &l, const Mat33 &r) {
	return {
		l.m00*r.m00 + l.m01*r.m10 + l.m02*r.m20,
		l.m00*r.m01 + l.m01*r.m11 + l.m02*r.m21,
		l.m00*r.m02 + l.m01*r.m12 + l.m02*r.m22,
		l.m10*r.m00 + l.m11*r.m10 + l.m12*r.m20,
		l.m10*r.m01 + l.m11*r.m11 + l.m12*r.m21,
		l.m10*r.m02 + l.m11*r.m12 + l.m12*r.m22,
		l.m20*r.m00 + l.m21*r.m10 + l.m22*r.m20,
		l.m20*r.m01 + l.m21*r.m11 + l.m22*r.m21,
		l.m20*r.m02 + l.m21*r.m12 + l.m22*r.m22,
	};
}
Mat34 operator*(const Mat33 &l, const Mat34 &r) {
	return {
		l.m00*r.m00 + l.m01*r.m10 + l.m02*r.m20,
		l.m00*r.m01 + l.m01*r.m11 + l.m02*r.m21,
		l.m00*r.m02 + l.m01*r.m12 + l.m02*r.m22,
		l.m00*r.m03 + l.m01*r.m13 + l.m02*r.m23,
		l.m10*r.m00 + l.m11*r.m10 + l.m12*r.m20,
		l.m10*r.m01 + l.m11*r.m11 + l.m12*r.m21,
		l.m10*r.m02 + l.m11*r.m12 + l.m12*r.m22,
		l.m10*r.m03 + l.m11*r.m13 + l.m12*r.m23,
		l.m20*r.m00 + l.m21*r.m10 + l.m22*r.m20,
		l.m20*r.m01 + l.m21*r.m11 + l.m22*r.m21,
		l.m20*r.m02 + l.m21*r.m12 + l.m22*r.m22,
		l.m20*r.m03 + l.m21*r.m13 + l.m22*r.m23,
	};
}
Mat44 operator*(const Mat33 &l, const Mat44 &r) {
	return {
		l.m00*r.m00 + l.m01*r.m10 + l.m02*r.m20,
		l.m00*r.m01 + l.m01*r.m11 + l.m02*r.m21,
		l.m00*r.m02 + l.m01*r.m12 + l.m02*r.m22,
		l.m00*r.m03 + l.m01*r.m13 + l.m02*r.m23,
		l.m10*r.m00 + l.m11*r.m10 + l.m12*r.m20,
		l.m10*r.m01 + l.m11*r.m11 + l.m12*r.m21,
		l.m10*r.m02 + l.m11*r.m12 + l.m12*r.m22,
		l.m10*r.m03 + l.m11*r.m13 + l.m12*r.m23,
		l.m20*r.m00 + l.m21*r.m10 + l.m22*r.m20,
		l.m20*r.m01 + l.m21*r.m11 + l.m22*r.m21,
		l.m20*r.m02 + l.m21*r.m12 + l.m22*r.m22,
		l.m20*r.m03 + l.m21*r.m13 + l.m22*r.m23,
		r.m30,
		r.m31,
		r.m32,
		r.m33,
	};
}
Mat34 operator*(const Mat34 &l, const Mat34_3 &r) {
	return {
		l.m00,
		l.m01,
		l.m02,
		l.m00*r.m03 + l.m01*r.m13 + l.m02*r.m23 + l.m03,
		l.m10,
		l.m11,
		l.m12,
		l.m10*r.m03 + l.m11*r.m13 + l.m12*r.m23 + l.m13,
		l.m20,
		l.m21,
		l.m22,
		l.m20*r.m03 + l.m21*r.m13 + l.m22*r.m23 + l.m23,
	};
}
Mat34 operator*(const Mat34 &l, const Mat33_D &r) {
	return {
		l.m00*r.m00,
		l.m01*r.m11,
		l.m02*r.m22,
		l.m03,
		l.m10*r.m00,
		l.m11*r.m11,
		l.m12*r.m22,
		l.m13,
		l.m20*r.m00,
		l.m21*r.m11,
		l.m22*r.m22,
		l.m23,
	};
}
Mat34 operator*(const Mat34 &l, const Mat33 &r) {
	return {
		l.m00*r.m00 + l.m01*r.m10 + l.m02*r.m20,
		l.m00*r.m01 + l.m01*r.m11 + l.m02*r.m21,
		l.m00*r.m02 + l.m01*r.m12 + l.m02*r.m22,
		l.m03,
		l.m10*r.m00 + l.m11*r.m10 + l.m12*r.m20,
		l.m10*r.m01 + l.m11*r.m11 + l.m12*r.m21,
		l.m10*r.m02 + l.m11*r.m12 + l.m12*r.m22,
		l.m13,
		l.m20*r.m00 + l.m21*r.m10 + l.m22*r.m20,
		l.m20*r.m01 + l.m21*r.m11 + l.m22*r.m21,
		l.m20*r.m02 + l.m21*r.m12 + l.m22*r.m22,
		l.m23,
	};
}
Mat34 operator*(const Mat34 &l, const Mat34 &r) {
	return {
		l.m00*r.m00 + l.m01*r.m10 + l.m02*r.m20,
		l.m00*r.m01 + l.m01*r.m11 + l.m02*r.m21,
		l.m00*r.m02 + l.m01*r.m12 + l.m02*r.m22,
		l.m00*r.m03 + l.m01*r.m13 + l.m02*r.m23 + l.m03,
		l.m10*r.m00 + l.m11*r.m10 + l.m12*r.m20,
		l.m10*r.m01 + l.m11*r.m11 + l.m12*r.m21,
		l.m10*r.m02 + l.m11*r.m12 + l.m12*r.m22,
		l.m10*r.m03 + l.m11*r.m13 + l.m12*r.m23 + l.m13,
		l.m20*r.m00 + l.m21*r.m10 + l.m22*r.m20,
		l.m20*r.m01 + l.m21*r.m11 + l.m22*r.m21,
		l.m20*r.m02 + l.m21*r.m12 + l.m22*r.m22,
		l.m20*r.m03 + l.m21*r.m13 + l.m22*r.m23 + l.m23,
	};
}
Mat44 operator*(const Mat34 &l, const Mat44 &r) {
	return {
		l.m00*r.m00 + l.m01*r.m10 + l.m02*r.m20 + l.m03*r.m30,
		l.m00*r.m01 + l.m01*r.m11 + l.m02*r.m21 + l.m03*r.m31,
		l.m00*r.m02 + l.m01*r.m12 + l.m02*r.m22 + l.m03*r.m32,
		l.m00*r.m03 + l.m01*r.m13 + l.m02*r.m23 + l.m03*r.m33,
		l.m10*r.m00 + l.m11*r.m10 + l.m12*r.m20 + l.m13*r.m30,
		l.m10*r.m01 + l.m11*r.m11 + l.m12*r.m21 + l.m13*r.m31,
		l.m10*r.m02 + l.m11*r.m12 + l.m12*r.m22 + l.m13*r.m32,
		l.m10*r.m03 + l.m11*r.m13 + l.m12*r.m23 + l.m13*r.m33,
		l.m20*r.m00 + l.m21*r.m10 + l.m22*r.m20 + l.m23*r.m30,
		l.m20*r.m01 + l.m21*r.m11 + l.m22*r.m21 + l.m23*r.m31,
		l.m20*r.m02 + l.m21*r.m12 + l.m22*r.m22 + l.m23*r.m32,
		l.m20*r.m03 + l.m21*r.m13 + l.m22*r.m23 + l.m23*r.m33,
		r.m30,
		r.m31,
		r.m32,
		r.m33,
	};
}
Mat44 operator*(const Mat44 &l, const Mat34_3 &r) {
	return {
		l.m00,
		l.m01,
		l.m02,
		l.m00*r.m03 + l.m01*r.m13 + l.m02*r.m23 + l.m03,
		l.m10,
		l.m11,
		l.m12,
		l.m10*r.m03 + l.m11*r.m13 + l.m12*r.m23 + l.m13,
		l.m20,
		l.m21,
		l.m22,
		l.m20*r.m03 + l.m21*r.m13 + l.m22*r.m23 + l.m23,
		l.m30,
		l.m31,
		l.m32,
		l.m30*r.m03 + l.m31*r.m13 + l.m32*r.m23 + l.m33,
	};
}
Mat44 operator*(const Mat44 &l, const Mat33_D &r) {
	return {
		l.m00*r.m00,
		l.m01*r.m11,
		l.m02*r.m22,
		l.m03,
		l.m10*r.m00,
		l.m11*r.m11,
		l.m12*r.m22,
		l.m13,
		l.m20*r.m00,
		l.m21*r.m11,
		l.m22*r.m22,
		l.m23,
		l.m30*r.m00,
		l.m31*r.m11,
		l.m32*r.m22,
		l.m33,
	};
}
Mat44 operator*(const Mat44 &l, const Mat33 &r) {
	return {
		l.m00*r.m00 + l.m01*r.m10 + l.m02*r.m20,
		l.m00*r.m01 + l.m01*r.m11 + l.m02*r.m21,
		l.m00*r.m02 + l.m01*r.m12 + l.m02*r.m22,
		l.m03,
		l.m10*r.m00 + l.m11*r.m10 + l.m12*r.m20,
		l.m10*r.m01 + l.m11*r.m11 + l.m12*r.m21,
		l.m10*r.m02 + l.m11*r.m12 + l.m12*r.m22,
		l.m13,
		l.m20*r.m00 + l.m21*r.m10 + l.m22*r.m20,
		l.m20*r.m01 + l.m21*r.m11 + l.m22*r.m21,
		l.m20*r.m02 + l.m21*r.m12 + l.m22*r.m22,
		l.m23,
		l.m30*r.m00 + l.m31*r.m10 + l.m32*r.m20,
		l.m30*r.m01 + l.m31*r.m11 + l.m32*r.m21,
		l.m30*r.m02 + l.m31*r.m12 + l.m32*r.m22,
		l.m33,
	};
}
Mat44 operator*(const Mat44 &l, const Mat34 &r) {
	return {
		l.m00*r.m00 + l.m01*r.m10 + l.m02*r.m20,
		l.m00*r.m01 + l.m01*r.m11 + l.m02*r.m21,
		l.m00*r.m02 + l.m01*r.m12 + l.m02*r.m22,
		l.m00*r.m03 + l.m01*r.m13 + l.m02*r.m23 + l.m03,
		l.m10*r.m00 + l.m11*r.m10 + l.m12*r.m20,
		l.m10*r.m01 + l.m11*r.m11 + l.m12*r.m21,
		l.m10*r.m02 + l.m11*r.m12 + l.m12*r.m22,
		l.m10*r.m03 + l.m11*r.m13 + l.m12*r.m23 + l.m13,
		l.m20*r.m00 + l.m21*r.m10 + l.m22*r.m20,
		l.m20*r.m01 + l.m21*r.m11 + l.m22*r.m21,
		l.m20*r.m02 + l.m21*r.m12 + l.m22*r.m22,
		l.m20*r.m03 + l.m21*r.m13 + l.m22*r.m23 + l.m23,
		l.m30*r.m00 + l.m31*r.m10 + l.m32*r.m20,
		l.m30*r.m01 + l.m31*r.m11 + l.m32*r.m21,
		l.m30*r.m02 + l.m31*r.m12 + l.m32*r.m22,
		l.m30*r.m03 + l.m31*r.m13 + l.m32*r.m23 + l.m33,
	};
}
Mat44 operator*(const Mat44 &l, const Mat44 &r) {
	return {
		l.m00*r.m00 + l.m01*r.m10 + l.m02*r.m20 + l.m03*r.m30,
		l.m00*r.m01 + l.m01*r.m11 + l.m02*r.m21 + l.m03*r.m31,
		l.m00*r.m02 + l.m01*r.m12 + l.m02*r.m22 + l.m03*r.m32,
		l.m00*r.m03 + l.m01*r.m13 + l.m02*r.m23 + l.m03*r.m33,
		l.m10*r.m00 + l.m11*r.m10 + l.m12*r.m20 + l.m13*r.m30,
		l.m10*r.m01 + l.m11*r.m11 + l.m12*r.m21 + l.m13*r.m31,
		l.m10*r.m02 + l.m11*r.m12 + l.m12*r.m22 + l.m13*r.m32,
		l.m10*r.m03 + l.m11*r.m13 + l.m12*r.m23 + l.m13*r.m33,
		l.m20*r.m00 + l.m21*r.m10 + l.m22*r.m20 + l.m23*r.m30,
		l.m20*r.m01 + l.m21*r.m11 + l.m22*r.m21 + l.m23*r.m31,
		l.m20*r.m02 + l.m21*r.m12 + l.m22*r.m22 + l.m23*r.m32,
		l.m20*r.m03 + l.m21*r.m13 + l.m22*r.m23 + l.m23*r.m33,
		l.m30*r.m00 + l.m31*r.m10 + l.m32*r.m20 + l.m33*r.m30,
		l.m30*r.m01 + l.m31*r.m11 + l.m32*r.m21 + l.m33*r.m31,
		l.m30*r.m02 + l.m31*r.m12 + l.m32*r.m22 + l.m33*r.m32,
		l.m30*r.m03 + l.m31*r.m13 + l.m32*r.m23 + l.m33*r.m33,
	};
}

Mat23 operator*(const Mat23 &l, const Mat23 &r)
{
	return {
		l.m00*r.m00 + l.m01*r.m10,
		l.m10*r.m00 + l.m11*r.m10,
		l.m00*r.m01 + l.m01*r.m11,
		l.m10*r.m01 + l.m11*r.m11,
		l.m00*r.m02 + l.m01*r.m12 + l.m02,
		l.m10*r.m02 + l.m11*r.m12 + l.m12,
	};
}

// Multiply by vector

Vec4 operator*(const Mat34_3 &l, const Vec4 &r) {
	return { l.m03*r.w + r.x, l.m13*r.w + r.y, l.m23*r.w + r.z, r.w };
}
Vec4 operator*(const Mat33_D &l, const Vec4 &r) {
	return { l.m00*r.x, l.m11*r.y, l.m22*r.z, r.w };
}
Vec4 operator*(const Mat33 &l, const Vec4 &r) {
	return {
		l.m00*r.x + l.m01*r.y + l.m02*r.z,
		l.m10*r.x + l.m11*r.y + l.m12*r.z,
		l.m20*r.x + l.m21*r.y + l.m22*r.z,
		r.w,
	};
}
Vec4 operator*(const Mat34 &l, const Vec4 &r) {
	return {
		l.m00*r.x + l.m01*r.y + l.m02*r.z + l.m03*r.w,
		l.m10*r.x + l.m11*r.y + l.m12*r.z + l.m13*r.w,
		l.m20*r.x + l.m21*r.y + l.m22*r.z + l.m23*r.w,
		r.w,
	};
}
Vec4 operator*(const Mat44 &l, const Vec4 &r) {
	return {
		l.m00*r.x + l.m01*r.y + l.m02*r.z + l.m03*r.w,
		l.m10*r.x + l.m11*r.y + l.m12*r.z + l.m13*r.w,
		l.m20*r.x + l.m21*r.y + l.m22*r.z + l.m23*r.w,
		l.m30*r.x + l.m31*r.y + l.m32*r.z + l.m33*r.w,
	};
}

Vec3 operator*(const Mat23 &l, const Vec3 &r)
{
	return {
		l.m00*r.x + l.m01*r.y + l.m02*r.z,
		l.m10*r.x + l.m11*r.y + l.m12*r.z,
		r.z,
	};
}

// Transform point

Vec2 transformPoint(const Mat23 &l, const Vec2 &r)
{
	return {
		l.m00*r.x + l.m01*r.y + l.m02,
		l.m10*r.x + l.m11*r.y + l.m12,
	};
}

// Determinant

float determinant(const Mat34_3 &m) {
	return 1.0f;
}
float determinant(const Mat33_D &m) {
	return m.m00 * m.m11 * m.m22;
}
float determinant(const Mat33 &m) {
	return
		- m.m02*m.m11*m.m20 + m.m01*m.m12*m.m20 + m.m02*m.m10*m.m21
		- m.m00*m.m12*m.m21 - m.m01*m.m10*m.m22 + m.m00*m.m11*m.m22;
}
float determinant(const Mat34 &m) {
	return
		- m.m02*m.m11*m.m20 + m.m01*m.m12*m.m20 + m.m02*m.m10*m.m21
		- m.m00*m.m12*m.m21 - m.m01*m.m10*m.m22 + m.m00*m.m11*m.m22;
}
float determinant(const Mat44 &m) {
	return
		+ m.m03*m.m12*m.m21*m.m30 - m.m02*m.m13*m.m21*m.m30 - m.m03*m.m11*m.m22*m.m30 + m.m01*m.m13*m.m22*m.m30
		+ m.m02*m.m11*m.m23*m.m30 - m.m01*m.m12*m.m23*m.m30 - m.m03*m.m12*m.m20*m.m31 + m.m02*m.m13*m.m20*m.m31
		+ m.m03*m.m10*m.m22*m.m31 - m.m00*m.m13*m.m22*m.m31 - m.m02*m.m10*m.m23*m.m31 + m.m00*m.m12*m.m23*m.m31
		+ m.m03*m.m11*m.m20*m.m32 - m.m01*m.m13*m.m20*m.m32 - m.m03*m.m10*m.m21*m.m32 + m.m00*m.m13*m.m21*m.m32
		+ m.m01*m.m10*m.m23*m.m32 - m.m00*m.m11*m.m23*m.m32 - m.m02*m.m11*m.m20*m.m33 + m.m01*m.m12*m.m20*m.m33
		+ m.m02*m.m10*m.m21*m.m33 - m.m00*m.m12*m.m21*m.m33 - m.m01*m.m10*m.m22*m.m33 + m.m00*m.m11*m.m22*m.m33;
}

float determinant(const Mat23 &m) {
	return
		- m.m01*m.m10* + m.m00*m.m11;
}

// Inverse

Mat34_3 inverse(const Mat34_3 &m) {
	return { -m.m03, -m.m13, -m.m23 };
}
Mat33_D inverse(const Mat33_D &m) {
	return { 1.0f / m.m00, 1.0f / m.m11, 1.0f / m.m22 };
}
Mat33 inverse(const Mat33 &m) {
	float invDet = 1.0f / determinant(m);
	return {
		( - m.m12*m.m21 + m.m11*m.m22) * invDet,
		( + m.m12*m.m20 - m.m10*m.m22) * invDet,
		( - m.m11*m.m20 + m.m10*m.m21) * invDet,
		( + m.m02*m.m21 - m.m01*m.m22) * invDet,
		( - m.m02*m.m20 + m.m00*m.m22) * invDet,
		( + m.m01*m.m20 - m.m00*m.m21) * invDet,
		( - m.m02*m.m11 + m.m01*m.m12) * invDet,
		( + m.m02*m.m10 - m.m00*m.m12) * invDet,
		( - m.m01*m.m10 + m.m00*m.m11) * invDet,
	};
}
Mat34 inverse(const Mat34 &m) {
	float invDet = 1.0f / determinant(m);
	return {
		( - m.m12*m.m21 + m.m11*m.m22) * invDet,
		( + m.m12*m.m20 - m.m10*m.m22) * invDet,
		( - m.m11*m.m20 + m.m10*m.m21) * invDet,
		( + m.m02*m.m21 - m.m01*m.m22) * invDet,
		( - m.m02*m.m20 + m.m00*m.m22) * invDet,
		( + m.m01*m.m20 - m.m00*m.m21) * invDet,
		( - m.m02*m.m11 + m.m01*m.m12) * invDet,
		( + m.m02*m.m10 - m.m00*m.m12) * invDet,
		( - m.m01*m.m10 + m.m00*m.m11) * invDet,
		(m.m03*m.m12*m.m21 - m.m02*m.m13*m.m21 - m.m03*m.m11*m.m22 + m.m01*m.m13*m.m22 + m.m02*m.m11*m.m23 - m.m01*m.m12*m.m23) * invDet,
		(m.m02*m.m13*m.m20 - m.m03*m.m12*m.m20 + m.m03*m.m10*m.m22 - m.m00*m.m13*m.m22 - m.m02*m.m10*m.m23 + m.m00*m.m12*m.m23) * invDet,
		(m.m03*m.m11*m.m20 - m.m01*m.m13*m.m20 - m.m03*m.m10*m.m21 + m.m00*m.m13*m.m21 + m.m01*m.m10*m.m23 - m.m00*m.m11*m.m23) * invDet,
	};
}
Mat44 inverse(const Mat44 &m) {
	float invDet = 1.0f / determinant(m);
	return {
		(m.m12*m.m23*m.m31 - m.m13*m.m22*m.m31 + m.m13*m.m21*m.m32 - m.m11*m.m23*m.m32 - m.m12*m.m21*m.m33 + m.m11*m.m22*m.m33) * invDet,
		(m.m13*m.m22*m.m30 - m.m12*m.m23*m.m30 - m.m13*m.m20*m.m32 + m.m10*m.m23*m.m32 + m.m12*m.m20*m.m33 - m.m10*m.m22*m.m33) * invDet,
		(m.m11*m.m23*m.m30 - m.m13*m.m21*m.m30 + m.m13*m.m20*m.m31 - m.m10*m.m23*m.m31 - m.m11*m.m20*m.m33 + m.m10*m.m21*m.m33) * invDet,
		(m.m12*m.m21*m.m30 - m.m11*m.m22*m.m30 - m.m12*m.m20*m.m31 + m.m10*m.m22*m.m31 + m.m11*m.m20*m.m32 - m.m10*m.m21*m.m32) * invDet,

		(m.m03*m.m22*m.m31 - m.m02*m.m23*m.m31 - m.m03*m.m21*m.m32 + m.m01*m.m23*m.m32 + m.m02*m.m21*m.m33 - m.m01*m.m22*m.m33) * invDet,
		(m.m02*m.m23*m.m30 - m.m03*m.m22*m.m30 + m.m03*m.m20*m.m32 - m.m00*m.m23*m.m32 - m.m02*m.m20*m.m33 + m.m00*m.m22*m.m33) * invDet,
		(m.m03*m.m21*m.m30 - m.m01*m.m23*m.m30 - m.m03*m.m20*m.m31 + m.m00*m.m23*m.m31 + m.m01*m.m20*m.m33 - m.m00*m.m21*m.m33) * invDet,
		(m.m01*m.m22*m.m30 - m.m02*m.m21*m.m30 + m.m02*m.m20*m.m31 - m.m00*m.m22*m.m31 - m.m01*m.m20*m.m32 + m.m00*m.m21*m.m32) * invDet,

		(m.m02*m.m13*m.m31 - m.m03*m.m12*m.m31 + m.m03*m.m11*m.m32 - m.m01*m.m13*m.m32 - m.m02*m.m11*m.m33 + m.m01*m.m12*m.m33) * invDet,
		(m.m03*m.m12*m.m30 - m.m02*m.m13*m.m30 - m.m03*m.m10*m.m32 + m.m00*m.m13*m.m32 + m.m02*m.m10*m.m33 - m.m00*m.m12*m.m33) * invDet,
		(m.m01*m.m13*m.m30 - m.m03*m.m11*m.m30 + m.m03*m.m10*m.m31 - m.m00*m.m13*m.m31 - m.m01*m.m10*m.m33 + m.m00*m.m11*m.m33) * invDet,
		(m.m02*m.m11*m.m30 - m.m01*m.m12*m.m30 - m.m02*m.m10*m.m31 + m.m00*m.m12*m.m31 + m.m01*m.m10*m.m32 - m.m00*m.m11*m.m32) * invDet,

		(m.m03*m.m12*m.m21 - m.m02*m.m13*m.m21 - m.m03*m.m11*m.m22 + m.m01*m.m13*m.m22 + m.m02*m.m11*m.m23 - m.m01*m.m12*m.m23) * invDet,
		(m.m02*m.m13*m.m20 - m.m03*m.m12*m.m20 + m.m03*m.m10*m.m22 - m.m00*m.m13*m.m22 - m.m02*m.m10*m.m23 + m.m00*m.m12*m.m23) * invDet,
		(m.m03*m.m11*m.m20 - m.m01*m.m13*m.m20 - m.m03*m.m10*m.m21 + m.m00*m.m13*m.m21 + m.m01*m.m10*m.m23 - m.m00*m.m11*m.m23) * invDet,
		(m.m01*m.m12*m.m20 - m.m02*m.m11*m.m20 + m.m02*m.m10*m.m21 - m.m00*m.m12*m.m21 - m.m01*m.m10*m.m22 + m.m00*m.m11*m.m22) * invDet,
	};
}

Mat23 inverse(const Mat23 &m)
{
	float invDet = 1.0f / determinant(m);
	return {
		( - m.m12 + m.m11) * invDet,
		( + m.m12 - m.m10) * invDet,
		( + m.m02 - m.m01) * invDet,
		( - m.m02 + m.m00) * invDet,
		( - m.m02*m.m11 + m.m01*m.m12) * invDet,
		( + m.m02*m.m10 - m.m00*m.m12) * invDet,
	};
}

// Initializers

namespace mat {

Mat34_3 translateX(float x) { return { x, 0, 0 }; }
Mat34_3 translateY(float y) { return { 0, y, 0 }; }
Mat34_3 translateZ(float z) { return { 0, 0, z }; }
Mat34_3 translate(float x, float y, float z) {
	return { x, y, z };
}
Mat34_3 translate(const Vec3 &amount) {
	return { amount.x, amount.y, amount.z };
}

Mat33_D scaleX(float x) { return { x, 0, 0 }; }
Mat33_D scaleY(float y) { return { 0, y, 0 }; }
Mat33_D scaleZ(float z) { return { 0, 0, z }; }
Mat33_D scale(float s) { return { s, s, s }; }
Mat33_D scale(float x, float y, float z) {
	return { x, y, z };
}
Mat33_D scale(const Vec3 &amount) {
	return { amount.x, amount.y, amount.z };
}

Mat33 rotateX(float angle) {
	float c = cosf(angle), s = sinf(angle);
	return {
		1, 0,  0,
		0, c, -s,
		0, s,  c,
	};
}

Mat33 rotateY(float angle) {
	float c = cosf(angle), s = sinf(angle);
	return {
		 c, 0, s,
		 0, 1, 0,
		-s, 0, c,
	};
}

Mat33 rotateZ(float angle) {
	float c = cosf(angle), s = sinf(angle);
	return {
		c, -s, 0,
		s,  c, 0,
		0,  0, 1,
	};
}

Mat44 perspectiveD3D(float fov, float aspect, float near, float far)
{
	float fovScale = 1.0f / tanf(fov / 2.0f);
	return {
		fovScale / aspect, 0, 0, 0,
		0, fovScale, 0, 0,
		0, 0, far / (far-near), -(far*near)/(far-near),
		0, 0, 1, 0,
	};
}

Mat44 perspectiveGL(float fov, float aspect, float near, float far)
{
	float fovScale = 1.0f / tanf(fov / 2.0f);
	return {
		fovScale / aspect, 0, 0, 0,
		0, fovScale, 0, 0,
		0, 0, (far+near) / (far-near), -2.0f * (far*near)/(far-near),
		0, 0, 1, 0,
	};
}

}

namespace mat2D {

Mat23 translateX(float x) { return { 1, 0, 0, 1, x, 0 }; }
Mat23 translateY(float y) { return { 1, 0, 0, 1, 0, y }; }
Mat23 translate(float x, float y) { return { 1, 0, 0, 1, x, y }; }
Mat23 translate(const Vec2 &amount) { return { 1, 0, 0, 1, amount.x, amount.y }; }

Mat23 scaleX(float x) { return { x, 0, 0, 1, 0, 0 }; }
Mat23 scaleY(float y) { return { 1, 0, 0, y, 0, 0 }; }
Mat23 scale(float s) { return { s, 0, 0, s, 0, 0 }; }
Mat23 scale(float x, float y) { return { x, 0, 0, y, 0, 0 }; }
Mat23 scale(const Vec2 &amount) { return { amount.x, 0, 0, amount.y, 0, 0 }; }

Mat23 rotate(float angle) {
	float c = cosf(angle), s = sinf(angle);
	return {
		c, -s,
		s,  c,
		0, 0,
	};
}

}

}
