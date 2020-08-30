#pragma once

#include "ServerState.h"

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
	#if SF_DEBUG
		virtual void debugForceVtable() { }
	#endif

	enum Type {
		Error,
		Join,
		Load,
		Update,
		RequestEdit,
		RequestEditUndo,
		RequestEditRedo,
		RequestReplayBegin,
		RequestReplayReplay,
		RequestAction,
		QueryFiles,
		QueryFilesResult,
		ErrorList,

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
	uint32_t sessionId = 0, sessionSecret = 0;
	uint32_t playerId = 0;
	sf::Symbol name;
	sf::Symbol editPath;
};

struct MessageLoad : MessageBase<Message::Load>
{
	uint32_t sessionId, sessionSecret;
	sf::Box<ServerState> state;
	uint32_t clientId;
	sf::Symbol editPath;
};

struct MessageUpdate : MessageBase<Message::Update>
{
	sf::Array<sf::Box<Event>> events;
};

struct MessageRequestEdit : MessageBase<Message::RequestEdit>
{
	sf::Array<sf::Box<Edit>> edits;
};

struct MessageRequestEditUndo : MessageBase<Message::RequestEditUndo>
{
};

struct MessageRequestEditRedo : MessageBase<Message::RequestEditRedo>
{
};

struct MessageRequestReplayBegin : MessageBase<Message::RequestReplayBegin>
{
};

struct MessageRequestReplayReplay : MessageBase<Message::RequestReplayReplay>
{
};

struct MessageRequestAction : MessageBase<Message::RequestAction>
{
	sf::Box<sv::Action> action;
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

struct MessageErrorList : MessageBase<Message::ErrorList>
{
	sf::Array<sf::StringBuf> errors;
};

struct MessageEncoding
{
	bool binary = false;
	int compressionLevel = 0;
};

sf::Box<Message> decodeMessage(sf::Slice<char> data);
void encodeMessage(sf::Array<char> &data, const Message &message, const MessageEncoding &encoding);

}
