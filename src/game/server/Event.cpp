#include "Event.h"

#include "sf/Reflection.h"

namespace sf {

template<> void initType<sv::Event>(Type *t)
{
	static PolymorphType polys[] = {
		sf_poly(sv::Event, Move, sv::EventMove),
		sf_poly(sv::Event, Spawn, sv::EventSpawn),
		sf_poly(sv::Event, Destroy, sv::EventDestroy),
		sf_poly(sv::Event, UpdateObject, sv::EventUpdateObject),
		sf_poly(sv::Event, UpdateObjectType, sv::EventUpdateObjectType),
		sf_poly(sv::Event, RemoveObject, sv::EventRemoveObject),
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

template<> void initType<sv::EventMove>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::EventMove, entity),
		sf_field(sv::EventMove, position),
		sf_field(sv::EventMove, waypoints),
	};
	sf_struct_base(t, sv::EventMove, sv::Event, fields);
}

template<> void initType<sv::EventSpawn>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::EventSpawn, data),
	};
	sf_struct_base(t, sv::EventSpawn, sv::Event, fields);
}

template<> void initType<sv::EventDestroy>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::EventDestroy, entity),
	};
	sf_struct_base(t, sv::EventDestroy, sv::Event, fields);
}

template<> void initType<sv::EventUpdateObject>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::EventUpdateObject, id),
		sf_field(sv::EventUpdateObject, object),
	};
	sf_struct_base(t, sv::EventUpdateObject, sv::Event, fields);
}

template<> void initType<sv::EventUpdateObjectType>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::EventUpdateObjectType, index),
		sf_field(sv::EventUpdateObjectType, object),
	};
	sf_struct_base(t, sv::EventUpdateObjectType, sv::Event, fields);
}

template<> void initType<sv::EventRemoveObject>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::EventRemoveObject, id),
	};
	sf_struct_base(t, sv::EventRemoveObject, sv::Event, fields);
}

}
