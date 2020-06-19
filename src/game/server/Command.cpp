#include "Command.h"
#include "sf/Reflection.h"

namespace sf {

template<> void initType<sv::Command>(Type *t)
{
	static PolymorphType polys[] = {
		sf_poly(sv::Command, Undo, sv::CommandUndo),
		sf_poly(sv::Command, Redo, sv::CommandRedo),
		sf_poly(sv::Command, AddObject, sv::CommandAddObject),
		sf_poly(sv::Command, UpdateObject, sv::CommandUpdateObject),
		sf_poly(sv::Command, RemoveObject, sv::CommandRemoveObject),
		sf_poly(sv::Command, UpdateObjectType, sv::CommandUpdateObjectType),
		sf_poly(sv::Command, LoadObjectType, sv::CommandLoadObjectType),
	};
	sf_struct_poly(t, sv::Command, type, { }, polys);
}

template<> void initType<sv::CommandUndo>(Type *t)
{
	sf_struct_base(t, sv::CommandUndo, sv::Command, { });
}

template<> void initType<sv::CommandRedo>(Type *t)
{
	sf_struct_base(t, sv::CommandRedo, sv::Command, { });
}

template<> void initType<sv::CommandAddObject>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::CommandAddObject, typePath),
		sf_field(sv::CommandAddObject, object),
	};
	sf_struct_base(t, sv::CommandAddObject, sv::Command, fields);
}

template<> void initType<sv::CommandUpdateObject>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::CommandUpdateObject, id),
		sf_field(sv::CommandUpdateObject, object),
	};
	sf_struct_base(t, sv::CommandUpdateObject, sv::Command, fields);
}

template<> void initType<sv::CommandRemoveObject>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::CommandRemoveObject, id),
	};
	sf_struct_base(t, sv::CommandRemoveObject, sv::Command, fields);
}

template<> void initType<sv::CommandUpdateObjectType>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::CommandUpdateObjectType, typePath),
		sf_field(sv::CommandUpdateObjectType, objectType),
	};
	sf_struct_base(t, sv::CommandUpdateObjectType, sv::Command, fields);
}

template<> void initType<sv::CommandLoadObjectType>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::CommandLoadObjectType, typePath),
	};
	sf_struct_base(t, sv::CommandLoadObjectType, sv::Command, fields);
}
}
