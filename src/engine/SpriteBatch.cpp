#include "SpriteBatch.h"

#include "ext/sokol/sokol_gfx.h"

#include "sf/Array.h"

static const uint32_t MaxQuadsPerDraw = 1024;

struct Vertex
{
	sf::Vec2 pos;
	uint16_t uv[2];
	uint32_t color;
};

struct Quad
{
	Vertex v[4];
};

static const char *shaderVsGlsl = R"(
precision mediump float;

attribute vec2 aPosition;
attribute vec2 aTexCoord;
attribute vec4 aColor;

varying vec2 vTexCoord;
varying vec4 vColor;

void main()
{
	gl_Position = vec4(aPosition.x, aPosition.y, 0.5, 1.0);
	vTexCoord = aTexCoord;
	vColor = aColor;
}
)";

static const char *shaderFsGlsl = R"(
precision mediump float;

uniform sampler2D uTexture;

varying vec2 vTexCoord;
varying vec4 vColor;

void main()
{
	vec4 color = texture2D(uTexture, vTexCoord);
	gl_FragColor = color * vColor;
}
)";


static const char *shaderVsHlsl = R"(
struct VS_INPUT
{
	float2 aPosition : POSITION;
	float2 aTexCoord : TEXCOORD0;
	float4 aColor    : TEXCOORD1;
};

struct VS_OUTPUT
{
	float2 vTexCoord : TEXCOORD0;
	float4 vColor    : TEXCOORD1;
	float4 vPosition : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.vPosition = float4(input.aPosition.x, input.aPosition.y, 0.5, 1.0);
	output.vTexCoord = input.aTexCoord;
	output.vColor = input.aColor;
	return output;
}
)";

static const char *shaderFsHlsl = R"(
Texture2D<float4> uTexture : register(t0);
sampler uSampler : register(s0);

struct FS_INPUT
{
	float2 vTexCoord : TEXCOORD0;
	float4 vColor    : TEXCOORD1;
};

float4 main(FS_INPUT input): SV_Target0
{
	float4 color = uTexture.Sample(uSampler, input.vTexCoord);
	return color * input.vColor;
}
)";

struct SpriteDraw
{
	Sprite sprite;
	sf::Mat23 transform;
	sf::Vec4 color;
};

struct SpriteBatch::Data
{
	sf::Array<SpriteDraw> draws;
	uint32_t imageIndex = 0;
	sf::Array<Quad> quads;
	sg_buffer vertexBuffer;
	sg_buffer indexBuffer;
	sg_shader shader;
	sg_pipeline pipeline;
	sg_bindings bindings = { };

	Data()
	{
		{
			sg_shader_desc desc = { };

			desc.attrs[0].name = "aPosition";
			desc.attrs[0].sem_name = "POSITION";
			desc.attrs[0].sem_index = 0;
			desc.attrs[1].name = "aTexCoord";
			desc.attrs[1].sem_name = "TEXCOORD";
			desc.attrs[1].sem_index = 0;
			desc.attrs[2].name = "aColor";
			desc.attrs[2].sem_name = "TEXCOORD";
			desc.attrs[2].sem_index = 1;
			desc.fs.images[0].name = "uTexture";
			desc.fs.images[0].type = SG_IMAGETYPE_2D;
			desc.label = "SpriteBatch";

			if (sg_query_backend() == SG_BACKEND_D3D11) {
				desc.vs.source = shaderVsHlsl;
				desc.fs.source = shaderFsHlsl;
			} else {
				desc.vs.source = shaderVsGlsl;
				desc.fs.source = shaderFsGlsl;
			}

			shader = sg_make_shader(&desc);
		}

		{
			sg_pipeline_desc desc = { };
			desc.shader = shader;
			desc.layout.buffers[0].stride = sizeof(Vertex);
			desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
			desc.layout.attrs[0].offset = 0;
			desc.layout.attrs[1].format = SG_VERTEXFORMAT_USHORT2N;
			desc.layout.attrs[1].offset = 8;
			desc.layout.attrs[2].format = SG_VERTEXFORMAT_UBYTE4N;
			desc.layout.attrs[2].offset = 12;
			desc.index_type = SG_INDEXTYPE_UINT16;
			desc.blend.enabled = true;
			desc.blend.src_factor_rgb = desc.blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
			desc.blend.dst_factor_rgb = desc.blend.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
			desc.label = "SpriteBatch";
			pipeline = sg_make_pipeline(&desc);
		}

		{
			sg_buffer_desc desc = { };
			desc.size = MaxQuadsPerDraw * sizeof(Quad);
			desc.type = SG_BUFFERTYPE_VERTEXBUFFER;
			desc.usage = SG_USAGE_STREAM;
			desc.label = "SpriteBatch VertexBuffer";
			vertexBuffer = sg_make_buffer(&desc);
		}

		{
			sf::Array<uint16_t> indices;
			indices.resizeUninit(MaxQuadsPerDraw * 6);
			for (uint32_t quad = 0; quad < MaxQuadsPerDraw; quad++) {
				uint32_t base = quad * 4;
				uint16_t *dst = indices.data + quad * 6;
				dst[0] = (uint16_t)(base + 0);
				dst[1] = (uint16_t)(base + 2);
				dst[2] = (uint16_t)(base + 1);
				dst[3] = (uint16_t)(base + 1);
				dst[4] = (uint16_t)(base + 2);
				dst[5] = (uint16_t)(base + 3);
			}

			sg_buffer_desc desc = { };
			desc.size = MaxQuadsPerDraw * sizeof(uint16_t) * 6;
			desc.type = SG_BUFFERTYPE_INDEXBUFFER;
			desc.usage = SG_USAGE_IMMUTABLE;
			desc.label = "SpriteBatch IndexBuffer";
			desc.content = indices.data;
			indexBuffer = sg_make_buffer(&desc);
		}

		bindings.vertex_buffers[0] = vertexBuffer;
		bindings.index_buffer = indexBuffer;
	}

	~Data()
	{
		sg_destroy_shader(shader);
		sg_destroy_pipeline(pipeline);
		sg_destroy_buffer(vertexBuffer);
		sg_destroy_buffer(indexBuffer);
	}

	void flush()
	{
		if (quads.size == 0) return;

		int offset = sg_append_buffer(vertexBuffer, quads.data, quads.byteSize());
		bindings.vertex_buffer_offsets[0] = offset;
		bindings.fs_images[0].id = imageIndex;

		sg_apply_pipeline(pipeline);
		sg_apply_bindings(&bindings);
		sg_draw(0, quads.size * 6, 1);

		quads.clear();
	}
};

SpriteBatch::SpriteBatch()
	: data(new Data())
{
}

SpriteBatch::~SpriteBatch()
{
	delete data;
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

static void packUv(uint16_t *dst, sf::Vec2 v)
{
	dst[0] = (uint16_t)sf::clamp((uint32_t)(v.x * 65535.0f), 0u, 65535u);
	dst[1] = (uint16_t)sf::clamp((uint32_t)(v.y * 65535.0f), 0u, 65535u);
}

void SpriteBatch::draw(Sprite sprite, const sf::Mat23 &transform, const sf::Vec4 &color)
{
	SpriteInfo info;
	if (!getSpriteInfo(sprite, info)) return;

	if (info.imageIndex != data->imageIndex || data->quads.size >= MaxQuadsPerDraw) {
		data->flush();
		data->imageIndex = info.imageIndex;
	}

	uint32_t packedCol = packColor(color);

	uint16_t uvMin[2], uvMax[2];
	packUv(uvMin, info.uvMin);
	packUv(uvMax, info.uvMax);

	Quad &quad = data->quads.push();

	quad.v[0].pos.x = transform.m02;
	quad.v[0].pos.y = transform.m12;
	quad.v[0].uv[0] = uvMin[0];
	quad.v[0].uv[1] = uvMin[1];
	quad.v[0].color = packedCol;

	quad.v[1].pos.x = transform.m02 + transform.m00;
	quad.v[1].pos.y = transform.m12 + transform.m10;
	quad.v[1].uv[0] = uvMax[0];
	quad.v[1].uv[1] = uvMin[1];
	quad.v[1].color = packedCol;

	quad.v[2].pos.x = transform.m02 + transform.m01;
	quad.v[2].pos.y = transform.m12 + transform.m11;
	quad.v[2].uv[0] = uvMin[0];
	quad.v[2].uv[1] = uvMax[1];
	quad.v[2].color = packedCol;

	quad.v[3].pos.x = transform.m02 + transform.m00 + transform.m01;
	quad.v[3].pos.y = transform.m12 + transform.m10 + transform.m11;
	quad.v[3].uv[0] = uvMax[0];
	quad.v[3].uv[1] = uvMax[1];
	quad.v[3].color = packedCol;
}

void SpriteBatch::end()
{
	data->flush();
	data->imageIndex = 0;
}
