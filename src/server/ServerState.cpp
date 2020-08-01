#include "ServerState.h"

#include "sf/Reflection.h"

namespace sf {

template<> void initType<sv::StatusEffect>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::StatusEffect, id),
		sf_field(sv::StatusEffect, entityId),
	};
	sf_struct(t, sv::StatusEffect, fields);
}

}

