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

}
