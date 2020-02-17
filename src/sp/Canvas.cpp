#include "Canvas.h"

#include "Sprite.h"
#include "sf/Array.h"

namespace sp {

struct SpriteDraw
{
	Sprite *sprite;
	sf::Mat23 transform;
};

struct CanvasImp
{
	sf::Array<SpriteDraw> spriteDraws;

	void render();
};

static void filterAtlases(sf::Array<Atlas*> &atlases, sf::Slice<SpriteResidency> residency)
{
	Atlas **dst = atlases.data;
	SpriteResidency *res = residency.begin(), *resEnd = residency.end();
	for (Atlas *atlas : atlases) {

		// Skip residency until we find `atlas` (or skip past it)
		while (res < resEnd && res->atlas < atlas) {
			res++;
		}

		if (res != resEnd && res->atlas == atlas) {
			// Found the atlas in residency, add to `dst`
			// Note that the write is always before the pointer to `atlas`
			// and cannot be seen by the iteration.
			*dst++ = atlas;
		}
	}

	// Remove the atlases past `dst`
	atlases.resize(dst - atlases.data);
}

static void drawSprites(sf::Slice<SpriteDraw> draws, Atlas *atlas)
{
}

void CanvasImp::render()
{
	sf::Slice<SpriteDraw> draws = spriteDraws;

	sf::SmallArray<Atlas*, 16> atlases;
	sf::SmallArray<SpriteResidency, 16> residency;

	uint32_t begin = 0;
	while (begin < draws.size) {
		atlases.clear();

		draws[begin].sprite->getResidency(residency);
		if (residency.size == 0) {
			begin++;
			continue;
		}

		// Initialize with the list of atlases the
		// first sprite can use
		for (SpriteResidency &r : residency) {
			atlases.push(r.atlas);
		}
		Atlas *atlas = atlases[0];

		// Keep appending sprites that share some atlases
		uint32_t end = begin + 1;
		for (; end < draws.size; end++) {
			draws[end].sprite->getResidency(residency);
			if (residency.size == 0) continue;

			filterAtlases(atlases, residency);
			if (atlases.size == 0) break;
			atlas = atlases[0];
		}

		drawSprites(sf::slice(draws.data + begin, end - begin), atlas);

		begin = end;
	}
}

Canvas::Canvas()
	: imp(new CanvasImp())
{
}

Canvas::~Canvas()
{
	delete imp;
}

void Canvas::clear()
{
	imp->spriteDraws.clear();
}

void Canvas::draw(Sprite *s, const sf::Mat23 &transform)
{
	if (!s || !s->shouldBeLoaded()) return;
	s->willBeRendered();

	SpriteDraw &draw = imp->spriteDraws.push();
	draw.sprite = s;
	draw.transform = transform;
}

void Canvas::drawText(Font *f, sf::String str, const sf::Vec2 &pos)
{
}

}
