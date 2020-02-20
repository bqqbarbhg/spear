#include "Canvas.h"

#include "Sprite.h"
#include "sf/Array.h"
#include "sf/Sort.h"

#include "ext/sokol/sokol_gfx.h"
#include "shader/Sprite.h"

namespace sp {

static const uint32_t MaxQuadsPerDraw = 1024;
static const uint32_t MaxQuadsPerFrame = 16*1024;

struct SpriteDrawImp
{
	SpriteDraw draw;
	uint64_t sortKey;
};

struct TextDrawImp
{
	TextDraw draw;
	uint64_t sortKey;
};

struct CanvasDrawImp
{
	CanvasDraw draw;
	uint64_t sortKey;
};

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

struct CanvasContext
{
	sg_shader spriteShader;
	sg_pipeline spritePipeline;
	sg_bindings spriteBindings = { };

	sg_buffer quadIndexBuffer;
	sg_buffer quadVertexBuffer;

	sf::Array<Quad> quads;
};

CanvasContext g_canvasContext;

struct CanvasImp
{
	CanvasImp() = default;
	CanvasImp(CanvasImp&&) = default;
	CanvasImp(const CanvasImp &) = delete;

	sf::Array<sf::StringBuf> textData;
	sf::Array<SpriteDrawImp> spriteDraws;
	sf::Array<TextDrawImp> textDraws;
	sf::Array<CanvasDrawImp> canvasDraws;
	uint32_t nextDrawIndex = 0;

	bool spriteDrawsSorted = true;
	bool textDrawsSorted = true;
	bool canvasDrawsSorted = true;
};

static_assert(sizeof(Canvas::impData) >= sizeof(CanvasImp), "impData too small");
static_assert(sizeof(Canvas::impData) <= sizeof(CanvasImp) * 4, "impData too large");

Canvas::Canvas()
{
	new ((CanvasImp*)impData) CanvasImp();
}

Canvas::Canvas(Canvas &&rhs) noexcept
{
	CanvasImp *rhsImp = (CanvasImp*)rhs.impData;
	new ((CanvasImp*)impData) CanvasImp(std::move(*rhsImp));
}

Canvas::~Canvas()
{
	((CanvasImp*)impData)->~CanvasImp();
}

void Canvas::clear()
{
	CanvasImp *imp = (CanvasImp*)impData;

	for (SpriteDrawImp &draw : imp->spriteDraws) {
		draw.draw.sprite->release();
	}
	for (TextDrawImp &draw : imp->textDraws) {
		// TODO draw.draw.font->release();
		sf_failf("TODO");
	}

	imp->spriteDraws.clear();
	imp->textDraws.clear();
	imp->canvasDraws.clear();

	imp->spriteDrawsSorted = true;
	imp->textDrawsSorted = true;
	imp->canvasDrawsSorted = true;

	imp->nextDrawIndex = 0;

	imp->textData.clear();
}

static uint64_t makeSortKey(float depth, uint32_t index)
{
	// Convert float to sortable integer
	uint32_t depthBits;
	memcpy(&depthBits, &depth, sizeof(float));
	// Flip the sign bit (and other bits if negative)
	// to get proper unsigned sorting
	depthBits ^= (uint32_t)-(int32_t)depthBits | 0x80000000u;

	return (uint64_t)depthBits << 32u | (uint64_t)index;
}

void Canvas::draw(const SpriteDraw &draw)
{
	CanvasImp *imp = (CanvasImp*)impData;
	if (!draw.sprite) return;
	draw.sprite->retain();

	if (imp->spriteDraws.size > 0 && draw.depth > imp->spriteDraws.back().draw.depth) {
		imp->spriteDrawsSorted = false;
	}
	SpriteDrawImp &drawImp = imp->spriteDraws.pushUninit();
	drawImp.draw = draw;
	drawImp.sortKey = makeSortKey(draw.depth, imp->nextDrawIndex++);
}

void Canvas::drawText(const TextDraw &draw)
{
	CanvasImp *imp = (CanvasImp*)impData;
	if (!draw.font) return;
	// TODO draw.font->retain();
	sf_failf("TODO");

	if (imp->textDraws.size > 0 && draw.depth > imp->textDraws.back().draw.depth) {
		imp->textDrawsSorted = false;
	}
	TextDrawImp &drawImp = imp->textDraws.pushUninit();
	drawImp.draw = draw;
	drawImp.sortKey = makeSortKey(draw.depth, imp->nextDrawIndex++);
}

void Canvas::drawCanvas(const CanvasDraw &draw)
{
	CanvasImp *imp = (CanvasImp*)impData;
	if (!draw.canvas) return;

	if (imp->canvasDraws.size > 0 && draw.depth > imp->canvasDraws.back().draw.depth) {
		imp->canvasDrawsSorted = false;
	}
	CanvasDrawImp &drawImp = imp->canvasDraws.pushUninit();
	drawImp.draw = draw;
	drawImp.sortKey = makeSortKey(draw.depth, imp->nextDrawIndex++);
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

static void spriteToQuad(Quad &quad, const SpriteDraw &draw, const CanvasRenderOpts &opts)
{
	Sprite *sprite = draw.sprite;
	float rcpAtlasWidth = 1.0f / (float)sprite->atlas->width;
	float rcpAtlasHeight = 1.0f / (float)sprite->atlas->height;

	float uvMinX = (float)sprite->x * rcpAtlasWidth;
	float uvMinY = (float)sprite->y * rcpAtlasHeight;
	float uvMaxX = (float)(sprite->x + sprite->width) * rcpAtlasWidth;
	float uvMaxY = (float)(sprite->y + sprite->height) * rcpAtlasHeight;

	uint32_t color = packColor(draw.color * opts.color);

	const sf::Mat23 &t = draw.transform;

	float xx0 = sprite->minVert.x*t.m00 + t.m02;
	float xx1 = sprite->maxVert.x*t.m00 + t.m02;
	float yy0 = sprite->minVert.y*t.m11 + t.m12;
	float yy1 = sprite->maxVert.y*t.m11 + t.m12;

	float xy0 = sprite->minVert.y*t.m01;
	float xy1 = sprite->maxVert.y*t.m01;
	float yx0 = sprite->minVert.x*t.m10;
	float yx1 = sprite->maxVert.x*t.m10;

	quad.v[0].position.x = xx0 + xy0;
	quad.v[0].position.y = yx0 + yy0;
	quad.v[0].texCoord.x = uvMinX;
	quad.v[0].texCoord.y = uvMinY;
	quad.v[0].color = color;

	quad.v[1].position.x = xx1 + xy0;
	quad.v[1].position.y = yx1 + yy0;
	quad.v[1].texCoord.x = uvMaxX;
	quad.v[1].texCoord.y = uvMinY;
	quad.v[1].color = color;

	quad.v[2].position.x = xx0 + xy1;
	quad.v[2].position.y = yx0 + yy1;
	quad.v[2].texCoord.x = uvMinX;
	quad.v[2].texCoord.y = uvMaxY;
	quad.v[2].color = color;

	quad.v[3].position.x = xx1 + xy1;
	quad.v[3].position.y = yx1 + yy1;
	quad.v[3].texCoord.x = uvMaxX;
	quad.v[3].texCoord.y = uvMaxY;
	quad.v[3].color = color;
}

static void drawSprites(CanvasContext &ctx, sf::Slice<SpriteDrawImp> draws, Atlas *atlas, const CanvasRenderOpts &opts)
{
	sf_assert(draws.size > 0);
	sf_assert(draws.size <= MaxQuadsPerDraw);

	ctx.quads.clear();
	for (const SpriteDrawImp &drawImp : draws) {
		const SpriteDraw &draw = drawImp.draw;

		Sprite *sprite = draw.sprite;
		sf_assert(sprite->isLoaded() && sprite->atlas == atlas);

		Quad &quad = ctx.quads.pushUninit();
		spriteToQuad(quad, draw, opts);
	}

	uint32_t offset = sg_append_buffer(ctx.quadVertexBuffer, ctx.quads.data, ctx.quads.byteSize());

	ctx.spriteBindings.vertex_buffer_offsets[0] = offset;
	ctx.spriteBindings.fs_images[SLOT_Sprite_atlasTexture] = atlas->image;


	sg_apply_bindings(&ctx.spriteBindings);
	sg_draw(0, 6 * draws.size, 1);
}

static void drawTexts(CanvasContext &ctx, sf::Slice<TextDrawImp> draws)
{
	sf_assert(draws.size > 0);
	sf_assert(draws.size <= MaxQuadsPerDraw);
}

static void drawCanvases(CanvasContext &ctx, sf::Slice<CanvasDrawImp> draws)
{
}

void Canvas::render(const CanvasRenderOpts &opts)
{
	CanvasContext &ctx = g_canvasContext;
	CanvasImp *imp = (CanvasImp*)impData;

	// Sort lists if necessary

	if (!imp->spriteDrawsSorted) {
		imp->spriteDrawsSorted = true;
		sf::sortBy(imp->spriteDraws, [](const SpriteDrawImp &draw) {
			return draw.sortKey;
		});
	}

	if (!imp->textDrawsSorted) {
		imp->textDrawsSorted = true;
		sf::sortBy(imp->textDraws, [](const TextDrawImp &draw) {
			return draw.sortKey;
		});
	}

	if (!imp->canvasDrawsSorted) {
		imp->canvasDrawsSorted = true;
		sf::sortBy(imp->canvasDraws, [](const CanvasDrawImp &draw) {
			return draw.sortKey;
		});
	}

	uint32_t spriteI = 0;
	uint32_t textI = 0;
	uint32_t canvasI = 0;

	uint64_t nextSprite = imp->spriteDraws.size > 0 ? imp->spriteDraws[0].sortKey : UINT64_MAX;
	uint64_t nextText = imp->textDraws.size > 0 ? imp->textDraws[0].sortKey : UINT64_MAX;
	uint64_t nextCanvas = imp->canvasDraws.size > 0 ? imp->canvasDraws[0].sortKey : UINT64_MAX;

	uint64_t next = sf::min(nextSprite, nextText, nextCanvas);
	while (next != UINT64_MAX) {

		// All sort keys are unique so dispatch to the type whose sort key
		// is equal to the lowest one
		if (next == nextSprite) {
			next = sf::min(nextText, nextCanvas);
			SpriteDrawImp *draws = imp->spriteDraws.data;
			uint32_t numDraws = imp->spriteDraws.size;

			// Setup sprite rendering
			sg_apply_pipeline(ctx.spritePipeline);
			Sprite_Transform_t transform;
			opts.transform.writeColMajor(transform.transform);
			sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_Sprite_Transform, &transform, sizeof(transform));

			do {
				Sprite *sprite = draws[spriteI].draw.sprite;
				if (!sprite->shouldBeLoaded()) {
					spriteI++;
					continue;
				}

				Atlas *atlas = sprite->atlas;

				// Keep appending sprites that are in the same atlas
				uint32_t end = spriteI + 1;
				for (; end < numDraws && end - spriteI < MaxQuadsPerDraw; end++) {
					if (draws[end].sortKey > next) break;
					sprite = draws[end].draw.sprite;
					if (!sprite->shouldBeLoaded()) break;
					if (sprite->atlas != atlas) {
						atlas->brokeBatch();
						sprite->atlas->brokeBatch();
						break;
					}
				}

				drawSprites(ctx, sf::slice(draws + spriteI, end - spriteI), atlas, opts);

				nextSprite = spriteI < numDraws ? draws[spriteI].sortKey : UINT64_MAX;
			} while (nextSprite < next);

		} else if (next == nextText) {
			next = sf::min(nextSprite, nextCanvas);
			TextDrawImp *draws = imp->textDraws.data;
			uint32_t numDraws = imp->textDraws.size;

			do {
				sf_failf("TODO");

				nextText = textI < numDraws ? draws[textI].sortKey : UINT64_MAX;
			} while (nextText < next);

		} else if (next == nextCanvas) {
			next = sf::min(nextSprite, nextText);
			CanvasDrawImp *draws = imp->canvasDraws.data;
			uint32_t numDraws = imp->canvasDraws.size;

			do {
				CanvasDraw &draw = draws[canvasI++].draw;

				sf::Mat44 transform;
				transform.m00 = draw.transform.m00;
				transform.m01 = draw.transform.m01;
				transform.m03 = draw.transform.m02;
				transform.m10 = draw.transform.m10;
				transform.m11 = draw.transform.m11;
				transform.m13 = draw.transform.m12;

				CanvasRenderOpts innerOpts;
				innerOpts.transform = transform * opts.transform;
				innerOpts.color = draw.color * opts.color;
				draw.canvas->render(innerOpts);

				nextCanvas = canvasI < numDraws ? draws[canvasI].sortKey : UINT64_MAX;
			} while (nextCanvas < next);
		}
	}

}

void Canvas::globalInit()
{
	CanvasContext &ctx = g_canvasContext;

	ctx.spriteShader = sg_make_shader(Sprite_Sprite_shader_desc());

	// Pipeline
	{
		sg_pipeline_desc desc = { };
		desc.shader = ctx.spriteShader;

		desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
		desc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;
		desc.layout.attrs[2].format = SG_VERTEXFORMAT_UBYTE4N;

		desc.index_type = SG_INDEXTYPE_UINT16;
		desc.blend.enabled = true;
		desc.blend.src_factor_rgb = desc.blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
		desc.blend.dst_factor_rgb = desc.blend.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;

		ctx.spritePipeline = sg_make_pipeline(&desc);
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
		ctx.quadIndexBuffer = sg_make_buffer(&desc);
	}

	// Vertex buffer
	{
		sg_buffer_desc desc = { };
		desc.type = SG_BUFFERTYPE_VERTEXBUFFER;
		desc.size = MaxQuadsPerDraw * sizeof(Quad);
		desc.usage = SG_USAGE_STREAM;
		desc.label = "quadVertexBuffer";
		ctx.quadVertexBuffer = sg_make_buffer(&desc);
	}

	ctx.spriteBindings.vertex_buffers[0] = ctx.quadVertexBuffer;
	ctx.spriteBindings.index_buffer = ctx.quadIndexBuffer;
}

void Canvas::globalCleanup()
{
	CanvasContext &ctx = g_canvasContext;
	sg_destroy_buffer(ctx.quadVertexBuffer);
	sg_destroy_buffer(ctx.quadIndexBuffer);
	sg_destroy_pipeline(ctx.spritePipeline);
	sg_destroy_shader(ctx.spriteShader);
}

void Canvas::globalUpdate()
{
}


}
