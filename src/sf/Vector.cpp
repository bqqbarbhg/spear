#include "Vector.h"
#include "Reflection.h"

namespace sf {

template<>
Type *initType<Vec2>()
{
	static Field fields[] = {
		sf_field(Vec2, x),
		sf_field(Vec2, y),
	};
	return sf_struct(Vec2, fields);
}

template<>
Type *initType<Vec3>()
{
	static Field fields[] = {
		sf_field(Vec3, x),
		sf_field(Vec3, y),
		sf_field(Vec3, z),
	};
	return sf_struct(Vec3, fields);
}

template<>
Type *initType<Vec4>()
{
	static Field fields[] = {
		sf_field(Vec4, x),
		sf_field(Vec4, y),
		sf_field(Vec4, z),
		sf_field(Vec4, w),
	};
	return sf_struct(Vec4, fields);
}

template<>
Type *initType<Vec2i>()
{
	static Field fields[] = {
		sf_field(Vec2i, x),
		sf_field(Vec2i, y),
	};
	return sf_struct(Vec2i, fields);
}

template<>
Type *initType<Vec3i>()
{
	static Field fields[] = {
		sf_field(Vec3i, x),
		sf_field(Vec3i, y),
		sf_field(Vec3i, z),
	};
	return sf_struct(Vec3i, fields);
}

template<>
Type *initType<Vec4i>()
{
	static Field fields[] = {
		sf_field(Vec4i, x),
		sf_field(Vec4i, y),
		sf_field(Vec4i, z),
		sf_field(Vec4i, w),
	};
	return sf_struct(Vec4i, fields);
}

}
