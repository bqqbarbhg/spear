#include "Event.h"

#include "sf/Reflection.h"

namespace sf {

template<> void initType<sv::Event>(Type *t)
{
	static PolymorphType polys[] = {
		sf_poly(sv::Event, CreateObject, sv::EventCreateObject),
		sf_poly(sv::Event, UpdateObject, sv::EventUpdateObject),
		sf_poly(sv::Event, CreateInstance, sv::EventCreateInstance),
		sf_poly(sv::Event, UpdateInstance, sv::EventUpdateInstance),
		sf_poly(sv::Event, RemoveInstance, sv::EventRemoveInstance),
	};
	sf_struct_poly(t, sv::Event, type, { }, polys);
}

#if 0
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

template<> void initType<sv::EventMove>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::EventMove, entity),
		sf_field(sv::EventMove, position),
		sf_field(sv::EventMove, waypoints),
	};
	sf_struct_base(t, sv::EventMove, sv::Event, fields);
}
#endif

template<> void initType<sv::EventCreateObject>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::EventCreateObject, id),
		sf_field(sv::EventCreateObject, object),
	};
	sf_struct_base(t, sv::EventCreateObject, sv::Event, fields);
}

template<> void initType<sv::EventUpdateObject>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::EventUpdateObject, id),
		sf_field(sv::EventUpdateObject, object),
	};
	sf_struct_base(t, sv::EventUpdateObject, sv::Event, fields);
}

template<> void initType<sv::EventCreateInstance>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::EventCreateInstance, id),
		sf_field(sv::EventCreateInstance, instance),
	};
	sf_struct_base(t, sv::EventCreateInstance, sv::Event, fields);
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
