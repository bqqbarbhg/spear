#include "Message.h"
#include "sf/Reflection.h"

namespace sv {

uint32_t Message::serialCounter = 0;

}

namespace sf {

template<> void initType<sv::Message>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::Message, serial)
	};
	static PolymorphType polys[] = {
		sf_poly(sv::Message, Action, sv::MessageAction),
		sf_poly(sv::Message, ActionSuccess, sv::MessageActionSuccess),
		sf_poly(sv::Message, ActionFailure, sv::MessageActionFailure),
		sf_poly(sv::Message, Update, sv::MessageUpdate),
	};
	sf_struct_poly(t, sv::Message, type, fields, polys);
}

template<> void initType<sv::MessageAction>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageAction, test)
	};
	sf_struct(t, sv::MessageAction, fields);
}

template<> void initType<sv::MessageActionSuccess>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageActionSuccess, testSuccessFlag)
	};
	sf_struct(t, sv::MessageActionSuccess, fields);
}

template<> void initType<sv::MessageActionFailure>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageActionFailure, testDescription)
	};
	sf_struct(t, sv::MessageActionFailure, fields);
}

template<> void initType<sv::MessageUpdate>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageUpdate, testMessages)
	};
	sf_struct(t, sv::MessageUpdate, fields);
}

}
