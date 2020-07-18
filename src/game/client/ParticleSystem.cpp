#include "ParticleSystem.h"

#include "sf/Array.h"
#include "sf/Float4.h"
#include "sf/Random.h"
#include "sf/Geometry.h"
#include "sf/Frustum.h"

#include "sp/Renderer.h"

#include "game/shader/GameShaders.h"
#include "game/shader/Particle.h"
#include "ext/sokol/sokol_gfx.h"

#include "sp/ContentFile.h"
#include "ext/sp_tools_common.h"

// TODO: Refactor, for getPixelFormatSuffix()
#include "game/client/MeshMaterial.h"

// TEMP HACK
#include "sf/Reflection.h"

namespace cl {

static const constexpr uint32_t MaxParticlesPerDraw = 4*1024;
static const constexpr uint32_t MaxParticlesPerFrame = 4*1024;
static const constexpr uint32_t MaxParticleCacheFrames = 16;
static const constexpr float HugeParticleLife = 1e20f;
static const constexpr float HugeParticleLifeCmp = 1e19f;

sg_pixel_format ParticleTexture::pixelFormat;
sg_image ParticleTexture::defaultImage;

struct ParticleTextureImp : ParticleTexture
{
	virtual void assetStartLoading() final;
	virtual void assetUnload() final;
};

sp::AssetType ParticleTexture::SelfType = { "ParticleTexture", sizeof(ParticleTextureImp), sizeof(ParticleTexture::PropType),
	[](Asset *a) { new ((ParticleTextureImp*)a) ParticleTextureImp(); }
};

static void loadTextureImp(void *user, const sp::ContentFile &file)
{
	ParticleTextureImp *imp = (ParticleTextureImp*)user;

	if (file.size > 0) {
		sptex_util su;
		sptex_util_init(&su, file.data, file.size);

		sptex_header header = sptex_decode_header(&su);

		sg_image_desc d = { };
		d.pixel_format = ParticleTexture::pixelFormat;
		d.num_mipmaps = header.info.num_mips;
		d.width = header.info.width;
		d.height = header.info.height;
		d.mag_filter = SG_FILTER_LINEAR;
		d.min_filter = SG_FILTER_LINEAR_MIPMAP_LINEAR;
		d.label = imp->name.data;

		// TODO: Config
		d.max_anisotropy = 4;

		uint32_t mipDrop = 0;

        // TODO: Mip drop
#if 0
		while (extent > atlas.textureExtent) {
			extent /= 2;
			mipDrop++;
		}
		sf_assert(extent == atlas.textureExtent);
#endif

		for (uint32_t mipI = 0; mipI < (uint32_t)d.num_mipmaps; mipI++) {
			d.content.subimage[0][mipI].ptr = sptex_decode_mip(&su, mipDrop + mipI);
			d.content.subimage[0][mipI].size = header.s_mips[mipDrop + mipI].uncompressed_size;
		}

        imp->image = sg_make_image(&d);

		spfile_util_free(&su.file);
	}

	imp->assetFinishLoading();
}

void ParticleTextureImp::assetStartLoading()
{
	sf::SmallStringBuf<256> path;

    path.clear(); path.format("%s.%s.sptex", name.data, getPixelFormatSuffix(ParticleTexture::pixelFormat));
	sp::ContentFile::loadMainThread(path, &loadTextureImp, this);
}

void ParticleTextureImp::assetUnload()
{
	if (image.id != 0) {
		sg_destroy_image(image);
		image.id = 0;
	}
}


void ParticleTexture::globalInit()
{
    sg_pixel_format formats[] = {
        SG_PIXELFORMAT_BQQ_BC3_SRGB,
        SG_PIXELFORMAT_BQQ_ASTC_4X4_SRGB,
        SG_PIXELFORMAT_BQQ_SRGBA8,
    };

    for (sg_pixel_format format : formats) {
        if (!sg_query_pixelformat(format).sample) continue;
        ParticleTexture::pixelFormat = format;
        break;
    }

    {
        sg_image_desc desc = { };
		desc.width = 1;
		desc.height = 1;
		desc.num_mipmaps = 1;
		desc.mag_filter = SG_FILTER_LINEAR;
		desc.min_filter = SG_FILTER_LINEAR_MIPMAP_LINEAR;
		desc.max_anisotropy = 4;

		{
			const unsigned char content[] = { 0xff, 0xff, 0xff, 0xff };
			desc.pixel_format = SG_PIXELFORMAT_BQQ_SRGBA8;
            desc.content.subimage[0][0].ptr = content;
            desc.content.subimage[0][0].size = sizeof(content);
			ParticleTexture::defaultImage = sg_make_image(&desc);
		}

    }
}

void ParticleTexture::globalCleanup()
{
	sg_destroy_image(ParticleTexture::defaultImage);
}

sf::Vec3 RandomVec3::sample(sf::Random &rng) const
{
	sf::Vec3 p = origin;

	if ((boxExtent.x != 0.0f) | (boxExtent.y != 0.0f) | (boxExtent.z != 0.0f)) {
		p += boxExtent * (rng.nextVec3() - sf::Vec3(0.5f));
	}

	if (sphereMaxRadius > 0.0f) {
		float u = rng.nextFloat();
		float v = rng.nextFloat();
		float w = rng.nextFloat();
		float theta = u * sf::F_2PI;
		float phi = acosf(2.0f * v - 1.0f);
		float cosTheta = cosf(theta), sinTheta = sinf(theta);
		float cosPhi = cosf(phi), sinPhi = sinf(phi);
		float radius = cbrtf(1.0f - w * (1.0f - sphereMinRadius / sphereMaxRadius)) * sphereMaxRadius;
		p += sf::Vec3(sinPhi * cosTheta, sinPhi * sinTheta, cosPhi) * radius;
	}

	return p;
}

sf::Vec4 ParticleFloat::packShader() const
{
	float rcpIn = fadeIn > 0.0f ? 1.0f / fadeIn : 1e18f;
	float rcpOut = fadeOut > 0.0f ? 1.0f / fadeOut : 1e18f;
	return { base, variance, rcpIn, rcpOut };
}

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
	uint32_t seed;
};

struct ParticleContext
{
	sp::Buffer vertexBuffers[MaxParticleCacheFrames];
	sp::Buffer indexBuffer;
	sp::Pipeline particlePipe;
	uint64_t frameIndex = MaxParticleCacheFrames;
	uint32_t frameParticlesLeft = MaxParticlesPerFrame;
};

ParticleContext g_particleContext;

struct ParticleSystemImp : ParticleSystem
{
	sf::Array<Particle4> particles;
	sf::Array<GpuParticle> gpuParticles;
	sf::Array<uint32_t> freeIndices;
	float spawnTimer = 0.0f;

	float timeDelta = 0.0f;
	uint64_t uploadedFrameIndex = 0;
	uint32_t uploadedByteOffset = 0;

	sf::Bounds3 gpuBounds;

	void simulate(float dt, sf::Random &rng)
	{
		sf::ScalarFloat4 dt4 = dt;
		sf::ScalarFloat4 drag4 = drag;

		spawnTimer -= dt;
		int spawnsLeft = 10;
		while (spawnTimer <= 0.0f) {
			if (spawnsLeft-- <= 0) break;
			spawnTimer += spawnTime + spawnTimeVariance * rng.nextFloat();

			if (freeIndices.size == 0) {
				uint32_t base = particles.size * 4;
				Particle4 &parts = particles.push();
				memset(&parts, 0, sizeof(parts));
				for (uint32_t i = 0; i < 4; i++) {
					freeIndices.push(base + i);
				}
			}

			uint32_t index = freeIndices.popValue();

			sf::Vec3 pos = spawnPosition.sample(rng);
			sf::Vec3 vel = spawnVelocity.sample(rng);

			Particle4 &p = particles[index >> 2];
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
		spawnTimer = sf::max(spawnTimer, 0.0f);

		size_t numGpuParticles = particles.size * (4 * 4);
		gpuParticles.reserveGeometric(numGpuParticles);
		gpuParticles.resizeUninit(numGpuParticles);
		GpuParticle *dst = gpuParticles.data;

		sf::Float4 pMin = +HUGE_VALF, pMax = -HUGE_VALF;

		for (Particle4 &p : particles) {

			sf::Float4 life = p.life;

			if (!life.allGreaterThanZero()) {
				uint32_t base = (uint32_t)(&p - particles.data) * 4;
				float lifes[4];
				life.storeu(lifes);
				for (uint32_t i = 0; i < 4; i++) {
					if (lifes[i] <= 0.0f) {
						freeIndices.push(base + i);
						lifes[i] = HugeParticleLife;
					}
				}
				life = sf::Float4::loadu(lifes);
			}

			life -= dt4;
			p.life = life;

			sf::Float4 px = p.px, py = p.py, pz = p.pz;
			sf::Float4 vx = p.vx, vy = p.vy, vz = p.vz;

			sf::Float4 ax = gravity.x;
			sf::Float4 ay = gravity.y;
			sf::Float4 az = gravity.z;
			ax -= vx * drag4;
			ay -= vy * drag4;
			az -= vz * drag4;

			vx += ax * dt4;
			vy += ay * dt4;
			vz += az * dt4;

			p.vx = vx; p.vy = vy; p.vz = vz;

			px += vx * dt4;
			py += vy * dt4;
			pz += vz * dt4;

			p.px = px; p.py = py; p.pz = pz;

			sf::Float4::transpose4(px, py, pz, life);

			sf::Float4 seed = p.seed;
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

		gpuParticles.resizeUninit(dst - gpuParticles.data);

		gpuBounds.origin = ((pMin + pMax) * 0.5f).asVec3();
		gpuBounds.extent = ((pMax - pMin) * 0.5f + cullPadding).asVec3();
		uploadedFrameIndex = 0;
	}

	void update(float dt, sf::Random &rng)
	{
		if (dt > 1.0f) dt = 1.0f;

		bool updated = false;
		timeDelta += dt;
		while (timeDelta >= timeStep) {
			updated = true;
			simulate(timeStep, rng);
			timeDelta -= timeStep;
		}
	}

	void render(const sf::Mat34 &worldToView, const sf::Mat44 &viewToClip, const sf::Frustum &frustum)
	{
		uint32_t numParticles = gpuParticles.size / 4;
		if (numParticles == 0) return;

		if (!frustum.intersects(gpuBounds)) return;

		ParticleContext &ctx = g_particleContext;
		uint64_t frame = ctx.frameIndex;

		if (frame - uploadedFrameIndex >= MaxParticleCacheFrames) {
			if (numParticles > ctx.frameParticlesLeft) return;

			ctx.frameParticlesLeft -= numParticles;
			uploadedByteOffset = (uint32_t)sg_append_buffer(ctx.vertexBuffers[frame % MaxParticleCacheFrames].buffer, gpuParticles.data, (int)gpuParticles.byteSize());
			uploadedFrameIndex = frame;
		}

		sp::Buffer &vertexBuffer = ctx.vertexBuffers[uploadedFrameIndex % MaxParticleCacheFrames];

		sf::Mat44 worldToClip = viewToClip * worldToView;

		Particle_Vertex_t ubo;
		worldToClip.writeColMajor44(ubo.u_WorldToClip);
		ubo.u_InvDelta = timeStep - timeDelta;
		ubo.u_FrameCount = sf::Vec2(frameCount);
		ubo.u_FrameRate = frameRate;
		ubo.u_Aspect = viewToClip.m00 / viewToClip.m11;
		ubo.u_ScaleAnim = scaleAnim.packShader();
		ubo.u_AlphaAnim = alphaAnim.packShader();
		ubo.u_RotationControl = sf::Vec4(angle, angleVariance, spin, spinVariance) * (sf::F_PI / 180.0f);
		ubo.u_Additive = additive;
		ubo.u_StartFrame = randomStartFrame ? 1.0f : 0.0f;

		g_particleContext.particlePipe.bind();

		sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_Particle_Vertex, &ubo, sizeof(ubo));

		sg_image image = ParticleTexture::defaultImage;
		if (texture.isLoaded() && texture->image.id) {
			image = texture->image;
		}

		sg_bindings binds = { };
		binds.vertex_buffers[0] = vertexBuffer.buffer;
		binds.vertex_buffer_offsets[0] = uploadedByteOffset;
		binds.index_buffer = g_particleContext.indexBuffer.buffer;
		binds.fs_images[SLOT_Particle_u_Texture] = image;
		sg_apply_bindings(&binds);

		sg_draw(0, numParticles * 6, 1);
	}
};

void ParticleSystem::update(float dt, sf::Random &rng)
{
	((ParticleSystemImp*)this)->update(dt, rng);
}

void ParticleSystem::render(const sf::Mat34 &worldToView, const sf::Mat44 &viewToClip, const sf::Frustum &frustum)
{
	((ParticleSystemImp*)this)->render(worldToView, viewToClip, frustum);
}

ParticleSystem *ParticleSystem::create()
{
	return new ParticleSystemImp();
}

void ParticleSystem::free(ParticleSystem *s)
{
	ParticleSystemImp *imp = (ParticleSystemImp*)s;
	delete imp;
}

void ParticleSystem::globalInit()
{
	sf::Array<uint16_t> indices;
	indices.resizeUninit(MaxParticlesPerDraw * 6);
	uint16_t *dst = indices.data;
	for (uint32_t i = 0; i < MaxParticlesPerDraw; i++) {
		uint32_t vi = i * 4;
		dst[0] = (uint16_t)(vi + 0);
		dst[1] = (uint16_t)(vi + 1);
		dst[2] = (uint16_t)(vi + 2);
		dst[3] = (uint16_t)(vi + 2);
		dst[4] = (uint16_t)(vi + 1);
		dst[5] = (uint16_t)(vi + 3);
		dst += 6;
	}

	g_particleContext.indexBuffer.initIndex("particle indexBuffer", indices.slice());

	for (uint32_t i = 0; i < MaxParticleCacheFrames; i++) {
		sf::SmallStringBuf<128> name;
		name.format("particle vertexBuffer %u", i);
		g_particleContext.vertexBuffers[i].initDynamicVertex(name.data, sizeof(GpuParticle) * 4 * MaxParticlesPerFrame);
	}

	{
		uint32_t flags = sp::PipeIndex16|sp::PipeDepthTest|sp::PipeBlendPremultiply;
		sg_pipeline_desc &d = g_particleContext.particlePipe.init(gameShaders.particle, flags);
		d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT4;
		d.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT4;
	}
}

void ParticleSystem::globalCleanup()
{
	g_particleContext = ParticleContext();
}

void ParticleSystem::globalUpdate()
{
	g_particleContext.frameIndex++;
	g_particleContext.frameParticlesLeft = MaxParticlesPerFrame;
}

}

// TEMP HACK

namespace sf {

template<>
void initType<cl::RandomVec3>(Type *t)
{
	static Field fields[] = {
		sf_field(cl::RandomVec3, origin),
		sf_field(cl::RandomVec3, boxExtent),
		sf_field(cl::RandomVec3, sphereMinRadius),
		sf_field(cl::RandomVec3, sphereMaxRadius),
	};
	sf_struct(t, cl::RandomVec3, fields);
}

template<>
void initType<cl::ParticleFloat>(Type *t)
{
	static Field fields[] = {
		sf_field(cl::ParticleFloat, base),
		sf_field(cl::ParticleFloat, variance),
		sf_field(cl::ParticleFloat, fadeIn),
		sf_field(cl::ParticleFloat, fadeOut),
	};
	sf_struct(t, cl::ParticleFloat, fields);
}

template<>
void initType<cl::ParticleSystem>(Type *t)
{
	static Field fields[] = {
		sf_field(cl::ParticleSystem, spawnPosition),
		sf_field(cl::ParticleSystem, spawnVelocity),
		sf_field(cl::ParticleSystem, gravity),
		sf_field(cl::ParticleSystem, drag),
		sf_field(cl::ParticleSystem, spawnTime),
		sf_field(cl::ParticleSystem, spawnTimeVariance),
		sf_field(cl::ParticleSystem, cullPadding),
		sf_field(cl::ParticleSystem, timeStep),
		sf_field(cl::ParticleSystem, frameCount),
		sf_field(cl::ParticleSystem, frameRate),
		sf_field(cl::ParticleSystem, alphaAnim),
		sf_field(cl::ParticleSystem, scaleAnim),
		sf_field(cl::ParticleSystem, angle),
		sf_field(cl::ParticleSystem, angleVariance),
		sf_field(cl::ParticleSystem, spin),
		sf_field(cl::ParticleSystem, spinVariance),
		sf_field(cl::ParticleSystem, additive),
		sf_field(cl::ParticleSystem, randomStartFrame),
	};
	sf_struct(t, cl::ParticleSystem, fields);
}

}
