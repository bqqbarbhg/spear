#include "Action.h"
#include "sf/Reflection.h"

namespace sf {

template<> void initType<sv::Action>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::Action, object),
	};
	static PolymorphType polys[] = {
		sf_poly(sv::Action, Move, sv::ActionMove),
	};
	sf_struct_poly(t, sv::Action, type, fields, polys);
}

template<> void initType<sv::ActionMove>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::ActionMove, position),
	};
	sf_struct_base(t, sv::ActionMove, sv::Action, fields);
}

}
