#pragma once

#include "sf/Array.h"
#include "sf/Box.h"
#include "sf/HashMap.h"
#include "server/HashTable.h"

namespace sv {

struct StatusEffect
{
	enum {
		Id,
		EntityId,
		NumColumns,
	};

	uint32_t id;
	uint32_t entityId;

};

struct ServerState
{
	sv::HashTable<StatusEffect> statusEffects;
};

}
