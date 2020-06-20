#include "GameComponent.h"

#include "sf/Reflection.h"

namespace sf {

template<> void initType<sv::Card>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::Card, type),
	};
	sf_struct(t, sv::Card, fields);
}

template<> void initType<sv::Component>(Type *t)
{
	static PolymorphType polys[] = {
		sf_poly(sv::Component, Model, sv::ModelComponent),
		sf_poly(sv::Component, PointLight, sv::PointLightComponent),
		sf_poly(sv::Component, Character, sv::CharacterComponent),
		sf_poly(sv::Component, Card, sv::CardComponent),
	};
	sf_struct_poly(t, sv::Component, type, { }, polys);
}

template<> void initType<sv::ModelComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::ModelComponent, model),
		sf_field(sv::ModelComponent, shadowModel),
		sf_field(sv::ModelComponent, material),
		sf_field(sv::ModelComponent, position),
		sf_field(sv::ModelComponent, rotation),
		sf_field(sv::ModelComponent, scale),
		sf_field(sv::ModelComponent, stretch),
		sf_field(sv::ModelComponent, castShadows),
	};
	sf_struct_base(t, sv::ModelComponent, sv::Component, fields);
}

template<> void initType<sv::PointLightComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::PointLightComponent, color),
		sf_field(sv::PointLightComponent, intensity),
		sf_field(sv::PointLightComponent, radius),
		sf_field(sv::PointLightComponent, position),
	};
	sf_struct_base(t, sv::PointLightComponent, sv::Component, fields);
}

template<> void initType<sv::CharacterComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::CharacterComponent, model),
		sf_field(sv::CharacterComponent, players),
		sf_field(sv::CharacterComponent, cards),
	};
	sf_struct_base(t, sv::CharacterComponent, sv::Component, fields);
}

template<> void initType<sv::CardComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::CardComponent, image),
		sf_field(sv::CardComponent, name),
		sf_field(sv::CardComponent, description),
	};
	sf_struct_base(t, sv::CardComponent, sv::Component, fields);
}

template<> void initType<sv::GameObject>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::GameObject, id),
		sf_field(sv::GameObject, name),
		sf_field(sv::GameObject, components),
	};
	sf_struct(t, sv::GameObject, fields);
}

}
