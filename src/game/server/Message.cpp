#include "Message.h"
#include "sf/Reflection.h"

namespace sf {

template<> void initType<sv::QueryFile>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::QueryFile, name),
	};
	sf_struct(t, sv::QueryFile, fields);
}

template<> void initType<sv::QueryDir>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::QueryDir, name),
		sf_field(sv::QueryDir, dirs),
		sf_field(sv::QueryDir, files),
	};
	sf_struct(t, sv::QueryDir, fields);
}

template<> void initType<sv::Message>(Type *t)
{
	static PolymorphType polys[] = {
		sf_poly(sv::Message, Join, sv::MessageJoin),
		sf_poly(sv::Message, Action, sv::MessageAction),
		sf_poly(sv::Message, Command, sv::MessageCommand),
		sf_poly(sv::Message, MultiCommand, sv::MessageMultiCommand),
		sf_poly(sv::Message, ActionSuccess, sv::MessageActionSuccess),
		sf_poly(sv::Message, ActionFailure, sv::MessageActionFailure),
		sf_poly(sv::Message, Update, sv::MessageUpdate),
		sf_poly(sv::Message, Load, sv::MessageLoad),
		sf_poly(sv::Message, QueryFiles, sv::MessageQueryFiles),
		sf_poly(sv::Message, QueryFilesResult, sv::MessageQueryFilesResult),
	};
	sf_struct_poly(t, sv::Message, type, { }, polys);
}

template<> void initType<sv::MessageJoin>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageJoin, sessionId),
		sf_field(sv::MessageJoin, sessionSecret),
		sf_field(sv::MessageJoin, playerId),
		sf_field(sv::MessageJoin, name),
		sf_field(sv::MessageJoin, editRoomPath),
	};
	sf_struct_base(t, sv::MessageJoin, sv::Message, fields);
}

template<> void initType<sv::MessageAction>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageAction, action),
	};
	sf_struct_base(t, sv::MessageAction, sv::Message, fields);
}

template<> void initType<sv::MessageCommand>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageCommand, command),
	};
	sf_struct_base(t, sv::MessageCommand, sv::Message, fields);
}

template<> void initType<sv::MessageMultiCommand>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageMultiCommand, commands),
	};
	sf_struct_base(t, sv::MessageMultiCommand, sv::Message, fields);
}

template<> void initType<sv::MessageActionSuccess>(Type *t)
{
	sf_struct_base(t, sv::MessageActionSuccess, sv::Message, { });
}

template<> void initType<sv::MessageActionFailure>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageActionFailure, description),
	};
	sf_struct_base(t, sv::MessageActionFailure, sv::Message, fields);
}

template<> void initType<sv::MessageUpdate>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageUpdate, events),
	};
	sf_struct_base(t, sv::MessageUpdate, sv::Message, fields);
}

template<> void initType<sv::MessageLoad>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageLoad, state),
		sf_field(sv::MessageLoad, sessionId),
		sf_field(sv::MessageLoad, sessionSecret),
		sf_field(sv::MessageLoad, editRoomPath),
	};
	sf_struct_base(t, sv::MessageLoad, sv::Message, fields);
}

template<> void initType<sv::MessageQueryFiles>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageQueryFiles, root),
	};
	sf_struct_base(t, sv::MessageQueryFiles, sv::Message, fields);
}

template<> void initType<sv::MessageQueryFilesResult>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageQueryFilesResult, root),
		sf_field(sv::MessageQueryFilesResult, dir),
	};
	sf_struct_base(t, sv::MessageQueryFilesResult, sv::Message, fields);
}

}
