#include "GameSystem.h"

#include "server/Pathfinding.h"

#include "game/DebugDraw.h"

namespace cl {

struct GameSystemImp final : GameSystem
{
	uint32_t selectedCharacterId = 102;

	void update(const sv::ServerState &svState, const FrameArgs &frameArgs) override
	{
		{
			const sv::Character *chr = svState.characters.find(selectedCharacterId);

			if (chr) {
				sv::PathfindOpts opts;
				opts.isBlockedFn = &sv::isBlockedByPropOrCharacter;
				opts.maxDistance = 10;
				sv::ReachableSet reachable = sv::findReachableSet(svState, opts, chr->tile);

				for (const auto &pair : reachable.distanceToTile) {
					sf::Bounds3 bounds;
					bounds.origin = sf::Vec3((float)pair.key.x, 0.0f, (float)pair.key.y);
					bounds.extent = sf::Vec3(0.95f, 0.2f, 0.95f) / 2.0f;
					debugDrawBox(bounds, sf::Vec3(1.0f, 1.0f, 0.0f));
				}
			}
		}
	}

	void updateTransform(Systems &systems, const EntityComponent &ec, const TransformUpdate &update) override
	{
	}

	void remove(Systems &systems, const EntityComponent &ec) override
	{
	}
};

sf::Box<GameSystem> GameSystem::create() { return sf::box<GameSystemImp>(); }

}
