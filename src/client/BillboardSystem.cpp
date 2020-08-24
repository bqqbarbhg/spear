#include "BillboardSystem.h"

#include "sp/Sprite.h"

#include "game/shader/GameShaders.h"
#include "game/shader/Billboard.h"

#include "sp/Renderer.h"
#include "sf/Sort.h"

namespace cl {

static constexpr const uint32_t MaxBillboardsPerFrame = 4096;

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

struct BillboardSystemImp final : BillboardSystem
{
	struct BillboardImp
	{
		sp::Sprite *sprite;
		sf::Mat34 transform;
		sf::Vec2 anchor;
		uint32_t color;
		sf::Vec2 cropMin;
		sf::Vec2 cropMax;
		bool cropOnlyUv;
	};

	struct BillboardOrder
	{
		float depth;
		uint32_t index;

		bool operator<(const BillboardOrder &rhs) const {
			if (depth != rhs.depth) return depth < rhs.depth;
			if (index != rhs.index) return index < rhs.index;
			return false;
		}
	};

	struct BillboardVertex
	{
		sf::Vec3 position;
		sf::Vec2 texCoord;
		uint32_t color;
	};

	struct BillboardDraw
	{
		sp::Atlas *atlas = nullptr;
		uint32_t count = 0;
	};

	sf::Array<BillboardImp> billboards;
	sf::Array<BillboardOrder> billboardOrder;
	sf::Array<BillboardDraw> billboardDraws;

	sf::Array<BillboardVertex> billboardVertices;
	sp::Buffer billboardVertexBuffer;

	// API

	BillboardSystemImp()
	{
		billboardVertexBuffer.initDynamicVertex("Billboard vertexBuffer", sizeof(BillboardVertex) * 4 * MaxBillboardsPerFrame);
	}

	void addBillboard(sp::Sprite *sprite, const sf::Mat34 &transform, const sf::Vec4 &color, float depth) override
	{
		if (!sprite || !sprite->isLoaded()) return;

		BillboardImp &imp = billboards.pushUninit();
		imp.sprite = sprite;
		imp.transform = transform;
		imp.color = packColor(color);
		imp.anchor = sf::Vec2(0.5f);
		imp.cropMin = sf::Vec2(0.0f);
		imp.cropMax = sf::Vec2(1.0f);
		imp.cropOnlyUv = false;

		BillboardOrder &order = billboardOrder.pushUninit();
		order.depth = depth;
		order.index = billboards.size - 1;
	}

	void addBillboard(const Billboard &billboard) override
	{
		if (!billboard.sprite || !billboard.sprite->isLoaded()) return;

		BillboardImp &imp = billboards.pushUninit();
		imp.sprite = billboard.sprite;
		imp.transform = billboard.transform;
		imp.color = packColor(billboard.color);
		imp.anchor = billboard.anchor;
		imp.cropMin = billboard.cropMin;
		imp.cropMax = billboard.cropMax;
		imp.cropOnlyUv = billboard.cropOnlyUv;

		BillboardOrder &order = billboardOrder.pushUninit();
		order.depth = billboard.depth;
		order.index = billboards.size - 1;
	}

	void renderMain(const VisibleAreas &visibleAreas, const RenderArgs &renderArgs) override
	{
		billboardVertices.clear();
		billboardDraws.clear();

		sf::sort(billboardOrder);

		if (billboardOrder.size > MaxBillboardsPerFrame) {
			billboardOrder.resizeUninit(MaxBillboardsPerFrame);
		}

		for (BillboardOrder &order : billboardOrder) {
			BillboardImp &billboard = billboards[order.index];
			sp::Sprite *sprite = billboard.sprite;
			float rcpAtlasWidth = 1.0f / (float)sprite->atlas->width;
			float rcpAtlasHeight = 1.0f / (float)sprite->atlas->height;

			uint32_t color = billboard.color;

			const sf::Mat34 &t = billboard.transform;

			sf::Vec2 minVert = sf::max(sprite->minVert, billboard.cropMin);
			sf::Vec2 maxVert = sf::min(sprite->maxVert, billboard.cropMax);
			if (minVert.x >= maxVert.x || minVert.y >= maxVert.y) continue;

			float uvMinX = minVert.x * sprite->vertUvScale.x + sprite->vertUvBias.x;
			float uvMinY = minVert.y * sprite->vertUvScale.y + sprite->vertUvBias.y;
			float uvMaxX = maxVert.x * sprite->vertUvScale.x + sprite->vertUvBias.x;
			float uvMaxY = maxVert.y * sprite->vertUvScale.y + sprite->vertUvBias.y;

			minVert -= billboard.anchor;
			maxVert -= billboard.anchor;

			float xx0 = minVert.x*t.m00 + t.m03;
			float xx1 = maxVert.x*t.m00 + t.m03;
			float yy0 = minVert.y*t.m11 + t.m13;
			float yy1 = maxVert.y*t.m11 + t.m13;

			float xy0 = minVert.y*t.m01;
			float xy1 = maxVert.y*t.m01;
			float yx0 = minVert.x*t.m10;
			float yx1 = maxVert.x*t.m10;

			float zx0 = minVert.x*t.m20 + t.m23;
			float zx1 = maxVert.x*t.m20 + t.m23;
			float zy0 = minVert.y*t.m21;
			float zy1 = maxVert.y*t.m21;

			BillboardVertex *v = billboardVertices.pushUninit(4);

			v[0].position.x = xx0 + xy0;
			v[0].position.y = yx0 + yy0;
			v[0].position.z = zx0 + zy0;
			v[0].texCoord.x = uvMinX;
			v[0].texCoord.y = uvMinY;
			v[0].color = color;

			v[1].position.x = xx1 + xy0;
			v[1].position.y = yx1 + yy0;
			v[1].position.z = zx1 + zy0;
			v[1].texCoord.x = uvMaxX;
			v[1].texCoord.y = uvMinY;
			v[1].color = color;

			v[2].position.x = xx0 + xy1;
			v[2].position.y = yx0 + yy1;
			v[2].position.z = zx0 + zy1;
			v[2].texCoord.x = uvMinX;
			v[2].texCoord.y = uvMaxY;
			v[2].color = color;

			v[3].position.x = xx1 + xy1;
			v[3].position.y = yx1 + yy1;
			v[3].position.z = zx1 + zy1;
			v[3].texCoord.x = uvMaxX;
			v[3].texCoord.y = uvMaxY;
			v[3].color = color;

			sp::Atlas *atlas = sprite->atlas;
			if (billboardDraws.size > 0 && billboardDraws.back().atlas == atlas) {
				billboardDraws.back().count++;
			} else {
				billboardDraws.push({ atlas, 1 });
			}
		}

		if (billboardVertices.size > 0) {
			sg_update_buffer(billboardVertexBuffer.buffer, billboardVertices.data, (int)billboardVertices.byteSize());
			gameShaders.billboardPipe.bind();

			Billboard_Vertex_t vu;
			renderArgs.worldToClip.writeColMajor44(vu.worldToClip);
			sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_Billboard_Vertex, &vu, sizeof(vu));

			sg_bindings binds = { };

			uint32_t base = 0;
			for (BillboardDraw &draw : billboardDraws) {
				sp::Atlas *atlas = draw.atlas;

				binds.fs_images[SLOT_Billboard_u_Atlas] = atlas->image;
				binds.vertex_buffers[0] = billboardVertexBuffer.buffer;
				binds.vertex_buffer_offsets[0] = base * 4 * sizeof(BillboardVertex);
				binds.index_buffer = sp::getSharedQuadIndexBuffer();
				sg_apply_bindings(&binds);

				sg_draw(0, draw.count * 6, 1);

				base += draw.count;
			}
		}

		billboardOrder.clear();
		billboards.clear();
	}

};

sf::Box<BillboardSystem> BillboardSystem::create() { return sf::box<BillboardSystemImp>(); }

}

