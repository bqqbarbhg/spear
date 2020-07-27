#pragma once

#include "ext/sokol/sokol_gfx.h"
#include "sf/Vector.h"
#include "sf/Symbol.h"

namespace sp {

struct PassTime
{
	sf::Symbol name;
	double time;
};

struct FramebufferDesc
{
	sg_pixel_format colorFormat = SG_PIXELFORMAT_NONE;
	sg_pixel_format depthFormat = SG_PIXELFORMAT_NONE;
	uint32_t msaaSamples = 1;

	bool operator==(const FramebufferDesc &rhs) const { return colorFormat == rhs.colorFormat && depthFormat == rhs.depthFormat && msaaSamples == rhs.msaaSamples; }
	bool operator!=(const FramebufferDesc &rhs) const { return !(*this == rhs); }
};

struct RenderTarget
{
	sf::Vec2i resolution;
	sg_pixel_format format = SG_PIXELFORMAT_NONE;
	uint32_t msaaSamples = 1;
	sg_image image = { };

	RenderTarget();
	RenderTarget(RenderTarget &&rhs);
	RenderTarget &operator=(RenderTarget &&rhs);
	RenderTarget(const RenderTarget &rhs) = delete;
	RenderTarget &operator=(const RenderTarget &rhs) = delete;
	~RenderTarget();

	void init(const char *label, const sf::Vec2i &resolution, sg_pixel_format format, uint32_t samples=1, const sg_image_desc &desc=sg_image_desc{});
};

struct RenderPass
{
	sf::Symbol name;
	sf::Vec2i resolution;
	FramebufferDesc desc;
	sg_pass pass = { };

	RenderPass();
	RenderPass(RenderPass &&rhs);
	RenderPass &operator=(RenderPass &&rhs);
	RenderPass(const RenderPass &rhs) = delete;
	RenderPass &operator=(const RenderPass &rhs) = delete;
	~RenderPass();

	void init(const sg_pass_desc &desc, const sf::Vec2i &resolution, const FramebufferDesc &fbDesc);
	void init(const char *label, sf::Slice<const RenderTarget*> targets);
	void init(const char *label, const RenderTarget &a);
	void init(const char *label, const RenderTarget &a, const RenderTarget &b);
	void init(const char *label, const RenderTarget &a, const RenderTarget &b, const RenderTarget &c);
	void init(const char *label, const RenderTarget &a, const RenderTarget &b, const RenderTarget &c, const RenderTarget &d);
};

enum PipelineFlag
{
	PipeDepthTest = 0x1,
	PipeDepthWrite = 0x2,
	PipeCullCCW = 0x4,
	PipeCullAuto = 0x8,
	PipeIndex16 = 0x10,
	PipeIndex32 = 0x20,
	PipeBlendOver = 0x40,
	PipeBlendPremultiply = 0x80,
	PipeBlendAdd = 0x100,
	PipeVertexFloat2 = 0x200,
};

struct PipelineImp
{
	sg_pipeline pipeline = { };
	uint32_t framebufferType = 0;
	uint32_t lastUse = 0;
};

struct Pipeline
{
	sg_pipeline_desc desc = { };
	PipelineImp imps[4] = { };
	uint32_t lastUse = 0;

	Pipeline();
	Pipeline(Pipeline &&rhs);
	Pipeline &operator=(Pipeline &&rhs);
	Pipeline(const Pipeline &rhs) = delete;
	Pipeline &operator=(const Pipeline &rhs) = delete;
	~Pipeline();

	sg_pipeline_desc &init(sg_shader shader, uint32_t flags);
	bool bind();

	void reset();
};

struct Buffer
{
	sg_buffer buffer = { };

	Buffer();
	Buffer(Buffer &&rhs);
	Buffer &operator=(Buffer &&rhs);
	Buffer(const Buffer &rhs) = delete;
	Buffer &operator=(const Buffer &rhs) = delete;
	~Buffer();

	void reset();
	void initVertex(const char *name, const void *data, size_t size);
	void initDynamicVertex(const char *name, size_t size);
	void initIndex(const char *name, const void *data, size_t size);

	template <typename T>
	void initVertex(const char *name, const sf::Slice<T> &data) {
		initVertex(name, data.data, data.size * sizeof(T));
	}

	template <typename T>
	void initIndex(const char *name, const sf::Slice<T> &data) {
		initIndex(name, data.data, data.size * sizeof(T));
	}
};

struct Texture
{
	sg_image image = { };

	Texture();
	Texture(Texture &&rhs);
	Texture &operator=(Texture &&rhs);
	Texture(const Texture &rhs) = delete;
	Texture &operator=(const Texture &rhs) = delete;
	~Texture();

	void init(const sg_image_desc &desc);

	void reset();
};

FramebufferDesc getFramebufferTypeDesc(uint32_t typeIndex);

void beginPass(const RenderPass &pass, const sg_pass_action *action);
void beginDefaultPass(uint32_t width, uint32_t height, const sg_pass_action *action);
void endPass();

void beginFrame();
void endFrame();

sf::Slice<const PassTime> getPassTimes();

extern sg_buffer g_hackSharedQuadIndexBuffer;
sf_inline sg_buffer getSharedQuadIndexBuffer() { return g_hackSharedQuadIndexBuffer; }

}
