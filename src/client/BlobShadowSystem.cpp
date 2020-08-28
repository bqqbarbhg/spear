#include "BlobShadowSystem.h"

#include "client/AreaSystem.h"
#include "client/CharacterModelSystem.h"

#include "game/shader/GameShaders.h"
#include "game/shader/FakeShadow.h"

#include "sp/Renderer.h"

namespace cl {

static constexpr const uint32_t MaxBlobShadowsPerFrame = 1024;

struct BlobShadowSystemImp final : BlobShadowSystem
{
	struct BoneShadow
	{
		uint32_t listenerId;
		bool active = false;
		uint64_t updateFrame = 0;
		float alpha;
		float radius;
		float fadeDistance;
		sf::Vec3 offset;
		sf::Vec3 position;

		sf::Vec3 renderPosition;
		float renderRadius;
		float renderAlpha;
	};

	struct BlobShadow
	{
		uint32_t areaId = ~0u;
		uint64_t updateFrame = 0;

		sf::Array<BoneShadow> boneShadows;
	};

	struct ShadowVertex
	{
		sf::Vec3 position;
		sf::Vec2 uv;
		float alpha;
	};

	sf::Array<BlobShadow> blobShadows;
	sf::Array<uint32_t> freeBlobShadowIds;

	sf::Array<uint32_t> updatedBlobIds;

	sf::Array<ShadowVertex> shadowVertices;
	sp::Buffer shadowVertexBuffer;

	// API

	BlobShadowSystemImp()
	{
		shadowVertexBuffer.initDynamicVertex("BlobShadow vertexBuffer", sizeof(ShadowVertex) * 4 * MaxBlobShadowsPerFrame);
	}

	void addBlobShadow(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::BlobShadowComponent &c, const Transform &transform) override
	{
		uint32_t blobId = blobShadows.size;
		if (freeBlobShadowIds.size > 0) {
			blobId = freeBlobShadowIds.popValue();
		} else {
			blobShadows.push();
		}

		BlobShadow &blob = blobShadows[blobId];

		blob.boneShadows.reserve(c.blobs.size);
		for (const sv::ShadowBlob &svBlob : c.blobs) {
			uint32_t internalIndex = blob.boneShadows.size;

			uint32_t userId = (blobId << 8) | internalIndex;
			uint32_t listenerId = systems.characterModel->addBoneListener(systems, entityId, svBlob.boneName, BoneUpdates::Group::BlobShadow, userId);
			if (listenerId == ~0u) continue;

			BoneShadow &shadow = blob.boneShadows.push();
			shadow.listenerId = listenerId;
			shadow.alpha = svBlob.alpha;
			shadow.radius = svBlob.radius;
			shadow.fadeDistance = svBlob.fadeDistance;
			shadow.offset = svBlob.offset;
		}

		systems.entities.addComponent(entityId, this, blobId, 0, componentIndex, Entity::UpdateTransform);
	}

	void updateTransform(Systems &systems, uint32_t entityId, const EntityComponent &ec, const TransformUpdate &update) override
	{
		// TODO (?)
	}

	void remove(Systems &systems, uint32_t entityId, const EntityComponent &ec) override
	{
		uint32_t blobId = ec.userId;
		BlobShadow &blob = blobShadows[blobId];

		for (BoneShadow &boneShadow : blob.boneShadows) {
			systems.characterModel->freeBoneListener(boneShadow.listenerId);
		}

		if (blob.areaId != ~0u) {
			systems.area->removeBoxArea(blob.areaId);
		}

		freeBlobShadowIds.push(blobId);
		sf::reset(blob);
	}

	void updatePositions(AreaSystem *areaSystem, const BoneUpdates &boneUpdates, const FrameArgs &frameArgs) override
	{
		uint64_t frameIndex = frameArgs.frameIndex;
		updatedBlobIds.clear();

		for (const BoneUpdates::Update &update : boneUpdates.updates[(uint32_t)BoneUpdates::Group::BlobShadow]) {
			uint32_t userId = update.userId;
			uint32_t blobId = userId >> 8;
			uint32_t boneIndex = userId & 0xff;

			BlobShadow &blob = blobShadows[blobId];
			BoneShadow &boneShadow = blob.boneShadows[boneIndex];
			boneShadow.updateFrame = frameIndex;
			boneShadow.position = sf::transformPoint(update.boneToWorld, boneShadow.offset);

			if (blob.updateFrame != frameIndex) {
				blob.updateFrame = frameIndex;
				updatedBlobIds.push(blobId);
			}
		}

		for (uint32_t blobId : updatedBlobIds) {
			BlobShadow &blob = blobShadows[blobId];

			sf::Vec2 aabbMin = sf::Vec2(+HUGE_VALF);
			sf::Vec2 aabbMax = sf::Vec2(-HUGE_VALF);

			for (BoneShadow &boneShadow : blob.boneShadows) {
				if (boneShadow.updateFrame != frameIndex) {
					boneShadow.active = false;
					continue;
				}

				float radius = boneShadow.radius;
				float alpha = boneShadow.alpha;
				float y = sf::max(0.0f, boneShadow.position.y);
				radius *= 1.0f + boneShadow.fadeDistance * y * 0.5f;
				alpha -= boneShadow.fadeDistance * y;
				if (alpha <= 0.0f) {
					boneShadow.active = false;
					continue;
				}

				boneShadow.active = true;
				boneShadow.renderPosition = boneShadow.position;
				boneShadow.renderPosition.y = 0.0f;
				boneShadow.renderRadius = radius;
				boneShadow.renderAlpha = alpha;

				float x = boneShadow.position.x, z = boneShadow.position.z;
				aabbMin = sf::min(sf::Vec2(x - radius, z - radius), aabbMin);
				aabbMax = sf::max(sf::Vec2(x + radius, z + radius), aabbMax);
			}

			if (aabbMin.x < aabbMax.x && aabbMin.y < aabbMax.y) {
				sf::Bounds3 bounds = sf::Bounds3::minMax(
					sf::Vec3(aabbMin.x, 0.0f, aabbMin.y),
					sf::Vec3(aabbMax.x, 0.0f, aabbMax.y));

				if (blob.areaId == ~0u) {
					blob.areaId = areaSystem->addBoxArea(AreaGroup::BlobShadow, blobId, bounds, Area::Visibility);
				} else {
					areaSystem->updateBoxArea(blob.areaId, bounds);
				}
			}
		}
	}

	void renderMain(const VisibleAreas &visibleAreas, const RenderArgs &renderArgs) override
	{
		shadowVertices.clear();

		for (uint32_t blobId : visibleAreas.get(AreaGroup::BlobShadow)) {
			const BlobShadow &blob = blobShadows[blobId];

			for (const BoneShadow &boneShadow : blob.boneShadows) {
				if (!boneShadow.active) continue;

				if (shadowVertices.size * 4 < MaxBlobShadowsPerFrame) {
					sf::Vec3 p = boneShadow.renderPosition + sf::Vec3(0.0f, 0.005f, 0.0f);
					float r = boneShadow.renderRadius;
					float a = boneShadow.renderAlpha;

					shadowVertices.push({ p + sf::Vec3(-r, 0.0f, -r), sf::Vec2(-1.0f, -1.0f), a });
					shadowVertices.push({ p + sf::Vec3(+r, 0.0f, -r), sf::Vec2(+1.0f, -1.0f), a });
					shadowVertices.push({ p + sf::Vec3(-r, 0.0f, +r), sf::Vec2(-1.0f, +1.0f), a });
					shadowVertices.push({ p + sf::Vec3(+r, 0.0f, +r), sf::Vec2(+1.0f, +1.0f), a });
				}

			}
		}

		if (shadowVertices.size > 0) {
			sg_update_buffer(shadowVertexBuffer.buffer, shadowVertices.data, (int)shadowVertices.byteSize());

			gameShaders.fakeShadowPipe.bind();

			FakeShadow_Vertex_t vu;
			renderArgs.worldToClip.writeColMajor44(vu.worldToClip);
			sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_FakeShadow_Vertex, &vu, sizeof(vu));

			sg_bindings binds = { };
			binds.vertex_buffers[0] = shadowVertexBuffer.buffer;
			binds.index_buffer = sp::getSharedQuadIndexBuffer();
			sg_apply_bindings(&binds);

			sg_draw(0, (int)(shadowVertices.size / 4) * 6, 1);
		}
	}

};

sf::Box<BlobShadowSystem> BlobShadowSystem::create() { return sf::box<BlobShadowSystemImp>(); }

}

