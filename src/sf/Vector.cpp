#include "Vector.h"
#include "Reflection.h"

namespace sf {

template<>
void initType<Vec2>(Type *t)
{
	static Field fields[] = {
		sf_field(Vec2, x),
		sf_field(Vec2, y),
	};
	sf_struct(t, sf::Vec2, fields, Type::CompactString | Type::IsPod);
}

template<>
void initType<Vec3>(Type *t)
{
	static Field fields[] = {
		sf_field(Vec3, x),
		sf_field(Vec3, y),
		sf_field(Vec3, z),
	};
	sf_struct(t, sf::Vec3, fields, Type::CompactString | Type::IsPod);
}

template<>
void initType<Vec4>(Type *t)
{
	static Field fields[] = {
		sf_field(Vec4, x),
		sf_field(Vec4, y),
		sf_field(Vec4, z),
		sf_field(Vec4, w),
	};
	sf_struct(t, sf::Vec4, fields, Type::CompactString | Type::IsPod);
}

template<>
void initType<Vec2i>(Type *t)
{
	static Field fields[] = {
		sf_field(Vec2i, x),
		sf_field(Vec2i, y),
	};
	sf_struct(t, sf::Vec2i, fields, Type::CompactString | Type::IsPod);
}

template<>
void initType<Vec3i>(Type *t)
{
	static Field fields[] = {
		sf_field(Vec3i, x),
		sf_field(Vec3i, y),
		sf_field(Vec3i, z),
	};
	sf_struct(t, sf::Vec3i, fields, Type::CompactString | Type::IsPod);
}

template<>
void initType<Vec4i>(Type *t)
{
	static Field fields[] = {
		sf_field(Vec4i, x),
		sf_field(Vec4i, y),
		sf_field(Vec4i, z),
		sf_field(Vec4i, w),
	};
	sf_struct(t, sf::Vec4i, fields, Type::CompactString | Type::IsPod);
}

}
