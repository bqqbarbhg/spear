#pragma once

#include "game/server/GameState.h"

namespace sv {

struct Command
{
	enum Type
	{
		Error,
		Undo,
		Redo,
		AddObject,
		UpdateObject,
		RemoveObject,
		UpdateObjectType,
		LoadObjectType,
		LoadRoom,

		Type_Count,
		Type_ForceU32 = 0x7fffffff,
	};

	Type type;

	template <typename T> T *as() { return type == T::CommandType ? (T*)this : nullptr; }
	template <typename T> const T *as() const { return type == T::CommandType ? (T*)this : nullptr; }

	Command() : type(Error) { }
	Command(Type type) : type(type) { }
};

template <Command::Type SelfType>
struct CommandBase : Command
{
	static constexpr Type CommandType = SelfType;
	CommandBase() : Command(SelfType) { }
};

struct CommandUndo : CommandBase<Command::Undo>
{
};

struct CommandRedo : CommandBase<Command::Redo>
{
};

struct CommandAddObject : CommandBase<Command::AddObject>
{
	sf::Symbol typePath;
	sv::Object object;
};

struct CommandUpdateObject : CommandBase<Command::UpdateObject>
{
	uint32_t id;
	sv::Object object;
};

struct CommandRemoveObject : CommandBase<Command::RemoveObject>
{
	uint32_t id;
};

struct CommandUpdateObjectType : CommandBase<Command::UpdateObjectType>
{
	sf::Symbol typePath;
	sv::GameObject objectType;
};

struct CommandLoadObjectType : CommandBase<Command::LoadObjectType>
{
	sf::Symbol typePath;
};

}
