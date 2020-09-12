#include "Matrix.h"
#include "Quaternion.h"

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

// Multiply by scalar

Mat33 operator*(const Mat33 &l, float r)
{
	Mat33 m = Uninit;
	m.m00 = l.m00 * r; m.m10 = l.m10 * r; m.m20 = l.m20 * r;
	m.m01 = l.m01 * r; m.m11 = l.m11 * r; m.m21 = l.m21 * r;
	m.m02 = l.m02 * r; m.m12 = l.m12 * r; m.m22 = l.m22 * r;
	return m;
}

Mat33_D operator*(const Mat33_D &l, float r)
{
	Mat33_D m = Uninit;
	m.m00 = l.m00 * r;
	m.m11 = l.m11 * r;
	m.m22 = l.m22 * r;
	return m;
}

Mat34_3 operator*(const Mat34_3 &l, float r)
{
	Mat34_3 m = Uninit;
	m.m03 = l.m03 * r; m.m13 = l.m13 * r; m.m23 = l.m23 * r;
	return m;
}

Mat34 operator*(const Mat34 &l, float r)
{
	Mat34 m = Uninit;
	m.m00 = l.m00 * r; m.m10 = l.m10 * r; m.m20 = l.m20 * r;
	m.m01 = l.m01 * r; m.m11 = l.m11 * r; m.m21 = l.m21 * r;
	m.m02 = l.m02 * r; m.m12 = l.m12 * r; m.m22 = l.m22 * r;
	m.m03 = l.m03 * r; m.m13 = l.m13 * r; m.m23 = l.m23 * r;
	return m;
}

Mat44 operator*(const Mat44 &l, float r)
{
	Mat44 m = Uninit;
	m.m00 = l.m00 * r; m.m10 = l.m10 * r; m.m20 = l.m20 * r; m.m30 = l.m30 * r;
	m.m01 = l.m01 * r; m.m11 = l.m11 * r; m.m21 = l.m21 * r; m.m31 = l.m31 * r;
	m.m02 = l.m02 * r; m.m12 = l.m12 * r; m.m22 = l.m22 * r; m.m32 = l.m32 * r;
	m.m03 = l.m03 * r; m.m13 = l.m13 * r; m.m23 = l.m23 * r; m.m33 = l.m33 * r;
	return m;
}

Mat33 lerpFromIdentity(const Mat33 &l, float r)
{
	float s = 1.0f - r;
	Mat33 m = Uninit;
	m.m00 = l.m00 * r + s; m.m10 = l.m10 * r; m.m20 = l.m20 * r;
	m.m01 = l.m01 * r; m.m11 = l.m11 * r + s; m.m21 = l.m21 * r;
	m.m02 = l.m02 * r; m.m12 = l.m12 * r; m.m22 = l.m22 * r + s;
	return m;
}

Mat33_D lerpFromIdentity(const Mat33_D &l, float r)
{
	float s = 1.0f - r;
	Mat33_D m = Uninit;
	m.m00 = l.m00 * r + s;
	m.m11 = l.m11 * r + s;
	m.m22 = l.m22 * r + s;
	return m;
}

Mat34_3 lerpFromIdentity(const Mat34_3 &l, float r)
{
	Mat34_3 m = Uninit;
	m.m03 = l.m03 * r; m.m13 = l.m13 * r; m.m23 = l.m23 * r;
	return m;
}

Mat34 lerpFromIdentity(const Mat34 &l, float r)
{
	float s = 1.0f - r;
	Mat34 m = Uninit;
	m.m00 = l.m00 * r + s; m.m10 = l.m10 * r; m.m20 = l.m20 * r;
	m.m01 = l.m01 * r; m.m11 = l.m11 * r + s; m.m21 = l.m21 * r;
	m.m02 = l.m02 * r; m.m12 = l.m12 * r; m.m22 = l.m22 * r + s;
	m.m03 = l.m03 * r; m.m13 = l.m13 * r; m.m23 = l.m23 * r;
	return m;
}

Mat44 lerpFromIdentity(const Mat44 &l, float r)
{
	float s = 1.0f - r;
	Mat44 m = Uninit;
	m.m00 = l.m00 * r + s; m.m10 = l.m10 * r; m.m20 = l.m20 * r; m.m30 = l.m30 * r;
	m.m01 = l.m01 * r; m.m11 = l.m11 * r + s; m.m21 = l.m21 * r; m.m31 = l.m31 * r;
	m.m02 = l.m02 * r; m.m12 = l.m12 * r; m.m22 = l.m22 * r + s; m.m32 = l.m32 * r;
	m.m03 = l.m03 * r; m.m13 = l.m13 * r; m.m23 = l.m23 * r; m.m33 = l.m33 * r + s;
	return m;
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

Vec3 transformPoint(const Mat33 &l, const Vec3 &r)
{
	return {
		l.m00*r.x + l.m01*r.y + l.m02*r.z,
		l.m10*r.x + l.m11*r.y + l.m12*r.z,
		l.m20*r.x + l.m21*r.y + l.m22*r.z,
	};
}

Vec3 transformPoint(const Mat34 &l, const Vec3 &r)
{
	return {
		l.m00*r.x + l.m01*r.y + l.m02*r.z + l.m03,
		l.m10*r.x + l.m11*r.y + l.m12*r.z + l.m13,
		l.m20*r.x + l.m21*r.y + l.m22*r.z + l.m23,
	};
}

Vec2 transformPoint(const Mat23 &l, const Vec2 &r)
{
	return {
		l.m00*r.x + l.m01*r.y + l.m02,
		l.m10*r.x + l.m11*r.y + l.m12,
	};
}

// Transform direction

Vec3 transformDirection(const Mat33 &l, const Vec3 &r)
{
	return {
		l.m00*r.x + l.m01*r.y + l.m02*r.z,
		l.m10*r.x + l.m11*r.y + l.m12*r.z,
		l.m20*r.x + l.m21*r.y + l.m22*r.z,
	};
}

Vec3 transformDirection(const Mat34 &l, const Vec3 &r)
{
	return {
		l.m00*r.x + l.m01*r.y + l.m02*r.z,
		l.m10*r.x + l.m11*r.y + l.m12*r.z,
		l.m20*r.x + l.m21*r.y + l.m22*r.z,
	};
}

Vec3 transformDirectionAbs(const Mat33 &l, const Vec3 &r)
{
	return {
		sf::abs(l.m00)*r.x + sf::abs(l.m01)*r.y + sf::abs(l.m02)*r.z,
		sf::abs(l.m10)*r.x + sf::abs(l.m11)*r.y + sf::abs(l.m12)*r.z,
		sf::abs(l.m20)*r.x + sf::abs(l.m21)*r.y + sf::abs(l.m22)*r.z,
	};
}

Vec3 transformDirectionAbs(const Mat34 &l, const Vec3 &r)
{
	return {
		sf::abs(l.m00)*r.x + sf::abs(l.m01)*r.y + sf::abs(l.m02)*r.z,
		sf::abs(l.m10)*r.x + sf::abs(l.m11)*r.y + sf::abs(l.m12)*r.z,
		sf::abs(l.m20)*r.x + sf::abs(l.m21)*r.y + sf::abs(l.m22)*r.z,
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

// Transpose

Mat44 transpose(const Mat34_3 &m)
{
	Mat44 r;
	r.m30 = m.m03;
	r.m31 = m.m13;
	r.m32 = m.m23;
	return r;
}

Mat33_D transpose(const Mat33_D &m)
{
	return m;
}

Mat33 transpose(const Mat33 &m)
{
	Mat33 r = Uninit;
	r.m00 = m.m00;
	r.m01 = m.m10;
	r.m02 = m.m20;
	r.m10 = m.m01;
	r.m11 = m.m11;
	r.m12 = m.m21;
	r.m20 = m.m02;
	r.m21 = m.m12;
	r.m22 = m.m22;
	return r;
}

Mat44 transpose(const Mat34 &m)
{
	Mat44 r = Uninit;
	r.m00 = m.m00;
	r.m01 = m.m10;
	r.m02 = m.m20;
	r.m03 = 0.0f;
	r.m10 = m.m01;
	r.m11 = m.m11;
	r.m12 = m.m21;
	r.m13 = 0.0f;
	r.m20 = m.m02;
	r.m21 = m.m12;
	r.m22 = m.m22;
	r.m23 = 0.0f;
	r.m30 = m.m03;
	r.m31 = m.m13;
	r.m32 = m.m23;
	r.m33 = 1.0f;
	return r;
}

Mat44 transpose(const Mat44 &m)
{
	Mat44 r = Uninit;
	r.m00 = m.m00;
	r.m01 = m.m10;
	r.m02 = m.m20;
	r.m03 = m.m30;
	r.m10 = m.m01;
	r.m11 = m.m11;
	r.m12 = m.m21;
	r.m13 = m.m31;
	r.m20 = m.m02;
	r.m21 = m.m12;
	r.m22 = m.m22;
	r.m23 = m.m32;
	r.m30 = m.m03;
	r.m31 = m.m13;
	r.m32 = m.m23;
	r.m33 = m.m33;
	return r;
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
		( + m.m02*m.m21 - m.m01*m.m22) * invDet,
		( - m.m02*m.m11 + m.m01*m.m12) * invDet,
		(m.m03*m.m12*m.m21 - m.m02*m.m13*m.m21 - m.m03*m.m11*m.m22 + m.m01*m.m13*m.m22 + m.m02*m.m11*m.m23 - m.m01*m.m12*m.m23) * invDet,

		( + m.m12*m.m20 - m.m10*m.m22) * invDet,
		( - m.m02*m.m20 + m.m00*m.m22) * invDet,
		( + m.m02*m.m10 - m.m00*m.m12) * invDet,
		(m.m02*m.m13*m.m20 - m.m03*m.m12*m.m20 + m.m03*m.m10*m.m22 - m.m00*m.m13*m.m22 - m.m02*m.m10*m.m23 + m.m00*m.m12*m.m23) * invDet,

		( - m.m11*m.m20 + m.m10*m.m21) * invDet,
		( + m.m01*m.m20 - m.m00*m.m21) * invDet,
		( - m.m01*m.m10 + m.m00*m.m11) * invDet,
		(m.m03*m.m11*m.m20 - m.m01*m.m13*m.m20 - m.m03*m.m10*m.m21 + m.m00*m.m13*m.m21 + m.m01*m.m10*m.m23 - m.m00*m.m11*m.m23) * invDet,
	};
}
Mat44 inverse(const Mat44 &m) {
	float invDet = 1.0f / determinant(m);
	return {
		(m.m12*m.m23*m.m31 - m.m13*m.m22*m.m31 + m.m13*m.m21*m.m32 - m.m11*m.m23*m.m32 - m.m12*m.m21*m.m33 + m.m11*m.m22*m.m33) * invDet,
		(m.m03*m.m22*m.m31 - m.m02*m.m23*m.m31 - m.m03*m.m21*m.m32 + m.m01*m.m23*m.m32 + m.m02*m.m21*m.m33 - m.m01*m.m22*m.m33) * invDet,
		(m.m02*m.m13*m.m31 - m.m03*m.m12*m.m31 + m.m03*m.m11*m.m32 - m.m01*m.m13*m.m32 - m.m02*m.m11*m.m33 + m.m01*m.m12*m.m33) * invDet,
		(m.m03*m.m12*m.m21 - m.m02*m.m13*m.m21 - m.m03*m.m11*m.m22 + m.m01*m.m13*m.m22 + m.m02*m.m11*m.m23 - m.m01*m.m12*m.m23) * invDet,

		(m.m13*m.m22*m.m30 - m.m12*m.m23*m.m30 - m.m13*m.m20*m.m32 + m.m10*m.m23*m.m32 + m.m12*m.m20*m.m33 - m.m10*m.m22*m.m33) * invDet,
		(m.m02*m.m23*m.m30 - m.m03*m.m22*m.m30 + m.m03*m.m20*m.m32 - m.m00*m.m23*m.m32 - m.m02*m.m20*m.m33 + m.m00*m.m22*m.m33) * invDet,
		(m.m03*m.m12*m.m30 - m.m02*m.m13*m.m30 - m.m03*m.m10*m.m32 + m.m00*m.m13*m.m32 + m.m02*m.m10*m.m33 - m.m00*m.m12*m.m33) * invDet,
		(m.m02*m.m13*m.m20 - m.m03*m.m12*m.m20 + m.m03*m.m10*m.m22 - m.m00*m.m13*m.m22 - m.m02*m.m10*m.m23 + m.m00*m.m12*m.m23) * invDet,

		(m.m11*m.m23*m.m30 - m.m13*m.m21*m.m30 + m.m13*m.m20*m.m31 - m.m10*m.m23*m.m31 - m.m11*m.m20*m.m33 + m.m10*m.m21*m.m33) * invDet,
		(m.m03*m.m21*m.m30 - m.m01*m.m23*m.m30 - m.m03*m.m20*m.m31 + m.m00*m.m23*m.m31 + m.m01*m.m20*m.m33 - m.m00*m.m21*m.m33) * invDet,
		(m.m01*m.m13*m.m30 - m.m03*m.m11*m.m30 + m.m03*m.m10*m.m31 - m.m00*m.m13*m.m31 - m.m01*m.m10*m.m33 + m.m00*m.m11*m.m33) * invDet,
		(m.m03*m.m11*m.m20 - m.m01*m.m13*m.m20 - m.m03*m.m10*m.m21 + m.m00*m.m13*m.m21 + m.m01*m.m10*m.m23 - m.m00*m.m11*m.m23) * invDet,

		(m.m12*m.m21*m.m30 - m.m11*m.m22*m.m30 - m.m12*m.m20*m.m31 + m.m10*m.m22*m.m31 + m.m11*m.m20*m.m32 - m.m10*m.m21*m.m32) * invDet,
		(m.m01*m.m22*m.m30 - m.m02*m.m21*m.m30 + m.m02*m.m20*m.m31 - m.m00*m.m22*m.m31 - m.m01*m.m20*m.m32 + m.m00*m.m21*m.m32) * invDet,
		(m.m02*m.m11*m.m30 - m.m01*m.m12*m.m30 - m.m02*m.m10*m.m31 + m.m00*m.m12*m.m31 + m.m01*m.m10*m.m32 - m.m00*m.m11*m.m32) * invDet,
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

Mat34 inverseBasis(const sf::Vec3 &x, const sf::Vec3 &y, const sf::Vec3 &z, const sf::Vec3 &origin)
{
    Mat34 r = sf::Uninit;
    r.m00 = x.x;
    r.m01 = x.y;
    r.m02 = x.z;
    r.m03 = -dot(origin, x);
    r.m10 = y.x;
    r.m11 = y.y;
    r.m12 = y.z;
    r.m13 = -dot(origin, y);
    r.m20 = z.x;
    r.m21 = z.y;
    r.m22 = z.z;
    r.m23 = -dot(origin, z);
	return r;
}

Mat34 look(const sf::Vec3 &eye, const sf::Vec3 &dir, const sf::Vec3 &up)
{
    Vec3 dir2 = normalize(dir);
    Vec3 right = normalize(cross(dir2, up));
    Vec3 up2 = cross(right, dir2);
    return inverseBasis(right, up2, dir2, eye);
}

Mat34 world(const sf::Vec3 &translation, const sf::Quat &rotation, float scale)
{
	sf::Quat q = rotation;
	float s2 = 2.0f * scale;
	float xx = q.x*q.x, xy = q.x*q.y, xz = q.x*q.z, xw = q.x*q.w;
	float yy = q.y*q.y, yz = q.y*q.z, yw = q.y*q.w;
	float zz = q.z*q.z, zw = q.z*q.w;
	Mat34 m = sf::Uninit;
	m.m00 = s2 * (- yy - zz + 0.5f);
	m.m10 = s2 * (+ xy + zw);
	m.m20 = s2 * (- yw + xz);
	m.m01 = s2 * (- zw + xy);
	m.m11 = s2 * (- xx - zz + 0.5f);
	m.m21 = s2 * (+ xw + yz);
	m.m02 = s2 * (+ xz + yw);
	m.m12 = s2 * (- xw + yz);
	m.m22 = s2 * (- xx - yy + 0.5f);
	m.m03 = translation.x;
	m.m13 = translation.y;
	m.m23 = translation.z;
	return m;
}

Mat34 world(const sf::Vec3 &translation, const sf::Quat &rotation, const sf::Vec3 &scale)
{
	sf::Quat q = rotation;
	float sx = 2.0f * scale.x, sy = 2.0f * scale.y, sz = 2.0f * scale.z;
	float xx = q.x*q.x, xy = q.x*q.y, xz = q.x*q.z, xw = q.x*q.w;
	float yy = q.y*q.y, yz = q.y*q.z, yw = q.y*q.w;
	float zz = q.z*q.z, zw = q.z*q.w;
	Mat34 m = sf::Uninit;
	m.m00 = sx * (- yy - zz + 0.5f);
	m.m10 = sx * (+ xy + zw);
	m.m20 = sx * (- yw + xz);
	m.m01 = sy * (- zw + xy);
	m.m11 = sy * (- xx - zz + 0.5f);
	m.m21 = sy * (+ xw + yz);
	m.m02 = sz * (+ xz + yw);
	m.m12 = sz * (- xw + yz);
	m.m22 = sz * (- xx - yy + 0.5f);
	m.m03 = translation.x;
	m.m13 = translation.y;
	m.m23 = translation.z;
	return m;
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

Mat44 orthoSkewedD3D(const sf::Vec2 &extent, const sf::Vec2 &skew, float near, float far)
{
	Mat44 mat;

	mat.m00 = 1.0f / extent.x;
	mat.m11 = 1.0f / extent.y;
	mat.m02 = -skew.x * mat.m00;
	mat.m12 = -skew.y * mat.m11;
	mat.m22 = 1.0f / (far - near);
	mat.m23 = near / (near - far);

	return mat;
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
