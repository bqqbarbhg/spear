#include "VisFogSystem.h"

#include "ext/sokol/sokol_gfx.h"
#include "sp/Renderer.h"

#include "server/Pathfinding.h"

namespace cl {

static const sf::Vec2i visFogNeighbors[] = {
	{ 0, 0 }, { +1, 0 }, { -1, 0 }, { 0, +1 }, { 0, -1 },
	{ +1, -1 }, { -1, -1 }, { +1, +1 }, { -1, +1 },  
};

struct VisFogSystemImp final : VisFogSystem
{
	struct DirtyTile
	{
		uint8_t val[2];
	};

	sf::Vec2i visFogOffset = sf::Vec2i(-128, -128);
	sf::Vec2i visFogResolution = sf::Vec2i(256, 256);
	sp::Texture visFogTexture;
	sf::Array<uint8_t> visFog;
	sv::ReachableSet reachableSet;
	sf::Vec4 worldMad;
	sf::HashMap<uint32_t, DirtyTile> dirtyTiles;
	float deltaFade = 0.0f;

	void addVisibleTile(const sf::Vec2i &tile, uint32_t val1)
	{
		sf::Vec2i relBase = tile - visFogOffset;
		for (const sf::Vec2i &nb : sf::slice(visFogNeighbors)) {
			sf::Vec2i rel = relBase + nb;
			if (rel.x < 0 || rel.y < 0 || rel.x >= visFogResolution.x || rel.y >= visFogResolution.y) continue;

			uint32_t ix = (rel.y * visFogResolution.x + rel.x) * 2;
			uint32_t val0 = &nb == visFogNeighbors ? 255 : 128;

			if (val0 > visFog[ix + 0] || val1 > visFog[ix + 1]) {
				DirtyTile &dirty = dirtyTiles[ix];
				dirty.val[0] = sf::max(dirty.val[0], (uint8_t)val0, visFog[ix + 0]);
				dirty.val[1] = sf::max(dirty.val[1], (uint8_t)val1, visFog[ix + 1]);
			}
		}
	}

	// -- API

	VisFogSystemImp()
	{
		visFog.resize(visFogResolution.x * visFogResolution.y * 2);

		worldMad.x = 1.0f / (float)visFogResolution.x;
		worldMad.y = 1.0f / (float)visFogResolution.y;
		worldMad.z = (0.5f + (float)-visFogOffset.x) / (float)visFogResolution.x;
		worldMad.w = (0.5f + (float)-visFogOffset.y) / (float)visFogResolution.y;

		{
			sg_image_desc d = { };
			d.label = "visFogTexture";
			d.pixel_format = SG_PIXELFORMAT_RG8;
			d.width = visFogResolution.x;
			d.height = visFogResolution.y;
			d.usage = SG_USAGE_DYNAMIC;
			d.min_filter = SG_FILTER_LINEAR;
			d.mag_filter = SG_FILTER_LINEAR;
			visFogTexture.init(d);
		}
	}

	void updateVisibility(const sv::VisibleUpdateEvent &e) override
	{
		for (const sv::VisibleTile &visTile : e.visibleTiles) {
			int32_t val = sf::clamp(((int32_t)visTile.amount - 1) * 60, 0, 255);
			sf::Vec2i tile = sv::unpackTile(visTile.packedTile);
			addVisibleTile(tile, (uint32_t)val);
		}
	}

	void updateTexture(float dt) override
	{
		if (dirtyTiles.size() == 0) return;

		const float rcpFadeUnitSpeed = 255.0f / 3.0f;
		deltaFade += dt * rcpFadeUnitSpeed;

		if (deltaFade < 1.0f) return;
		uint32_t fadeNum = (uint32_t)deltaFade;
		deltaFade -= (float)fadeNum;

		for (uint32_t i = 0; i < dirtyTiles.size(); i++) {
			auto &pair = dirtyTiles.data[i];
			uint8_t *dst = visFog.data + pair.key;

			dst[0] = pair.val.val[0];

			if ((uint32_t)(pair.val.val[1] - dst[1]) <= fadeNum) {
				dst[1] = pair.val.val[1];
				dirtyTiles.remove(pair.key);
				i--;
			} else {
				dst[1] += fadeNum;
			}
		}

		// TODO: Dirty rectangle
		sg_image_content content = { };
		content.subimage[0][0].ptr = visFog.data;
		content.subimage[0][0].size = visFog.size;
		sg_update_image(visFogTexture.image, &content);
	}

	VisFogImage getVisFogImage() const override
	{
		VisFogImage image;
		image.image = visFogTexture.image;
		image.worldMad = worldMad;
		return image;
	}
};

sf::Box<VisFogSystem> VisFogSystem::create() { return sf::box<VisFogSystemImp>(); }

}
