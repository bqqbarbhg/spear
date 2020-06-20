#include "Command.h"
#include "sf/Reflection.h"

namespace sf {

template<> void initType<sv::Command>(Type *t)
{
	static PolymorphType polys[] = {
		sf_poly(sv::Command, Undo, sv::CommandUndo),
		sf_poly(sv::Command, Redo, sv::CommandRedo),
		sf_poly(sv::Command, AddInstance, sv::CommandAddInstance),
		sf_poly(sv::Command, UpdateInstance, sv::CommandUpdateInstance),
		sf_poly(sv::Command, RemoveInstance, sv::CommandRemoveInstance),
		sf_poly(sv::Command, UpdateObject, sv::CommandUpdateObject),
		sf_poly(sv::Command, LoadObject, sv::CommandLoadObject),
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

template<> void initType<sv::CommandAddInstance>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::CommandAddInstance, typePath),
		sf_field(sv::CommandAddInstance, instance),
	};
	sf_struct_base(t, sv::CommandAddInstance, sv::Command, fields);
}

template<> void initType<sv::CommandUpdateInstance>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::CommandUpdateInstance, id),
		sf_field(sv::CommandUpdateInstance, instance),
	};
	sf_struct_base(t, sv::CommandUpdateInstance, sv::Command, fields);
}

template<> void initType<sv::CommandRemoveInstance>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::CommandRemoveInstance, id),
	};
	sf_struct_base(t, sv::CommandRemoveInstance, sv::Command, fields);
}

template<> void initType<sv::CommandUpdateObject>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::CommandUpdateObject, typePath),
		sf_field(sv::CommandUpdateObject, object),
	};
	sf_struct_base(t, sv::CommandUpdateObject, sv::Command, fields);
}

template<> void initType<sv::CommandLoadObject>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::CommandLoadObject, typePath),
	};
	sf_struct_base(t, sv::CommandLoadObject, sv::Command, fields);
}
}
