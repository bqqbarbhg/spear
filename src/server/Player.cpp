#include "Player.h"

#include "sf/Reflection.h"

namespace sf {

template<> void initType<sv2::Player>(Type *t)
{
	static Field fields[] = {
		sf_field(sv2::Player, name),
		sf_field(sv2::Player, cards),
	};
	sf_struct_base(t, sv2::Player, sv2::Object, fields);
}

}
