#include "TileState.h"

#include "server/Event.h"
#include "server/Components.h"

namespace sv {

static void addTileEntityImp(TileState &ts, const sv::Entity &entity)
{
}

static void removeTileEntityImp(TileState &ts, const sv::EntityId &entity)
{
}

struct TileRange
{
	sf::Vec2i min;
	sf::Vec2i max;
};

static TileRange getPointBounds(const sf::Vec2i &offset, const sf::Vec2i &size, uint8_t rotation)
{
	int32_t ox = offset.x, oy = offset.y;
	int32_t sx = size.x - 1, sy = size.y - 1;

	rotation = (uint8_t)((unsigned)rotation + 256/8);
	if (rotation < 256/4*1) {
		return { { ox, oy }, { ox + sx, oy + sy } };
	} else if (rotation < 256/4*2) {
		return { { oy, -ox - sx }, { oy + sy, -ox } };
	} else if (rotation < 256/4*3) {
		return { { -ox - sx, -oy - sy }, { -ox, -oy } };
	} else {
		return { { -oy - sy, ox }, { -oy, ox + sx } };
	}
}

static sf::Vec2i getChunkFromTile(const sf::Vec2i &tile)
{
	return { tile.x >> 4, tile.y >> 4 };
}

void TileState::update(const State *state, const DirtyList &dirty)
{
	for (auto &pair : dirty.entities)
	{
		uint32_t flags = pair.val;
		if ((flags & (D_Position|D_TileChunk)) == 0)  continue;
		EntityId eid = pair.key;

		const Entity *entity = state->findEntity(eid);
		if (entity) {
			const Proto *proto = state->findProto(entity->protoId);
			EntityChunks &ec = entityChunks[eid];

			sf::SmallArray<sf::Vec2i, 8> newChunks;
			sf::Vec2i pos = { entity->x, entity->y };

			for (const Component *comp : proto->components) {
				if (const TileAreaComponent *c = comp->as<TileAreaComponent>()) {
					TileRange range = getPointBounds(c->offset, c->size, entity->rotation);
					sf::Vec2i chunkMin = getChunkFromTile(pos + range.min);
					sf::Vec2i chunkMax = getChunkFromTile(pos + range.max);
					for (int32_t y = chunkMin.y; y <= chunkMax.y; y++)
					for (int32_t x = chunkMin.x; x <= chunkMax.x; x++)
					{
						sf::Vec2i chunk = { x, y };
						if (!sf::find(newChunks, chunk)) {
							newChunks.push(chunk);
						}
					}
				}
			}

			for (uint32_t i = 0; i < ec.chunks.size; i++) {
				const sf::Vec2i &oldChunk = ec.chunks[i];
				if (!sf::findRemoveSwap(newChunks, oldChunk)) {
					TileChunk &tileChunk = tileChunks[oldChunk];
					tileChunk.entities.remove(eid);
					ec.chunks.removeSwap(i--);
				}
			}

			for (const sf::Vec2i &newChunk : newChunks) {
				TileChunk &tileChunk = tileChunks[newChunk];
				tileChunk.entities.insert(eid);
				ec.chunks.push(newChunk);
			}

		} else {
			EntityChunks &ec = entityChunks[eid];
			for (const sf::Vec2i &oldChunk : ec.chunks) {
				TileChunk &tileChunk = tileChunks[oldChunk];
				tileChunk.entities.remove(eid);
			}
			entityChunks.remove(eid);
		}

	}
}

}
