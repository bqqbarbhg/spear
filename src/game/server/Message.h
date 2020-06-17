#pragma once

#include "Action.h"
#include "Command.h"
#include "Event.h"
#include "GameState.h"
#include "sf/Box.h"
#include "sf/Array.h"
#include "sf/String.h"

namespace sv {

struct QueryFile
{
	sf::StringBuf name;
};

struct QueryDir
{
	sf::StringBuf name;
	sf::Array<QueryDir> dirs;
	sf::Array<QueryFile> files;
};

struct Message
{
	enum Type {
		Error,
		Join,
		Action,
		Command,
		MultiCommand,
		ActionSuccess,
		ActionFailure,
		Update,
		Load,
		QueryFiles,
		QueryFilesResult,

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
	uint32_t playerId;
	sf::Symbol name;
	sf::Symbol editRoomPath;
};

struct MessageAction : MessageBase<Message::Action>
{
	sf::Box<sv::Action> action;
};

struct MessageCommand : MessageBase<Message::Command>
{
	sf::Box<sv::Command> command;
};

struct MessageMultiCommand : MessageBase<Message::MultiCommand>
{
	sf::Array<sf::Box<sv::Command>> commands;
};

struct MessageActionSuccess : MessageBase<Message::ActionSuccess>
{
};

struct MessageActionFailure : MessageBase<Message::ActionFailure>
{
	sf::StringBuf description;
};

struct MessageUpdate : MessageBase<Message::Update>
{
	sf::Array<sf::Box<Event>> events;
};

struct MessageLoad : MessageBase<Message::Load>
{
	sf::Box<sv::State> state;
	uint32_t sessionId;
	uint32_t sessionSecret;
	sf::Symbol editRoomPath;
};

struct MessageQueryFiles : MessageBase<Message::QueryFiles>
{
	sf::StringBuf root;
};

struct MessageQueryFilesResult : MessageBase<Message::QueryFilesResult>
{
	sf::StringBuf root;
	QueryDir dir;
};

}
