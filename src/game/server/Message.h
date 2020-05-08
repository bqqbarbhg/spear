#pragma once

#include "Action.h"
#include "Event.h"
#include "sf/Box.h"
#include "sf/Array.h"
#include "sf/String.h"

namespace sv {

struct Message
{
	virtual ~Message() { }

	enum Type {
		Error,
		Action,
		ActionSuccess,
		ActionFailure,
		Update,

		Type_Count,
		Type_ForceU32 = 0x7fffffff,
	};

	Type type;

	static uint32_t serialCounter;
	uint32_t serial;

	Message() { }
	Message(Type type) : type(type), serial(++serialCounter) { }
};

struct MessageAction : Message
{
	MessageAction() : Message(Action) { }

	uint32_t test;
	// sf::RcBox<sv::Action> action;
};

struct MessageActionSuccess : Message
{
	MessageActionSuccess() : Message(ActionSuccess) { }

	bool testSuccessFlag;
};

struct MessageActionFailure : Message
{
	MessageActionFailure() : Message(ActionFailure) { }

	sf::StringBuf testDescription;
};

struct MessageUpdate : Message
{
	MessageUpdate() : Message(Update) { }

	sf::Array<sf::RcBox<Message>> testMessages;
};

}
