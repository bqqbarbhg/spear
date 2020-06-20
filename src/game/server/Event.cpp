#include "Event.h"

#include "sf/Reflection.h"

namespace sf {

template<> void initType<sv::Event>(Type *t)
{
	static PolymorphType polys[] = {
		sf_poly(sv::Event, UpdateInstance, sv::EventUpdateInstance),
		sf_poly(sv::Event, UpdateObject, sv::EventUpdateObject),
		sf_poly(sv::Event, RemoveObject, sv::EventRemoveObject),
		sf_poly(sv::Event, UpdateInstance, sv::EventUpdateInstance),
		sf_poly(sv::Event, RemoveInstance, sv::EventRemoveInstance),
	};
	sf_struct_poly(t, sv::Event, type, { }, polys);
}

template<> void initType<sv::MoveType>(Type *t)
{
	static EnumValue values[] = {
		sf_enum(sv::MoveType, Walk),
		sf_enum(sv::MoveType, Run),
		sf_enum(sv::MoveType, Teleport),
	};
	sf_enum_type(t, sv::MoveType, values);
}

template<> void initType<sv::Waypoint>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::Waypoint, moveType),
		sf_field(sv::Waypoint, position),
	};
	sf_struct(t, sv::Waypoint, fields);
}

template<> void initType<sv::EventUpdateObject>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::EventUpdateObject, id),
		sf_field(sv::EventUpdateObject, object),
	};
	sf_struct_base(t, sv::EventUpdateObject, sv::Event, fields);
}
template<> void initType<sv::EventRemoveObject>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::EventRemoveObject, id),
	};
	sf_struct_base(t, sv::EventRemoveObject, sv::Event, fields);
}

template<> void initType<sv::EventUpdateInstance>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::EventUpdateInstance, id),
		sf_field(sv::EventUpdateInstance, instance),
	};
	sf_struct_base(t, sv::EventUpdateInstance, sv::Event, fields);
}
template<> void initType<sv::EventRemoveInstance>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::EventRemoveInstance, id),
	};
	sf_struct_base(t, sv::EventRemoveInstance, sv::Event, fields);
}

}
