#include "ParticleSystem.h"

#include "client/AreaSystem.h"

#include "sf/HashMap.h"
#include "sf/Frustum.h"
#include "sf/Random.h"
#include "sf/Float4.h"
#include "sf/Sort.h"

#include "sp/Renderer.h"

#include "client/ParticleTexture.h"
#include "client/BSpline.h"
#include "client/VisFogSystem.h"

#include "game/shader/GameShaders.h"
#include "game/shader/Particle.h"

#include "sp/Srgb.h"

namespace cl {

static const constexpr uint32_t MaxParticlesPerFrame = 4*1024;
static const constexpr uint32_t MaxParticleCacheFrames = 16;

static const constexpr float HugeParticleLife = 1e20f;
static const constexpr float HugeParticleLifeCmp = 1e19f;

struct ComponentKey
{
    void *ptr;
    
    bool operator==(const ComponentKey &rhs) const {
        return ptr == rhs.ptr;
    }
};

uint32_t hash(const ComponentKey &key) {
    return sf::hashPointer(key.ptr);
}

sf_inline float approxAcos(float a) {
	float x = sf::abs(a);
	float v = -1.280827681872808f + 1.280827681872808f*sf::sqrt(1.0f - x) - 0.28996864492208857f*x;
	return sf::F_PI*0.5f - sf::copysign(v, a);
}

sf_inline sf::Float4 approxCosSin2(float a, float b) {
	const sf::ScalarFloat4 rcp2pi = 1.0f / sf::F_2PI;
	const sf::Float4 bias = sf::Float4(0.0f, -0.25f, 0.0f, -0.25f);

	sf::Float4 t = sf::Float4(a, a, b, b);
	t *= rcp2pi;
	t += bias;
	t -= (t - 0.25f).round() + 0.25f;
	t *= (t.abs() - 0.5f) * 16.0f;
	t += t * (t.abs() - 1.0f) * 0.225f;
	return t;
}

sf_inline float approxCbrt(float a) {
	return 1.5142669123810535f*sf::sqrt(a) - 0.5142669123810535f*a;
}

struct RandomVec3Imp
{
	enum Flags {
		Box = 0x1,
		Sphere = 0x2,
		Rotation = 0x4,
	};

	uint32_t flags = 0;

	sf::Vec3 offset;

	// Box
	sf::Vec3 boxExtent;

	// Sphere
	float thetaBias, thetaScale;
	float cosPhiBias, cosPhiScale;
	float radiusCubeScale;
	float radiusScale;
	sf::Vec3 sphereScale;

	// Rotation
	sf::Quat rotation;

	void init(const sv::RandomVec3 &sv)
	{
		flags = 0;
		offset = sv.offset;

		if (sv.boxExtent.x != 0.0f || sv.boxExtent.y != 0.0f || sv.boxExtent.z != 0.0f) {
			flags |= Box;
			boxExtent = sv.boxExtent;
		}

		if (sv.sphere.maxRadius > 0.0f) {
			flags |= Sphere;
			thetaBias = sv.sphere.minTheta * (sf::F_PI/180.0f);
			thetaScale = (sv.sphere.maxTheta - sv.sphere.minTheta) * (sf::F_PI/180.0f);
			cosPhiBias = cosf(sv.sphere.maxPhi * (sf::F_PI/180.0f));
			cosPhiScale = cosf(sv.sphere.minPhi * (sf::F_PI/180.0f)) - cosPhiBias;
			radiusCubeScale = approxCbrt(1.0f - sv.sphere.minRadius / sv.sphere.maxRadius);
			radiusScale = sv.sphere.maxRadius;
			sphereScale = sv.sphere.scale;
		}

		if (sv.rotation != sf::Vec3(0.0f)) {
			flags |= Rotation;
			rotation = sf::eulerAnglesToQuat(sv.rotation * (sf::F_PI/180.0f));
		}
	}

	sf::Vec3 sample(sf::Random &rng) const
	{
		sf::Vec3 p = offset;

		if (flags & Box) {
			p += (rng.nextVec3() - sf::Vec3(0.5f)) * boxExtent;
		}
		if (flags & Sphere) {
			float u = rng.nextFloat();
			float v = rng.nextFloat();
			float w = rng.nextFloat();
			float theta = thetaBias + u * thetaScale;
			float phi = approxAcos(cosPhiBias + v * cosPhiScale);

			const sf::Float4 trig = approxCosSin2(theta, phi);
			const float cosTheta = trig.getX(), sinTheta = trig.getY();
			const float cosPhi = trig.getZ(), sinPhi = trig.getW();

			float radius = approxCbrt(1.0f - w * radiusCubeScale) * radiusScale;
			p += sf::Vec3(sinPhi * cosTheta, cosPhi, sinPhi * sinTheta) * radius * sphereScale;
		}

		if (flags & Rotation) {
			p = sf::rotate(rotation, p);
		}

		return p;
	}
};

struct ParticleSystemImp final : ParticleSystem
{
	struct EffectType
	{
		sf::Box<sv::ParticleSystemComponent> svComponent;

		double renderOrder = 0.0;
		uint32_t tiebreaker = 0;

		uint32_t refCount = 0;
		float updateRadius;
		float timeStep;

		RandomVec3Imp emitPosition;
		RandomVec3Imp emitVelocity;

		// TODO: Cache this?
		Particle_VertexType_t typeUbo;

		ParticleTextureRef texture;
	};

	struct Particle4
	{
		sf::Float4 px, py, pz;
		sf::Float4 vx, vy, vz;
		sf::Float4 life;
		sf::Float4 seed;
	};

	struct GpuParticle
	{
		sf::Vec3 position;
		float life;
		sf::Vec3 velocity;
		float seed;
	};

	struct Effect
	{
		uint32_t typeId;
		uint32_t areaId;

		sf::Vec3 origin;

		sf::Bounds3 gpuBounds;
		float timeDelta = 0.0f;
		float timeStep;

		sf::Vec3 gravity;
		double lastUpdateTime;

		sf::Float4 emitterToWorld[4];
		sf::Float4 prevEmitterToWorld[4];
		sf::Float4 prevWorldToEmitter[4];

		sf::Vec3 prevOrigin;
		sf::Mat34 particlesToWorld;

		bool stopEmit = false;
		bool firstEmit = true;
		bool instantDelete = false;
		float spawnTimer = 0.0f;
		float emitOnTimer = -1.0f;

		uint32_t uploadByteOffset = 0;
		uint64_t uploadFrameIndex = MaxParticleCacheFrames;
		sf::Array<Particle4> particles; // TODO: Alignment
		sf::Array<GpuParticle> gpuParticles;
		sf::Array<uint32_t> freeIndices;
	};

	struct ParticleUpdateCtx
	{
		sf::Random rng;
	};

	struct SortedEffect
	{
		double renderOrder;
		uint32_t tiebreaker;
		float depth;
		uint32_t effectId;

		bool operator<(const SortedEffect &rhs) const {
			if (renderOrder != rhs.renderOrder) return renderOrder < rhs.renderOrder;
			if (tiebreaker != rhs.tiebreaker) return tiebreaker < rhs.tiebreaker;
			if (depth != rhs.depth) return depth < rhs.depth;
			if (effectId != rhs.effectId) return effectId < rhs.effectId;
			return false;
		}
	};

	sf::Array<Effect> effects;
	sf::Array<uint32_t> freeEffectIds;

	sf::Array<EffectType> types;
	sf::Array<uint32_t> freeTypeIds;
	sf::HashMap<ComponentKey, uint32_t> svCompponentToType;

	sf::Array<SortedEffect> sortedEffects;

	sf::Random initRng;
	ParticleUpdateCtx updateCtx;

	static constexpr const uint32_t SplineSampleRate = 64;
	static constexpr const uint32_t SplineAtlasWidth = 8;
	static constexpr const uint32_t SplineAtlasHeight = 256;
	static constexpr const uint32_t SplineCountPerType = 2;
	static constexpr const uint32_t SplineAtlasResX = SplineSampleRate * SplineAtlasWidth;
	static constexpr const uint32_t SplineAtlasResY = SplineAtlasHeight * SplineCountPerType;
	sp::Texture splineTexture;

	sp::Buffer vertexBuffers[MaxParticleCacheFrames];
	sp::Pipeline particlePipe;
	uint64_t bufferFrameIndex = MaxParticleCacheFrames;

	ParticleSystemImp(const SystemsDesc &desc)
	{
		updateCtx.rng = sf::Random(desc.seed[1], 581271);

		for (uint32_t i = 0; i < MaxParticleCacheFrames; i++) {
			sf::SmallStringBuf<128> name;
			name.format("particle vertexBuffer %u", i);
			vertexBuffers[i].initDynamicVertex(name.data, sizeof(GpuParticle) * 4 * MaxParticlesPerFrame);
		}

		{
			uint32_t flags = sp::PipeIndex16|sp::PipeDepthTest|sp::PipeBlendPremultiply;
			sg_pipeline_desc &d = particlePipe.init(gameShaders.particle, flags);
			d.blend.src_factor_alpha = SG_BLENDFACTOR_ZERO;
			d.blend.dst_factor_alpha = SG_BLENDFACTOR_ONE;
			d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT4;
			d.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT4;
		}

		{
			sg_image_desc d = { };
			d.usage = SG_USAGE_IMMUTABLE;
			d.bqq_copy_target = true;
			d.pixel_format = SG_PIXELFORMAT_RGBA8;
			d.mag_filter = SG_FILTER_LINEAR;
			d.min_filter = SG_FILTER_LINEAR;
			d.width = SplineAtlasResX;
			d.height = SplineAtlasResY;
			d.label = "ParticleSystem splineTexture";
			splineTexture.init(d);
		}
	}

	void initEffectType(uint32_t typeId, const sv::ParticleSystemComponent &c)
	{
		EffectType &type = types[typeId];

		type.texture.load(c.texture);
		type.updateRadius = c.updateRadius;
		type.timeStep = c.timeStep;

		type.emitPosition.init(c.emitPosition);
		type.emitVelocity.init(c.emitVelocity);

		type.renderOrder = c.renderOrder;
		type.tiebreaker = typeId;

		sf::memZero(type.typeUbo);
		type.typeUbo.u_FrameCount = sf::Vec2(c.frameCount);
		type.typeUbo.u_FrameRate = c.frameRate;
		type.typeUbo.u_RotationControl.x = c.rotation * (sf::F_PI/180.0f);
		type.typeUbo.u_RotationControl.y = c.rotationVariance * (sf::F_PI/180.0f);
		type.typeUbo.u_RotationControl.z = c.spin * (sf::F_PI/180.0f);
		type.typeUbo.u_RotationControl.w = c.spinVariance * (sf::F_PI/180.0f);
		if (c.randomStartFrame) {
			type.typeUbo.u_StartFrame = 1.0f;
		}

		uint32_t atlasX = typeId / SplineAtlasHeight;
		uint32_t atlasY = typeId % SplineAtlasHeight;

		uint8_t splineTex[SplineSampleRate * 4 * 2];
		float splineY[SplineSampleRate];

		if (c.scaleSpline.points.size >= 1) {
			cl::discretizeBSplineY(splineY, c.scaleSpline.points, 0.0f, 1.0f);
		} else {
			for (float &v : splineY) v = 1.0f;
		}
		for (uint32_t i = 0; i < SplineSampleRate; i++) {
			splineTex[i * 4 + 0] = (uint8_t)sf::clamp((int)(splineY[i] * 255.0f), 0, 255);
		}

		if (c.alphaSpline.points.size >= 1) {
			cl::discretizeBSplineY(splineY, c.alphaSpline.points, 0.0f, 1.0f);
		} else {
			for (float &v : splineY) v = 1.0f;
		}
		for (uint32_t i = 0; i < SplineSampleRate; i++) {
			splineTex[i * 4 + 1] = (uint8_t)sf::clamp((int)(splineY[i] * 255.0f), 0, 255);
		}

		if (c.additiveSpline.points.size >= 1) {
			cl::discretizeBSplineY(splineY, c.additiveSpline.points, 0.0f, 1.0f);
			for (float &v : splineY) v = 1.0f - v;
		} else {
			for (float &v : splineY) v = 1.0f;
		}
		for (uint32_t i = 0; i < SplineSampleRate; i++) {
			splineTex[i * 4 + 2] = (uint8_t)sf::clamp((int)(splineY[i] * 255.0f), 0, 255);
		}

		if (c.erosionSpline.points.size >= 1) {
			cl::discretizeBSplineY(splineY, c.erosionSpline.points, 0.0f, 1.0f);
		} else {
			for (float &v : splineY) v = 1.0f;
		}
		for (uint32_t i = 0; i < SplineSampleRate; i++) {
			splineTex[i * 4 + 3] = (uint8_t)sf::clamp((int)(splineY[i] * 255.0f), 0, 255);
		}

		uint8_t *gradientTex = splineTex + SplineSampleRate * 4;
		sv::GradientPoint defaultGradient[] = {
			{ 1.0f, c.gradient.defaultColor },
		};

		{
			sf::Slice<const sv::GradientPoint> points = c.gradient.points;
			if (points.size == 0) {
				points = defaultGradient;
			}

			float splineStep = 1.0f / (SplineSampleRate - 1);
			uint32_t pointIx = 0;
			for (uint32_t i = 0; i < SplineSampleRate; i++) {
				float t = (float)i * splineStep;

				while (pointIx < points.size && t >= points[pointIx].t) {
					pointIx++;
				}

				sf::Vec3 col;
				if (pointIx == 0) {
					col = points[0].color;
				} else if (pointIx == points.size) {
					col = points[points.size - 1].color;
				} else {
					const sv::GradientPoint &p0 = points[pointIx - 1];
					const sv::GradientPoint &p1 = points[pointIx];
					float relT = (t - p0.t) / (p1.t - p0.t);
					col = sf::lerp(p0.color, p1.color, relT);
				}

				uint8_t *dst = gradientTex + i * 4;
				sp::linearToSrgbUnorm(dst, col);
			}
		}

		uint32_t x = atlasX * SplineSampleRate;
		uint32_t y = atlasY * SplineCountPerType;

		sf::Vec2 atlasRes = sf::Vec2((float)SplineAtlasResX, (float)SplineAtlasResY);
		sf::Vec2 uvBias = sf::Vec2((float)x + 0.5f, (float)y + 0.5f) / atlasRes;
		sf::Vec2 uvScale = sf::Vec2((float)(SplineSampleRate - 1), 1.0f) / atlasRes;

		type.typeUbo.u_SplineMad.x = uvScale.x;
		type.typeUbo.u_SplineMad.y = uvScale.y;
		type.typeUbo.u_SplineMad.z = uvBias.x;
		type.typeUbo.u_SplineMad.w = uvBias.y;

		type.typeUbo.u_ScaleBaseVariance.x = c.size;
		type.typeUbo.u_ScaleBaseVariance.y = c.sizeVariance;

		type.typeUbo.u_LifeTimeBaseVariance.x = c.lifeTime;
		type.typeUbo.u_LifeTimeBaseVariance.y = c.lifeTimeVariance * (1.0f / 16777216.0f);

		sg_image_desc d = { };
		d.width = SplineSampleRate;
		d.height = SplineCountPerType;
		d.pixel_format = SG_PIXELFORMAT_RGBA8;
		d.content.subimage[0][0].ptr = splineTex;
		d.content.subimage[0][0].size = sizeof(splineTex);
		sg_image staging = sg_make_image(&d);

		{
			sg_bqq_subimage_rect rect;
			rect.src_x = 0;
			rect.src_y = 0;
			rect.dst_x = (int)x;
			rect.dst_y = (int)y;
			rect.dst_z = 0;
			rect.width = (int)SplineSampleRate;
			rect.height = (int)SplineCountPerType;

			sg_bqq_subimage_copy_desc desc;
			desc.dst_image = splineTexture.image;
			desc.src_image = staging;
			desc.rects = &rect;
			desc.num_rects = 1;
			desc.num_mips = 1;

			sg_bqq_copy_subimages(&desc);
		}

		sg_destroy_image(staging);
	}

	void simulateParticlesImp(sf::Random &rng, Effect &effect, float dt)
	{
		EffectType &type = types[effect.typeId];
		const sv::ParticleSystemComponent &comp = *type.svComponent;

		bool firstEmit = effect.firstEmit;

		uint32_t burstAmount = 0;
		if (firstEmit) {
			memcpy(effect.prevEmitterToWorld, effect.emitterToWorld, sizeof(sf::Float4) * 4);
			burstAmount = comp.burstAmount;
			effect.firstEmit = false;
		}

		if (effect.emitOnTimer >= 0.0f) {
			effect.emitOnTimer -= dt;
			if (effect.emitOnTimer < 0.0f) {
				effect.stopEmit = true;
			}
		}

		sf::Float4 dt4 = dt;
		sf::ScalarFloat4 drag4 = comp.drag;

		// Update spawning (scalar)
		effect.spawnTimer -= dt;
		int spawnsLeft = 20;
		while ((effect.spawnTimer <= 0.0f && !effect.stopEmit) || burstAmount > 0) {
			if (burstAmount > 0) {
				burstAmount--;
			} else {
				if (spawnsLeft-- <= 0) break;
				effect.spawnTimer += comp.spawnTime + comp.spawnTimeVariance * rng.nextFloat();
			}

			if (effect.freeIndices.size == 0) {
				uint32_t base = effect.particles.size * 4;
				Particle4 &parts = effect.particles.push();
				parts.life = HugeParticleLife;
				for (uint32_t i = 0; i < 4; i++) {
					effect.freeIndices.push(base + i);
				}
			}

			uint32_t index = effect.freeIndices.popValue();

			sf::Vec3 pos = type.emitPosition.sample(rng);
			sf::Vec3 vel = type.emitVelocity.sample(rng);

			if (comp.emitVelocityAttractorStrength != 0.0f) {
				vel += sf::normalizeOrZero(pos - comp.emitVelocityAttractorOffset) * comp.emitVelocityAttractorStrength;
			}

			sf::ScalarFloat4 emitT = sf::clamp(-effect.spawnTimer / dt, 0.0f, 1.0f);
			sf::Float4 simdPos = effect.emitterToWorld[3] + (effect.prevEmitterToWorld[3] - effect.emitterToWorld[3]) * emitT;
			simdPos += (effect.emitterToWorld[0] + (effect.prevEmitterToWorld[0] - effect.emitterToWorld[0]) * emitT) * pos.x;
			simdPos += (effect.emitterToWorld[1] + (effect.prevEmitterToWorld[1] - effect.emitterToWorld[1]) * emitT) * pos.y;
			simdPos += (effect.emitterToWorld[2] + (effect.prevEmitterToWorld[2] - effect.emitterToWorld[2]) * emitT) * pos.z;

			pos = simdPos.asVec3();

			Particle4 &p = effect.particles[index >> 2];
			uint32_t lane = index & 3;
			sf::setLaneInMemory(p.px, lane, pos.x);
			sf::setLaneInMemory(p.py, lane, pos.y);
			sf::setLaneInMemory(p.pz, lane, pos.z);
			sf::setLaneInMemory(p.vx, lane, vel.x);
			sf::setLaneInMemory(p.vy, lane, vel.y);
			sf::setLaneInMemory(p.vz, lane, vel.z);
			sf::setLaneInMemory(p.life, lane, 1.0f);
			sf::setLaneInMemory(p.seed, lane, rng.nextFloat() * 16777216.0f);
		}
		effect.spawnTimer = sf::max(effect.spawnTimer, 0.0f);

		// Integration
		size_t numGpuParticles = effect.particles.size * (4 * 4);
		effect.gpuParticles.reserveGeometric(numGpuParticles);
		effect.gpuParticles.resizeUninit(numGpuParticles);
		GpuParticle *dst = effect.gpuParticles.data;

		sf::Float4 pMin = +HUGE_VALF, pMax = -HUGE_VALF;

		sf::ScalarAddFloat4 lifeTime = comp.lifeTime;
		sf::ScalarFloat4 lifeTimeVariance = comp.lifeTimeVariance * (1.0f / 16777216.0f);

		sf::Float4 rcpDt4 = 1.0f / dt;

		sf::SmallArray<sv::GravityPoint, 16> localGravityPoints;
		for (const sv::GravityPoint &p : comp.gravityPoints) {
			sv::GravityPoint &lp = localGravityPoints.push();

			sf::Float4 simdPos = effect.emitterToWorld[3] + (effect.prevEmitterToWorld[3] - effect.emitterToWorld[3]);
			simdPos += effect.emitterToWorld[0] * p.position.x;
			simdPos += effect.emitterToWorld[1] * p.position.y;
			simdPos += effect.emitterToWorld[2] * p.position.z;
			lp.position = simdPos.asVec3();
			lp.radius = sf::max(p.radius, 0.05f);
			lp.strength = -p.strength;
		}

		for (Particle4 &p : effect.particles) {

			sf::Float4 life = p.life;

			if (!life.allGreaterThanZero()) {
				uint32_t base = (uint32_t)(&p - effect.particles.data) * 4;
				float lifes[4];
				life.storeu(lifes);
				for (uint32_t i = 0; i < 4; i++) {
					if (lifes[i] <= 0.0f) {
						effect.freeIndices.push(base + i);
						lifes[i] = HugeParticleLife;
					}
				}
				life = sf::Float4::loadu(lifes);
			}

			sf::Float4 seed = p.seed;

			sf::Float4 px = p.px, py = p.py, pz = p.pz;
			sf::Float4 vx = p.vx, vy = p.vy, vz = p.vz;

			sf::Float4 ax = effect.gravity.x;
			sf::Float4 ay = effect.gravity.y;
			sf::Float4 az = effect.gravity.z;
			ax -= vx * drag4;
			ay -= vy * drag4;
			az -= vz * drag4;

			for (const sv::GravityPoint &p : localGravityPoints) {
				sf::Float4 dx = px - p.position.x;
				sf::Float4 dy = py - p.position.y;
				sf::Float4 dz = pz - p.position.z;
				sf::Float4 lenSq = (dx*dx + dy*dy + dz*dz) + p.radius;
				sf::Float4 weight = lenSq.rsqrt() / lenSq * p.strength;
				ax += dx * weight;
				ay += dy * weight;
				az += dz * weight;
			}

			vx += ax * dt4;
			vy += ay * dt4;
			vz += az * dt4;

			p.vx = vx; p.vy = vy; p.vz = vz;

			px += vx * dt4;
			py += vy * dt4;
			pz += vz * dt4;

			p.px = px; p.py = py; p.pz = pz;

			life -= dt4 / (seed * lifeTimeVariance + lifeTime);
			p.life = life;

			sf::Float4::transpose4(px, py, pz, life);

			sf::Float4::transpose4(vx, vy, vz, seed);

			if (px.getW() < HugeParticleLifeCmp) {
				px.storeu((float*)&dst[0] + 0);
				vx.storeu((float*)&dst[0] + 4);
				px.storeu((float*)&dst[1] + 0);
				vx.storeu((float*)&dst[1] + 4);
				px.storeu((float*)&dst[2] + 0);
				vx.storeu((float*)&dst[2] + 4);
				px.storeu((float*)&dst[3] + 0);
				vx.storeu((float*)&dst[3] + 4);
				pMin = pMin.min(px);
				pMax = pMax.max(px);
				dst += 4;
			}

			if (py.getW() < HugeParticleLifeCmp) {
				py.storeu((float*)&dst[0] + 0);
				vy.storeu((float*)&dst[0] + 4);
				py.storeu((float*)&dst[1] + 0);
				vy.storeu((float*)&dst[1] + 4);
				py.storeu((float*)&dst[2] + 0);
				vy.storeu((float*)&dst[2] + 4);
				py.storeu((float*)&dst[3] + 0);
				vy.storeu((float*)&dst[3] + 4);
				pMin = pMin.min(py);
				pMax = pMax.max(py);
				dst += 4;
			}

			if (pz.getW() < HugeParticleLifeCmp) {
				pz.storeu((float*)&dst[0] + 0);
				vz.storeu((float*)&dst[0] + 4);
				pz.storeu((float*)&dst[1] + 0);
				vz.storeu((float*)&dst[1] + 4);
				pz.storeu((float*)&dst[2] + 0);
				vz.storeu((float*)&dst[2] + 4);
				pz.storeu((float*)&dst[3] + 0);
				vz.storeu((float*)&dst[3] + 4);
				pMin = pMin.min(pz);
				pMax = pMax.max(pz);
				dst += 4;
			}

			if (life.getW() < HugeParticleLifeCmp) {
				life.storeu((float*)&dst[0] + 0);
				seed.storeu((float*)&dst[0] + 4);
				life.storeu((float*)&dst[1] + 0);
				seed.storeu((float*)&dst[1] + 4);
				life.storeu((float*)&dst[2] + 0);
				seed.storeu((float*)&dst[2] + 4);
				life.storeu((float*)&dst[3] + 0);
				seed.storeu((float*)&dst[3] + 4);
				pMin = pMin.min(life);
				pMax = pMax.max(life);
				dst += 4;
			}
		}

		memcpy(effect.prevEmitterToWorld, effect.emitterToWorld, sizeof(sf::Float4) * 4);

		effect.gpuParticles.resizeUninit(dst - effect.gpuParticles.data);

		effect.gpuBounds.origin = ((pMin + pMax) * 0.5f).asVec3();
		effect.gpuBounds.extent = ((pMax - pMin) * 0.5f + comp.cullPadding).asVec3();

		if (comp.localSpace) {
			effect.gpuBounds = sf::transformBounds(effect.particlesToWorld, effect.gpuBounds);
		}

		effect.uploadFrameIndex = 0;
	}

	void releaseEffectTypeImp(uint32_t typeId)
	{
		EffectType &type = types[typeId];
		if (--type.refCount == 0) {
			ComponentKey key = { (void*)type.svComponent.ptr };
			svCompponentToType.remove(key);

			sf::reset(type);
			freeTypeIds.push(typeId);
		}
	}

	// API

	virtual uint32_t reserveEffectType(Systems &systems, const sf::Box<sv::ParticleSystemComponent> &c) override
	{
		uint32_t typeId;
        ComponentKey key = { (void*)c.ptr };
		auto res = svCompponentToType.insert(key);
		if (res.inserted) {
			typeId = types.size;
			if (freeTypeIds.size > 0) {
				typeId = freeTypeIds.popValue();
			} else {
				types.push();
			}

			res.entry.val = typeId;
			types[typeId].svComponent = c;
			initEffectType(typeId, *c);
		} else {
			typeId = res.entry.val;
		}

		EffectType &type = types[typeId];
		type.refCount++;

		return typeId;
	}

	virtual void releaseEffectType(Systems &systems, const sf::Box<sv::ParticleSystemComponent> &c) override
	{
        ComponentKey key = { (void*)c.ptr };
		if (uint32_t *typeId = svCompponentToType.findValue(key)) {
			releaseEffectTypeImp(*typeId);
		}
	}

	virtual void addEffect(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sf::Box<sv::ParticleSystemComponent> &c, const Transform &transform) override
	{
		uint32_t effectId = effects.size;
		if (freeEffectIds.size > 0) {
			effectId = freeEffectIds.popValue();
		} else {
			effects.push();
		}

		uint32_t typeId = reserveEffectType(systems, c);
		EffectType &type = types[typeId];

		Effect &effect = effects[effectId];
		effect.typeId = typeId;
		effect.timeStep = type.timeStep * (1.0f + initRng.nextFloat() * 0.1f);

		effect.gravity = c->gravity;

		effect.prevOrigin = effect.origin = transform.position;

		effect.emitOnTimer = c->emitterOnTime;

		effect.instantDelete = c->instantDelete;

		sf::Mat34 entityToWorld = transform.asMatrix();
		if (type.svComponent->localSpace) {
			sf::Mat34().writeColMajor44((float*)effect.emitterToWorld);
			effect.particlesToWorld = entityToWorld;
		} else {
			entityToWorld.writeColMajor44((float*)effect.emitterToWorld);
		}

		sf::Sphere sphere = { transform.position, c->updateRadius };
		effect.areaId = systems.area->addSphereArea(AreaGroup::ParticleEffect, effectId, sphere, Area::Activate | Area::Visibility);

		systems.entities.addComponent(entityId, this, effectId, 0, componentIndex, Entity::UpdateTransform|Entity::PrepareForRemove);
	}

	void updateTransform(Systems &systems, uint32_t entityId, const EntityComponent &ec, const TransformUpdate &update) override
	{
		uint32_t effectId = ec.userId;
		Effect &effect = effects[effectId];
		EffectType &type = types[effect.typeId];

		if (type.svComponent->localSpace) {
			effect.particlesToWorld = update.entityToWorld;
		} else {
			update.entityToWorld.writeColMajor44((float*)effect.emitterToWorld);
		}

		effect.origin = update.transform.position;

		sf::Sphere sphere = { update.transform.position, type.updateRadius };
		systems.area->updateSphereArea(effect.areaId, sphere);
	}

	bool prepareForRemove(Systems &systems, uint32_t entityId, const EntityComponent &ec, const FrameArgs &frameArgs) override
	{
		uint32_t effectId = ec.userId;
		Effect &effect = effects[effectId];

		effect.stopEmit = true;

		return effect.gpuParticles.size == 0 || frameArgs.gameTime - effect.lastUpdateTime > 2.0 || effect.instantDelete;
	}

	void remove(Systems &systems, uint32_t entityId, const EntityComponent &ec) override
	{
		uint32_t effectId = ec.userId;
		Effect &effect = effects[effectId];

		systems.area->removeSphereArea(effect.areaId);
		releaseEffectTypeImp(effect.typeId);

		sf::reset(effect);
		freeEffectIds.push(effectId);
	}

	void updateParticles(const VisibleAreas &activeAreas, const FrameArgs &args) override
	{
		float clampedDt = sf::min(args.dt, 0.1f);

		for (uint32_t effectId : activeAreas.get(AreaGroup::ParticleEffect)) {
			Effect &effect = effects[effectId];
			effect.lastUpdateTime = args.gameTime;

			effect.timeDelta += clampedDt;
			while (effect.timeDelta >= effect.timeStep) {
				simulateParticlesImp(updateCtx.rng, effect, effect.timeStep);
				effect.timeDelta -= effect.timeStep;
			}
		}
	}

	void renderMain(const VisFogSystem *visFogSystem, const VisibleAreas &visibleAreas, const RenderArgs &renderArgs) override
	{
		// TODO: Sort by type
		uint32_t frameParticlesLeft = MaxParticlesPerFrame;

		VisFogImage visFog = visFogSystem->getVisFogImage();

		bufferFrameIndex++;
		uint64_t frame = bufferFrameIndex;

		sortedEffects.clear();
		for (uint32_t effectId : visibleAreas.get(AreaGroup::ParticleEffect)) {
			Effect &effect = effects[effectId];
			EffectType &type = types[effect.typeId];
			uint32_t numParticles = effect.gpuParticles.size / 4;
			if (numParticles == 0) continue;
			if (!renderArgs.frustum.intersects(effect.gpuBounds)) continue;
			SortedEffect &sorted = sortedEffects.push();
			sorted.renderOrder = type.renderOrder;
			sorted.tiebreaker = type.tiebreaker;
			sorted.depth = sf::lengthSq(renderArgs.cameraPosition - effect.origin);
			sorted.effectId = effectId;
		}

		sf::sort(sortedEffects);

		uint32_t prevTypeId = ~0u;
		for (SortedEffect &sorted : sortedEffects) {
			uint32_t effectId = sorted.effectId;

			Effect &effect = effects[effectId];
			EffectType &type = types[effect.typeId];
			uint32_t numParticles = effect.gpuParticles.size / 4;

			if (numParticles == 0) continue;
			if (!renderArgs.frustum.intersects(effect.gpuBounds)) continue;

			if (frame - effect.uploadFrameIndex >= MaxParticleCacheFrames) {
				if (numParticles >= frameParticlesLeft) return;

				frameParticlesLeft -= numParticles;
				effect.uploadByteOffset = (uint32_t)sg_append_buffer(vertexBuffers[(uint32_t)frame % MaxParticleCacheFrames].buffer, effect.gpuParticles.data, (int)effect.gpuParticles.byteSize());
				effect.uploadFrameIndex = frame;
			}

			sp::Buffer &vertexBuffer = vertexBuffers[(uint32_t)effect.uploadFrameIndex % MaxParticleCacheFrames];

			Particle_VertexInstance_t ubo;
			effect.particlesToWorld.writeColMajor44(ubo.u_ParticleToWorld);
			renderArgs.worldToClip.writeColMajor44(ubo.u_WorldToClip);
			ubo.u_Aspect = renderArgs.viewToClip.m00 / renderArgs.viewToClip.m11;
			ubo.u_InvDelta = effect.timeStep - effect.timeDelta;
			ubo.u_VisFogMad = visFog.worldMad;

			particlePipe.bind();

			if (effect.typeId != prevTypeId) {
				sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_Particle_VertexType, &type.typeUbo, sizeof(type.typeUbo));
				prevTypeId = effect.typeId;
			}

			sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_Particle_VertexInstance, &ubo, sizeof(ubo));

			sg_image image = ParticleTexture::defaultImage;
			if (type.texture.isLoaded() && type.texture->image.id) {
				image = type.texture->image;
			}

			sg_bindings binds = { };
			binds.vertex_buffers[0] = vertexBuffer.buffer;
			binds.vertex_buffer_offsets[0] = effect.uploadByteOffset;
			binds.index_buffer = sp::getSharedQuadIndexBuffer();
			binds.vs_images[SLOT_Particle_u_SplineTexture] = splineTexture.image;
			binds.vs_images[SLOT_Particle_u_VisFogTexture] = visFog.image;
			binds.fs_images[SLOT_Particle_u_Texture] = image;
			sg_apply_bindings(&binds);

			sg_draw(0, numParticles * 6, 1);
		}
	}

};

sf::Box<ParticleSystem> ParticleSystem::create(const SystemsDesc &desc) { return sf::box<ParticleSystemImp>(desc); }


}
