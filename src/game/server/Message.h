#pragma once

#include "Action.h"
#include "Event.h"
#include "GameState.h"
#include "sf/Box.h"
#include "sf/Array.h"
#include "sf/String.h"

namespace sv {

struct Message
{
	enum Type {
		Error,
		Join,
		Action,
		ActionSuccess,
		ActionFailure,
		Update,
		Load,

		Type_Count,
		Type_ForceU32 = 0x7fffffff,
	};

	Type type;

	Message() { }
	Message(Type type) : type(type) { }

	template <typename T> T *as() { return type == T::MessageType ? (T*)this : nullptr; }
	template <typename T> const T *as() const { return type == T::MessageType ? (T*)this : nullptr; }
};

template <Message::Type SelfType>
struct MessageBase : Message
{
	static constexpr Type MessageType = SelfType;
	MessageBase() : Message(SelfType) { }
};

struct MessageJoin : MessageBase<Message::Join>
{
	uint32_t sessionId, sessionSecret;
	sf::Symbol name;
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
	sf::Array<sf::Box<Event>> events;
};

struct MessageLoad : MessageBase<Message::Load>
{
	sf::Box<sv::State> state;
};

}
