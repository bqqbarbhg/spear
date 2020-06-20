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
		AddInstance,
		UpdateInstance,
		RemoveInstance,
		UpdateObject,
		LoadObject,
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

struct CommandAddInstance : CommandBase<Command::AddInstance>
{
	sf::Symbol typePath;
	sv::InstancedObject instance;
};

struct CommandUpdateInstance : CommandBase<Command::UpdateInstance>
{
	InstanceId id;
	sv::InstancedObject instance;
};

struct CommandRemoveInstance : CommandBase<Command::RemoveInstance>
{
	InstanceId id;
};

struct CommandUpdateObject : CommandBase<Command::UpdateObject>
{
	sf::Symbol typePath;
	sv::GameObject object;
};

struct CommandLoadObject : CommandBase<Command::LoadObject>
{
	sf::Symbol typePath;
};

}
