#include "Font.h"

#include "ContentFile.h"
#include "Image.h"
#include "Canvas.h"

#include "sf/Array.h"
#include "sf/HashMap.h"
#include "sf/Mutex.h"

#include "ext/stb/stb_truetype.h"
#include "ext/sokol/sokol_gfx.h"
#include "ext/sokol/sokol_config.h"

namespace sp {

static const uint32_t CellSize = 64;
static const uint32_t AtlasSize = 2048;
static const uint32_t AtlasCellStride = AtlasSize / CellSize;
static const uint32_t MaxAtlasSlots = (AtlasSize * AtlasSize) / (CellSize * CellSize);
static const uint32_t SdfPadding = 8;
static const float SdfStepsPerPixel = 16.0f;

struct AtlasSlot
{
	uint16_t prev, next;
	uint32_t globalGlyphIndex;
};

struct FontImp;

struct FontContext
{
	sf::Mutex mutex;
	sf::Array<FontImp*> fonts;
	sf::Array<uint32_t> freeList;

	sf::Array<AtlasSlot> atlasSlots;

	sf::Array<FontImp*> updateList;

	uint32_t globalGlyphIndexCounter = 0;
	sg_image atlasImage;
};

FontContext g_fontContext;

static sf_forceinline void retainSlot(FontContext &ctx, uint16_t slot)
{
	AtlasSlot *s = ctx.atlasSlots.data;
	sf_assert(slot != 0);
	if (s[slot].next != 0) {
		// Unlink
		s[s[slot].prev].next = s[slot].next;
		s[s[slot].next].prev = s[slot].prev;

		// Link
		sf_assert(s[s[0].prev].next == 0);
		s[s[0].prev].next = slot;
		s[slot].prev = s[0].prev;
		s[0].prev = slot;
		s[slot].next = 0;
	}
}

static uint16_t allocateSlot(FontContext &ctx)
{
	if (ctx.atlasSlots.size <= MaxAtlasSlots) {
		ctx.atlasSlots.push();
		uint16_t slot = (uint16_t)(ctx.atlasSlots.size - 1);
		sf_assert(slot != 0);
		AtlasSlot *s = ctx.atlasSlots.data;
		if (slot == 1) {
			s[0].next = slot;
			s[0].prev = slot;
		} else {
			sf_assert(s[s[0].prev].next == 0);
			s[s[0].prev].next = slot;
			s[slot].prev = s[0].prev;
			s[0].prev = slot;
			s[slot].next = 0;
		}
		return slot;
	} else {
		uint32_t index = ctx.atlasSlots[0].next;
		retainSlot(ctx, index);
		return index;
	}
}

struct Glyph
{
	int ttfGlyph = -1;
	uint16_t slot = 0;
	uint32_t globalGlyphIndex = 0;
	float rasterScale;
	int16_t advance, bearing;
	int16_t width, height, xoff, yoff;
	unsigned char *sdf = nullptr;
};

struct GlyphPair
{
	int a, b;

	bool operator==(const GlyphPair &rhs) const
	{
		return a == rhs.a && b == rhs.b;
	}
};

uint32_t hash(const GlyphPair &pair)
{
	return sf::hashCombine(sf::hash((uint32_t)pair.a), sf::hash((uint32_t)pair.b));
}

struct FontImp : Font
{
	sf::Mutex mutex;
	uint32_t index;

	sf::Array<char> dataCopy;
	stbtt_fontinfo info;

	sf::HashMap<GlyphPair, int> kerningPairs;
	uint32_t kerningPairRemoveIndex = 0;

	sf::HashMap<uint32_t, Glyph> glyphs;
	sf::Array<uint32_t> codepointsToAlloc;
	bool inUpdateList = false;

	virtual void assetStartLoading() final;
	virtual void assetUnload() final;
};

AssetType Font::SelfType = { "Font", sizeof(FontImp), sizeof(Font::PropType),
	[](Asset *a) { new ((FontImp*)a) FontImp(); }
};

static void loadImp(void *user, const ContentFile &file)
{
	FontImp *imp = (FontImp*)user;
	if (!file.isValid()) {
		imp->assetFailLoading();
		return;
	}

	const void *data = file.data;
	if (!file.stableData) {
		imp->dataCopy.push((const char*)file.data, file.size);
		data = imp->dataCopy.data;
	}

	if (!stbtt_InitFont(&imp->info, (const unsigned char*)data, 0)) {
		imp->assetFailLoading();
		return;
	}

	{
		FontContext &ctx = g_fontContext;
		sf::MutexGuard mg(ctx.mutex);

		uint32_t index = 0;
		if (ctx.freeList.size > 0) {
			index = ctx.freeList.popValue();
		} else {
			index = ctx.fonts.size;
			ctx.fonts.pushUninit();
		}
		ctx.fonts[index] = imp;
		imp->index = index;
	}

#if 0
	sf::Array<stbtt_kerningentry> kern;
	kern.resizeUninit(stbtt_GetKerningTableLength(&imp->info));
	stbtt_GetKerningTable(&imp->info, kern.data, kern.size);
	imp->kerningPairs.reserve(kern.size);
	for (stbtt_kerningentry &entry : kern) {
		GlyphPair pair = { entry.glyph1, entry.glyph2 };
		imp->kerningPairs[pair] = entry.advance;
	}
#endif

	imp->assetFinishLoading();
}

void FontImp::assetStartLoading()
{
	ContentFile::loadAsync(name, &loadImp, this);
}

void FontImp::assetUnload()
{
	FontContext &ctx = g_fontContext;
	sf::MutexGuard mg(ctx.mutex);
	ctx.freeList.push(index);
}

static uint32_t decodeUtf8Trail(const char *data, size_t size, size_t &pos, uint32_t a)
{
	const unsigned char *d = (const unsigned char*)data + pos;
	size_t left = size - pos;
	if ((a & 0xe0) == 0xc0 && left >= 1) {
		pos += 1;
		return (a&0x1f)<<6 | (d[0]&0x3f);
	} else if ((a & 0xf0) == 0xe0 && left >= 2) {
		pos += 2;
		uint32_t code = (a&0x0f)<<12 | (d[0]&0x3f)<<6 | (d[1]&0x3f);
		if (code - 0xd800 <= 0xdfff - 0xd800) code = ~0u;
		return code;
	} else if ((a & 0xf8) == 0xf0 && a <= 0xf4 && left >= 3) {
		pos += 3;
		return (a&0x07)<<18 | (d[0]&0x3f)<<12 | (d[1]&0x3f)<<6 | (d[2]&0x3f);
	} else {
		if (left > 0) pos++;
		return ~0u;
	}
}

static void initGlyph(FontImp *font, Glyph &glyph, uint32_t code)
{
	sf_assert(glyph.ttfGlyph < 0);
	glyph.ttfGlyph = stbtt_FindGlyphIndex(&font->info, (int)code);

	int advance, bearing;
	stbtt_GetGlyphHMetrics(&font->info, glyph.ttfGlyph, &advance, &bearing);
	glyph.advance = (int16_t)sf::max(advance, 0);
	glyph.bearing = (int16_t)bearing;
}

static void rasterizeGlyph(FontImp *font, Glyph &glyph, uint32_t code)
{
	if (glyph.sdf || glyph.width < 0) return;

	float scale = stbtt_ScaleForPixelHeight(&font->info, CellSize);
	unsigned char *sdf;
	int width, height, xoff, yoff;
	for (;;) {
		int ix0,iy0,ix1,iy1;
		stbtt_GetGlyphBitmapBoxSubpixel(&font->info, glyph.ttfGlyph, scale, scale, 0.0f,0.0f, &ix0,&iy0,&ix1,&iy1);
		width = (ix1 - ix0) + 2*SdfPadding;
		height = (iy1 - iy0) + 2*SdfPadding;
		if (width <= CellSize && height <= CellSize) break;
		scale *= 0.9f;
	}

	int sdfWidth, sdfHeight;
	sdf = stbtt_GetGlyphSDF(&font->info, scale, glyph.ttfGlyph, SdfPadding, 127, SdfStepsPerPixel, &sdfWidth, &sdfHeight, &xoff, &yoff);
	if (!sdf) {
		// Empty character
		glyph.width = -1;
		glyph.height = -1;
		return;
	}
	sf_assert(sdfWidth == width);
	sf_assert(sdfHeight == height);

	glyph.width = (int16_t)width;
	glyph.height = (int16_t)height;
	glyph.xoff = (int16_t)xoff;
	glyph.yoff = (int16_t)yoff;
	glyph.rasterScale = scale;

	glyph.sdf = sdf;
}

void Font::retainSlots(sf::String text)
{
    FontContext &ctx = g_fontContext;
    FontImp *imp = (FontImp*)this;
    
    sf::MutexGuard mg(ctx.mutex);
    sf::MutexGuard mg2(imp->mutex);

    size_t pos = 0;
    while (pos < text.size) {
        uint32_t code = (unsigned char)text.data[pos++];
        if (code >= 0x80) {
            code = decodeUtf8Trail(text.data, text.size, pos, code);
            if (code == ~0u) continue;
        }
        
        Glyph &glyph = imp->glyphs[code];
        if (glyph.ttfGlyph < 0) {
            initGlyph(imp, glyph, code);
        }
        
        if (glyph.globalGlyphIndex == 0) {
            glyph.globalGlyphIndex = ++ctx.globalGlyphIndexCounter;
        }
        
        if (glyph.width >= 0) {
            if (glyph.slot == 0 || ctx.atlasSlots[glyph.slot].globalGlyphIndex != glyph.globalGlyphIndex) {
                rasterizeGlyph(imp, glyph, code);
                if (glyph.width >= 0) {
                    glyph.slot = allocateSlot(ctx);
                    ctx.atlasSlots[glyph.slot].globalGlyphIndex = glyph.globalGlyphIndex;
                    
                    imp->codepointsToAlloc.push(code);
                    if (!imp->inUpdateList) {
                        imp->inUpdateList = true;
                        ctx.updateList.push(imp);
                    }
                }
            } else {
                retainSlot(ctx, glyph.slot);
            }
        }
    }
}

void Font::getQuads(sf::Array<FontQuad> &quads, const sp::TextDraw &draw, uint32_t color, sf::String text, size_t maxQuads)
{
	FontContext &ctx = g_fontContext;
	FontImp *imp = (FontImp*)this;

	sf::MutexGuard mg(ctx.mutex);
	sf::MutexGuard mg2(imp->mutex);

	float scale = stbtt_ScaleForPixelHeight(&imp->info, draw.height);

	size_t quadsLeft = maxQuads;
	float rcpAtlasSize = 1.0f / AtlasSize;

	sf::Vec2 nextOrigin;

	int prevTttfGlyph = -1;

	size_t pos = 0;
	while (pos < text.size && quadsLeft > 0) {
		uint32_t code = (unsigned char)text.data[pos++];
		if (code >= 0x80) {
			code = decodeUtf8Trail(text.data, text.size, pos, code);
			if (code == ~0u) continue;
		}

		Glyph &glyph = imp->glyphs[code];
		if (glyph.ttfGlyph < 0) {
			initGlyph(imp, glyph, code);
		}

		if (prevTttfGlyph >= 0) {
			GlyphPair pair = { prevTttfGlyph, glyph.ttfGlyph };
			auto res = imp->kerningPairs.insert(pair);
			if (res.inserted) {
				res.entry.val = stbtt_GetGlyphKernAdvance(&imp->info, prevTttfGlyph, glyph.ttfGlyph);
			}
			nextOrigin.x += (float)res.entry.val * scale;
		}

		prevTttfGlyph = glyph.ttfGlyph;

		if (glyph.width > 0) {
			FontQuad &quad = quads.push();

			float scaleRatio = scale / glyph.rasterScale;

			sf::Vec2 origin = nextOrigin;
			// origin.x += (float)glyph.bearing * scale;

			float radius = 0.45f;
			float width = AtlasSize * (SdfStepsPerPixel / 255.0f) * 1.0f;

			uint32_t params = 0;
			params |= sf::clamp((uint32_t)(radius * 255.0f), 0u, 255u) << 0;
			params |= sf::clamp((uint32_t)(width), 0u, 255u) << 8;

			sf::Vec2 min = origin + sf::Vec2((float)glyph.xoff, (float)glyph.yoff) * scaleRatio;
			sf::Vec2 max = min + sf::Vec2((float)glyph.width, (float)glyph.height) * scaleRatio;
			const sf::Mat23 &t = draw.transform;

			uint32_t slotX = (glyph.slot - 1) % AtlasCellStride * CellSize;
			uint32_t slotY = (glyph.slot - 1) / AtlasCellStride * CellSize;
			float uvMinX = (float)slotX * rcpAtlasSize;
			float uvMinY = (float)slotY * rcpAtlasSize;
			float uvMaxX = (float)(slotX + glyph.width) * rcpAtlasSize;
			float uvMaxY = (float)(slotY + glyph.height) * rcpAtlasSize;

			float xx0 = min.x*t.m00 + t.m02;
			float xx1 = max.x*t.m00 + t.m02;
			float yy0 = min.y*t.m11 + t.m12;
			float yy1 = max.y*t.m11 + t.m12;

			float xy0 = min.y*t.m01;
			float xy1 = max.y*t.m01;
			float yx0 = min.x*t.m10;
			float yx1 = max.x*t.m10;

			quad.v[0].position.x = xx0 + xy0;
			quad.v[0].position.y = yx0 + yy0;
			quad.v[0].texCoord.x = uvMinX;
			quad.v[0].texCoord.y = uvMinY;
			quad.v[0].color = color;
			quad.v[0].params = params;

			quad.v[1].position.x = xx1 + xy0;
			quad.v[1].position.y = yx1 + yy0;
			quad.v[1].texCoord.x = uvMaxX;
			quad.v[1].texCoord.y = uvMinY;
			quad.v[1].color = color;
			quad.v[1].params = params;

			quad.v[2].position.x = xx0 + xy1;
			quad.v[2].position.y = yx0 + yy1;
			quad.v[2].texCoord.x = uvMinX;
			quad.v[2].texCoord.y = uvMaxY;
			quad.v[2].color = color;
			quad.v[2].params = params;

			quad.v[3].position.x = xx1 + xy1;
			quad.v[3].position.y = yx1 + yy1;
			quad.v[3].texCoord.x = uvMaxX;
			quad.v[3].texCoord.y = uvMaxY;
			quad.v[3].color = color;
			quad.v[3].params = params;
				
			quadsLeft--;
		}

		nextOrigin.x += (float)glyph.advance * scale;
	}
}

sf::Vec2 Font::measureText(sf::String text, float height)
{
	FontContext &ctx = g_fontContext;
	FontImp *imp = (FontImp*)this;

	sf::MutexGuard mg(ctx.mutex);
	sf::MutexGuard mg2(imp->mutex);

	if (!imp->isLoaded()) return sf::Vec2(0.0f);

	float scale = stbtt_ScaleForPixelHeight(&imp->info, height);

	sf::Vec2 nextOrigin;

	int prevTttfGlyph = -1;

	size_t pos = 0;
	while (pos < text.size) {
		uint32_t code = (unsigned char)text.data[pos++];
		if (code >= 0x80) {
			code = decodeUtf8Trail(text.data, text.size, pos, code);
			if (code == ~0u) continue;
		}

		Glyph &glyph = imp->glyphs[code];
		if (glyph.ttfGlyph < 0) {
			initGlyph(imp, glyph, code);
		}

		if (prevTttfGlyph >= 0) {
			int kern = 0;
			GlyphPair pair = { prevTttfGlyph, glyph.ttfGlyph };
			auto it = imp->kerningPairs.find(pair);
			if (it != nullptr) {
				kern = it->val;
			}
			nextOrigin.x += (float)kern * scale;
		}

		prevTttfGlyph = glyph.ttfGlyph;
		nextOrigin.x += (float)glyph.advance * scale;
	}

	nextOrigin.y += height;

	return nextOrigin;
}

struct AtlasUpdate
{
	FontImp *font;
	uint16_t slot;
	unsigned char *sdf;
	int16_t width, height;
};

void Font::updateAtlasesForRendering()
{
	FontContext &ctx = g_fontContext;
	sf::MutexGuard mg(ctx.mutex);

	if (ctx.updateList.size == 0) return;

	sf::SmallArray<AtlasUpdate, 128> updates;
    
	for (FontImp *imp : ctx.updateList) {
		sf::MutexGuard mg(imp->mutex);
		imp->inUpdateList = false;

		for (uint32_t code : imp->codepointsToAlloc) {
			Glyph &glyph = imp->glyphs[code];

			// In really bad cases we might have duplicates in `codepointsToAlloc`
			if (!glyph.sdf) continue;

			AtlasUpdate &update = updates.push();
			update.font = imp;
			update.slot = glyph.slot;
			update.sdf = glyph.sdf;
			update.width = glyph.width;
			update.height = glyph.height;
            glyph.sdf = nullptr;
		}
		imp->codepointsToAlloc.clear();
	}
	ctx.updateList.clear();

	uint32_t widthInCells = updates.size;
	uint32_t heightInCells = 1;

	if (widthInCells * CellSize >= 1024) {
		widthInCells = (uint32_t)ceilf(sf::sqrt((float)updates.size));
		heightInCells = (updates.size + widthInCells - 1) / widthInCells;
	}

	uint32_t widthInPixels = widthInCells * CellSize;
	uint32_t heightInPixels = heightInCells * CellSize;

	sf::Array<char> data;
	#if defined(SOKOL_GLES2)
		// WebGL1 cannot copy from R8 textures
		data.resizeUninit(widthInPixels * heightInPixels * 4);
	#else
		data.resizeUninit(widthInPixels * heightInPixels);
	#endif

	uint32_t index = 0;

	sf::SmallArray<sg_bqq_subimage_rect, 128> rects;

	for (AtlasUpdate &update : updates) {
		FontImp *font = update.font;

		uint32_t cellX = index % widthInCells;
		uint32_t cellY = index / widthInCells;

		for (int dy = 0; dy < update.height; dy++) {
			#if defined(SOKOL_GLES2)
				for (int dx = 0; dx < update.width; dx++) {
					uint32_t base = ((cellY*CellSize + dy)*widthInPixels + cellX*CellSize + dx) * 4;
					char value = update.sdf[dy*update.width + dx];
					data.data[base + 0] = value;
					data.data[base + 1] = value;
					data.data[base + 2] = value;
					data.data[base + 3] = value;
				}
			#else
				memcpy(data.data + (cellY*CellSize + dy)*widthInPixels + cellX*CellSize, update.sdf + dy*update.width, update.width);
			#endif
		}

		uint32_t dstCellX = (update.slot - 1) % AtlasCellStride;
		uint32_t dstCellY = (update.slot - 1) / AtlasCellStride;

		sg_bqq_subimage_rect &rect = rects.push();
		rect.src_x = cellX * CellSize;
		rect.src_y = cellY * CellSize;
		rect.dst_x = dstCellX * CellSize;
		rect.dst_y = dstCellY * CellSize;
		rect.width = update.width; // TEST: CellSize
		rect.height = update.height; // TEST: CellSize

		stbtt_FreeSDF(update.sdf, nullptr);
		index++;
	}

	sg_image copySrc;

	{
		sg_image_desc desc = { };
		#if defined(SOKOL_GLES2)
			desc.pixel_format = SG_PIXELFORMAT_RGBA8;
		#else
			desc.pixel_format = SG_PIXELFORMAT_R8;
		#endif
		desc.width = widthInPixels;
		desc.height = heightInPixels;
		desc.content.subimage[0][0].ptr = data.data;
		desc.content.subimage[0][0].size = data.size;
		copySrc = sg_make_image(&desc);
	}

	{
		sg_bqq_subimage_copy_desc desc = { };
		desc.dst_image = ctx.atlasImage;
		desc.src_image = copySrc;
		desc.rects = rects.data;
		desc.num_rects = rects.size;
		desc.num_mips = 1;
		sg_bqq_copy_subimages(&desc);
	}

	sg_destroy_image(copySrc);
}

sg_image Font::getFontAtlasImage()
{
	FontContext &ctx = g_fontContext;
	return ctx.atlasImage;
}

void Font::globalInit()
{
	FontContext &ctx = g_fontContext;

	// Push zero atlas slot
	AtlasSlot &slot = ctx.atlasSlots.push();
	slot.next = slot.prev = 0;

	{
		sg_image_desc desc = { };
		desc.pixel_format = SG_PIXELFORMAT_R8;
		desc.width = AtlasSize;
		desc.height = AtlasSize;
		desc.min_filter = SG_FILTER_LINEAR;
		desc.mag_filter = SG_FILTER_LINEAR;
		desc.bqq_copy_target = true;
		desc.label = "FontAtlas";
		ctx.atlasImage = sg_make_image(&desc);
	}
}

void Font::globalCleanup()
{
	FontContext &ctx = g_fontContext;

	sg_destroy_image(ctx.atlasImage);
}

void Font::globalUpdate()
{
}

}
