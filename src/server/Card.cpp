#include "Card.h"

#include "sf/Reflection.h"

namespace sf {

template<> void initType<sv2::CardType>(Type *t)
{
	static Field fields[] = {
		sf_field(sv2::CardType, name),
		sf_field(sv2::CardType, description),
	};
	sf_struct_base(t, sv2::CardType, sv2::Object, fields);
}

template<> void initType<sv2::Card>(Type *t)
{
	static Field fields[] = {
		sf_field(sv2::Card, cardType),
		sf_field(sv2::Card, cooldownTurns),
	};
	sf_struct(t, sv2::Card, fields);
}

}
