#include "game/server/GameState.h"
#include "game/client/ClientState.h"

struct ClientEditor
{
	sv::ObjectId selectedObjectId;
	sv::InstanceId selectedInstanceId;

	uint32_t dragId;
};
