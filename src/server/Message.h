#pragma once

#include "ServerState.h"

namespace sv {

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
};

struct MessageLoad : MessageBase<Message::Load>
{
	uint32_t sessionId, sessionSecret;
	sf::Box<ServerState> state;
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

struct MessageEncoding
{
	bool binary = false;
	int compressionLevel = 0;
};

sf::Box<Message> decodeMessage(sf::Slice<char> data);
void encodeMessage(sf::Array<char> &data, const Message &message, const MessageEncoding &encoding);

}
