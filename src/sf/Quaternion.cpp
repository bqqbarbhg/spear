#include "Quaternion.h"
#include "Reflection.h"

namespace sf {

template<>
void initType<Quat>(Type *t)
{
	static Field fields[] = {
		sf_field(Quat, x),
		sf_field(Quat, y),
		sf_field(Quat, z),
		sf_field(Quat, w),
	};
	sf_struct(t, sf::Quat, fields, Type::CompactString | Type::IsPod);
}

Quat eulerAnglesToQuat(float x, float y, float z, EulerOrder order)
{
	x *= 0.5f;
	y *= 0.5f;
	z *= 0.5f;
	float cx = cosf(x), sx = sinf(x);
	float cy = cosf(y), sy = sinf(y);
	float cz = cosf(z), sz = sinf(z);
	sf::Quat q;

	switch (order) {
	case EulerOrder::XYZ:
		q.x = -cx*sy*sz + cy*cz*sx;
		q.y = cx*cz*sy + cy*sx*sz;
		q.z = cx*cy*sz - cz*sx*sy;
		q.w = cx*cy*cz + sx*sy*sz;
		break;
	case EulerOrder::XZY:
		q.x = cx*sy*sz + cy*cz*sx;
		q.y = cx*cz*sy + cy*sx*sz;
		q.z = cx*cy*sz - cz*sx*sy;
		q.w = cx*cy*cz - sx*sy*sz;
		break;
	case EulerOrder::YZX:
		q.x = -cx*sy*sz + cy*cz*sx;
		q.y = cx*cz*sy - cy*sx*sz;
		q.z = cx*cy*sz + cz*sx*sy;
		q.w = cx*cy*cz + sx*sy*sz;
		break;
	case EulerOrder::YXZ:
		q.x = -cx*sy*sz + cy*cz*sx;
		q.y = cx*cz*sy + cy*sx*sz;
		q.z = cx*cy*sz + cz*sx*sy;
		q.w = cx*cy*cz - sx*sy*sz;
		break;
	case EulerOrder::ZXY:
		q.x = cx*sy*sz + cy*cz*sx;
		q.y = cx*cz*sy - cy*sx*sz;
		q.z = cx*cy*sz - cz*sx*sy;
		q.w = cx*cy*cz + sx*sy*sz;
		break;
	case EulerOrder::ZYX:
		q.x = cx*sy*sz + cy*cz*sx;
		q.y = cx*cz*sy - cy*sx*sz;
		q.z = cx*cy*sz + cz*sx*sy;
		q.w = cx*cy*cz - sx*sy*sz;
		break;
	default:
		q.x = q.y = q.z = 0.0; q.w = 1.0;
		break;
	}

	return q;
}

Quat eulerAnglesToQuat(const sf::Vec3 &v, EulerOrder order)
{
	return eulerAnglesToQuat(v.x, v.y, v.z, order);
}

Quat axesToQuat(const sf::Vec3 &x, const sf::Vec3 &y, const sf::Vec3 &z)
{
	sf::Quat quat;

	float trace = x.x + y.y + z.z;
	if (trace > 0.0) {
		float s = sf::sqrt(sf::max(0.0f, 1.0f + trace)) * 2.0f, rs = 1.0f / s;
		quat.x = (y.z - z.y) * rs;
		quat.y = (z.x - x.z) * rs;
		quat.z = (x.y - y.x) * rs;
		quat.w = 0.25f * s;
	}
	else if (x.x > sf::max(y.y, z.z)) {
		float s = sf::sqrt(sf::max(0.0f, 1.0f + x.x - y.y - z.z)) * 2.0f, rs = 1.0f / s;
		quat.x = 0.25f * s;
		quat.y = (y.x + x.y) * rs;
		quat.z = (z.x + x.z) * rs;
		quat.w = (y.z - z.y) * rs;
	}
	else if (y.y > z.z) {
		float s = sf::sqrt(sf::max(0.0f, 1.0f - x.x + y.y - z.z)) * 2.0f, rs = 1.0f / s;
		quat.x = (y.x + x.y) * rs;
		quat.y = 0.25f * s;
		quat.z = (z.y + y.z) * rs;
		quat.w = (z.x - x.z) * rs;
	}
	else {
		float s = sf::sqrt(sf::max(0.0f, 1.0f - x.x - y.y + z.z)) * 2.0f, rs = 1.0f / s;
		quat.x = (z.x + x.z) * rs;
		quat.y = (z.y + y.z) * rs;
		quat.z = 0.25f * s;
		quat.w = (x.y - y.x) * rs;
	}

	return quat;
}

}
