#pragma once

#include "Action.h"
#include "Event.h"
#include "sf/Box.h"
#include "sf/Array.h"
#include "sf/String.h"

namespace sv {

struct Message
{
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

template <Message::Type SelfType>
struct MessageBase : Message
{
	static constexpr Type MessageType = SelfType;
	MessageBase() : Message(MessageType) { }
};

struct MessageAction : MessageBase<Message::Action>
{

	uint32_t test;
	// sf::Box<sv::Action> action;
};

struct MessageActionSuccess : MessageBase<Message::ActionSuccess>
{
	bool testSuccessFlag;
};

struct MessageActionFailure : MessageBase<Message::ActionFailure>
{
	sf::StringBuf testDescription;
};

struct MessageUpdate : MessageBase<Message::Update>
{
	sf::Array<sf::Box<Message>> testMessages;
};

}
