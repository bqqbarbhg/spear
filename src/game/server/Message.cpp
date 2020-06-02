#include "Message.h"
#include "sf/Reflection.h"

namespace sf {

template<> void initType<sv::Message>(Type *t)
{
	static PolymorphType polys[] = {
		sf_poly(sv::Message, Join, sv::MessageJoin),
		sf_poly(sv::Message, Action, sv::MessageAction),
		sf_poly(sv::Message, Command, sv::MessageCommand),
		sf_poly(sv::Message, ActionSuccess, sv::MessageActionSuccess),
		sf_poly(sv::Message, ActionFailure, sv::MessageActionFailure),
		sf_poly(sv::Message, Update, sv::MessageUpdate),
		sf_poly(sv::Message, Load, sv::MessageLoad),
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
	};
	sf_struct_base(t, sv::MessageLoad, sv::Message, fields);
}

}
