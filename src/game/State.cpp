#include "State.h"

#include "sf/Reflection.h"

namespace sf {

template<> void initType<Character>(sf::Type *t)
{
	static sf::Field fields[] = {
		sf_field(Character, name),
		sf_field(Character, position),
	};
	sf_struct(t, Character, fields);
}

}
