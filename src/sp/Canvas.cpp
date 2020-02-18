#include "Canvas.h"

#include "Sprite.h"
#include "sf/Array.h"

#include "ext/sokol/sokol_gfx.h"
#include "shader/Sprite.h"

namespace sp {

static const uint32_t MaxQuadsPerDraw = 1024;
static const uint32_t MaxQuadsPerFrame = 16*1024;

struct SpriteDraw;

struct Quad
{
	struct Vertex
	{
		sf::Vec2 position;
		sf::Vec2 texCoord;
		uint32_t color;
	};

	Vertex v[4];
};

struct CanvasRenderer
{
	sg_shader spriteShader;
	sg_pipeline spritePipeline;
	sg_bindings spriteBindings = { };

	sg_buffer quadIndexBuffer;
	sg_buffer quadVertexBuffer;

	sf::Array<Quad> quads;

	CanvasRenderer();
	~CanvasRenderer();

	void drawSprites(sf::Slice<SpriteDraw> draws, Atlas *atlas);
};

CanvasRenderer &getCanvasRenderer()
{
	static CanvasRenderer renderer;
	return renderer;
}

struct SpriteDraw
{
	SpriteRef sprite;
	sf::Mat23 transform;
	sf::Vec4 color;
};

struct CanvasImp
{
	sf::Array<SpriteDraw> spriteDraws;

	void render(const CanvasRenderOpts &pots);
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

CanvasRenderer::CanvasRenderer()
{
	spriteShader = sg_make_shader(Sprite_Sprite_shader_desc());

	// Pipeline
	{
		sg_pipeline_desc desc = { };
		desc.shader = spriteShader;

		desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
		desc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;
		desc.layout.attrs[2].format = SG_VERTEXFORMAT_UBYTE4N;

		desc.index_type = SG_INDEXTYPE_UINT16;

		spritePipeline = sg_make_pipeline(&desc);
	}

	// Index buffer
	{
		sf::Array<uint16_t> indices;
		indices.resizeUninit(6 * MaxQuadsPerDraw);
		for (uint32_t i = 0; i < MaxQuadsPerDraw; i++) {
			uint16_t *d = indices.data + i * 6;
			uint32_t a = i * 4;
			d[0] = (uint16_t)(a+0); d[1] = (uint16_t)(a+2); d[2] = (uint16_t)(a+1);
			d[3] = (uint16_t)(a+1); d[4] = (uint16_t)(a+2); d[5] = (uint16_t)(a+3);
		}

		sg_buffer_desc desc = { };
		desc.type = SG_BUFFERTYPE_INDEXBUFFER;
		desc.content = indices.data;
		desc.size = indices.byteSize();
		desc.label = "quadIndexBuffer";
		quadIndexBuffer = sg_make_buffer(&desc);
	}

	// Vertex buffer
	{
		sg_buffer_desc desc = { };
		desc.type = SG_BUFFERTYPE_VERTEXBUFFER;
		desc.size = MaxQuadsPerDraw * sizeof(Quad);
		desc.usage = SG_USAGE_STREAM;
		desc.label = "quadVertexBuffer";
		quadVertexBuffer = sg_make_buffer(&desc);
	}

	spriteBindings.vertex_buffers[0] = quadVertexBuffer;
	spriteBindings.index_buffer = quadIndexBuffer;
}

CanvasRenderer::~CanvasRenderer()
{
	sg_destroy_shader(spriteShader);
}

static uint32_t packChannel(float f)
{
	return sf::clamp((uint32_t)(f * 255.0f), 0u, 255u);
}

static uint32_t packColor(const sf::Vec4 &col)
{
	uint32_t r = packChannel(col.x);
	uint32_t g = packChannel(col.y);
	uint32_t b = packChannel(col.z);
	uint32_t a = packChannel(col.w);
	return r | (g << 8) | (b << 16) | (a << 24);
}

static void spriteToQuad(Quad &quad, const SpriteDraw &draw, const SpriteResidency &residency)
{
	Sprite *sprite = draw.sprite;
	float rcpAtlasWidth = 1.0f / (float)residency.atlas->width;
	float rcpAtlasHeight = 1.0f / (float)residency.atlas->height;

	float uvMinX = (float)residency.x * rcpAtlasWidth;
	float uvMinY = (float)residency.y * rcpAtlasWidth;
	float uvMaxX = (float)(residency.x + sprite->width) * rcpAtlasWidth;
	float uvMaxY = (float)(residency.y + sprite->height) * rcpAtlasWidth;

	uint32_t color = packColor(draw.color);

	const sf::Mat23 &t = draw.transform;

	quad.v[0].position.x = t.m02;
	quad.v[0].position.y = t.m12;
	quad.v[0].texCoord.x = uvMinX;
	quad.v[0].texCoord.y = uvMinY;
	quad.v[0].color = color;

	quad.v[1].position.x = t.m02 + t.m00;
	quad.v[1].position.y = t.m12 + t.m10;
	quad.v[1].texCoord.x = uvMaxX;
	quad.v[1].texCoord.y = uvMinY;
	quad.v[1].color = color;

	quad.v[2].position.x = t.m02 + t.m01;
	quad.v[2].position.y = t.m12 + t.m11;
	quad.v[2].texCoord.x = uvMinX;
	quad.v[2].texCoord.y = uvMaxY;
	quad.v[2].color = color;

	quad.v[3].position.x = t.m02 + t.m00 + t.m01;
	quad.v[3].position.y = t.m12 + t.m10 + t.m11;
	quad.v[3].texCoord.x = uvMaxX;
	quad.v[3].texCoord.y = uvMaxY;
	quad.v[3].color = color;
}

void CanvasRenderer::drawSprites(sf::Slice<SpriteDraw> draws, Atlas *atlas)
{
	sf_assert(draws.size <= MaxQuadsPerDraw);

	sf::SmallArray<SpriteResidency, 16> residency;

	quads.clear();
	for (const SpriteDraw &draw : draws) {
		Sprite *sprite = draw.sprite;
		sf_assert(sprite->isLoaded());

		residency.clear();
		sprite->getResidency(residency);
		SpriteResidency *res = nullptr;
		for (SpriteResidency &r : residency) {
			if (r.atlas == atlas) {
				res = &r;
				break;
			}
		}
		sf_assert(res != nullptr);

		Quad &quad = quads.pushUninit();
		spriteToQuad(quad, draw, *res);
	}

	uint32_t offset = sg_append_buffer(quadVertexBuffer, quads.data, quads.byteSize());

	spriteBindings.vertex_buffer_offsets[0] = offset;
	spriteBindings.fs_images[SLOT_Sprite_atlasTexture].id = atlas->getTexture();

	sg_draw(0, 6 * draws.size, 1);
}

void CanvasImp::render(const CanvasRenderOpts &opts)
{
	sf::Slice<SpriteDraw> draws = spriteDraws;

	sf::SmallArray<Atlas*, 16> atlases;
	sf::SmallArray<SpriteResidency, 16> residency;

	CanvasRenderer &renderer = getCanvasRenderer();

	sg_apply_pipeline(renderer.spritePipeline);

	Sprite_Transform_t transform;
	opts.transform.writeColMajor(transform.transform);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_Sprite_Transform, &transform, sizeof(transform));

	uint32_t begin = 0;
	while (begin < draws.size) {
		atlases.clear();

		Sprite *sprite = draws[begin].sprite;
		if (!sprite->shouldBeLoaded()) {
			begin++;
			continue;
		}

		sprite->getResidency(residency);
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
		for (; end < draws.size && end - begin < MaxQuadsPerDraw; end++) {
			sprite = draws[end].sprite;
			if (!sprite->shouldBeLoaded()) continue;

			draws[end].sprite->getResidency(residency);
			if (residency.size == 0) continue;

			filterAtlases(atlases, residency);
			if (atlases.size == 0) break;
			atlas = atlases[0];
		}

		renderer.drawSprites(sf::slice(draws.data + begin, end - begin), atlas);

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
	if (!s) return;
	if (s->shouldBeLoaded()) {
		s->willBeRendered();
	}

	SpriteDraw &draw = imp->spriteDraws.push();
	draw.sprite.reset(s);
	draw.transform = transform;
}

void Canvas::drawText(Font *f, sf::String str, const sf::Vec2 &pos)
{
}

void Canvas::render(const CanvasRenderOpts &opts)
{
	imp->render(opts);
}

}
