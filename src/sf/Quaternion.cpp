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

}
