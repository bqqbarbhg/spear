#pragma once

#include "sf/Vector.h"
#include "sf/Quaternion.h"
#include "sf/Matrix.h"

namespace cl {

struct VisualTransform
{
	sf::Vec3 position;
	sf::Quat rotation;
	sf::Vec3 scale;

	sf_forceinline VisualTransform operator+(const VisualTransform &rhs) const { return { position + rhs.position, rotation + rhs.rotation, scale + rhs.scale }; }
	sf_forceinline VisualTransform operator-(const VisualTransform &rhs) const { return { position - rhs.position, rotation - rhs.rotation, scale - rhs.scale }; }
	sf_forceinline VisualTransform operator*(float rhs) const { return { position * rhs, rotation * rhs, scale * rhs }; }
	sf_forceinline VisualTransform &operator+=(const VisualTransform &rhs) { position += rhs.position; rotation += rhs.rotation; scale += rhs.scale; return *this; }
	sf_forceinline VisualTransform &operator-=(const VisualTransform &rhs) { position -= rhs.position; rotation -= rhs.rotation; scale -= rhs.scale; return *this; }
	sf_forceinline VisualTransform &operator*=(float rhs) { position *= rhs; rotation *= rhs; scale *= rhs; return *this; }

	sf_forceinline static VisualTransform lcomb4(
		const VisualTransform &a, float fa,
		const VisualTransform &b, float fb,
		const VisualTransform &c, float fc,
		const VisualTransform &d, float fd)
	{
		VisualTransform ret;
		ret.position.x = a.position.x*fa + b.position.x*fb + c.position.x*fc + d.position.x*fd;
		ret.position.y = a.position.y*fa + b.position.y*fb + c.position.y*fc + d.position.y*fd;
		ret.position.z = a.position.z*fa + b.position.z*fb + c.position.z*fc + d.position.z*fd;
		ret.rotation.x = a.rotation.x*fa + b.rotation.x*fb + c.rotation.x*fc + d.rotation.x*fd;
		ret.rotation.y = a.rotation.y*fa + b.rotation.y*fb + c.rotation.y*fc + d.rotation.y*fd;
		ret.rotation.z = a.rotation.z*fa + b.rotation.z*fb + c.rotation.z*fc + d.rotation.z*fd;
		ret.rotation.w = a.rotation.w*fa + b.rotation.w*fb + c.rotation.w*fc + d.rotation.w*fd;
		ret.scale.x = a.scale.x*fa + b.scale.x*fb + c.scale.x*fc + d.scale.x*fd;
		ret.scale.y = a.scale.y*fa + b.scale.y*fb + c.scale.y*fc + d.scale.y*fd;
		ret.scale.z = a.scale.z*fa + b.scale.z*fb + c.scale.z*fc + d.scale.z*fd;
		return ret;
	}

	sf_forceinline sf::Mat34 asMatrix() const {
		return sf::mat::world(position, rotation, scale);
	}
};

struct VisualHermite
{
	VisualTransform p0, d0;
	VisualTransform p1, d1;

	VisualTransform evaluate(float t) const;
	VisualTransform derivative(float t) const;
};

}
