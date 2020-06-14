#include "Object.h"

#include "sf/Reflection.h"
#include "server/Card.h"
#include "server/Player.h"

namespace sf {

template<> void initType<sv2::Object>(Type *t)
{
	static PolymorphType polys[] = {
		sf_poly(sv2::Object, CardType, sv2::CardType),
		sf_poly(sv2::Object, Player, sv2::Player),
	};
	static Field fields[] = {
		sf_field(sv2::Object, objectId),
		sf_field(sv2::Object, objectRevision),
	};
	sf_struct_poly(t, sv2::Object, objectType, fields, polys);
}


}
