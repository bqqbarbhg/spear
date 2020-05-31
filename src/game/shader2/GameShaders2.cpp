#include "GameShaders2.h"
#include "ext/sokol/sokol_gfx.h"

#include "ext/sokol/sokol_config.h"
#include "ext/zstd.h"

#if defined(SOKOL_GLCORE33) || defined(SOKOL_DUMMY_BACKEND)
	#include "GameShadersImp_glsl.h"
#elif defined(SOKOL_GLES3)
	#include "GameShadersImp_gles.h"
#elif defined(SOKOL_D3D11)
	#include "GameShadersImp_hlsl.h"
#elif defined(SOKOL_METAL) && (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)
	#include "GameShadersImp_ios.h"
#elif defined(SOKOL_METAL)
	#include "GameShadersImp_macos.h"
#endif

static_assert(SP_MAX_UNIFORMS_PER_SHADER <= SG_MAX_SHADERSTAGE_UBS, "Too many uniforms");

static char *uncompressedShaderData;

static uint16_t getShaderStage2(sg_shader_stage_desc &d, const SpShaderStageInfo &info, sf::Slice<const uint8_t> permutations)
{
	uint32_t permIndex = 0;
	uint32_t prevNum = 0;
	for (uint32_t i = 0; i < info.permutationCount; i++) {
		SpPermutationMapping mapping = info.permutationMapping[i];
		if (mapping.numValues == 0) break;
		if (prevNum > 1) permIndex *= prevNum;
		uint32_t value = permutations[mapping.sharedIndex];
		sf_assert(value < mapping.numValues);
		permIndex += value;
		prevNum = mapping.numValues;
	}

	const SpPermutationInfo &permInfo = spPermutations[info.permutationOffset + permIndex];
	d.entry = "main";
	d.source = uncompressedShaderData + permInfo.dataOffset;

	for (uint32_t i = 0; i < SP_MAX_UNIFORMS_PER_SHADER; i++) {
		if (!permInfo.uniformBlocks[i]) break;
		const SpUniformBlockInfo &uboInfo = spUniformBlocks[permInfo.uniformBlocks[i]];
		d.uniform_blocks[i].uniforms[0].name = uboInfo.name;
		d.uniform_blocks[i].uniforms[0].type = (sg_uniform_type)0x214f4255;
		d.uniform_blocks[i].size = uboInfo.size;
	}

	for (uint32_t i = 0; i < SP_MAX_SAMPLERS_PER_SHADER; i++) {
		if (!permInfo.samplers[i]) break;
		const SpSamplerInfo &samplerInfo = spSamplers[permInfo.samplers[i]];
		d.images[i].name = samplerInfo.name;
		d.images[i].type = (sg_image_type)samplerInfo.type;
	}

	return info.permutationOffset + permIndex;
}

static Shader2 shaderCache[sf_arraysize(spPermutations)];

Shader2 getShader2(uint32_t index, sf::Slice<const uint8_t> permutations)
{
	sf_assert(index < sf_arraysize(spShaders));
	Shader2 &shader = shaderCache[index];
	if (shader.handle.id) return shader;

	if (!uncompressedShaderData) {
		uncompressedShaderData = (char*)sf::memAlloc(SpShaderDataSize);
		ZSTD_decompress(uncompressedShaderData, SpShaderDataSize, spShaderData, sizeof(spShaderData) - 1);
	}

	const SpShaderInfo &shaderInfo = spShaders[index];

	sg_shader_desc d = { };
	d.label = shaderInfo.name;

	shader.vsIndex = getShaderStage2(d.vs, shaderInfo.stages[0], permutations);
	shader.fsIndex = getShaderStage2(d.fs, shaderInfo.stages[1], permutations);

	{
		const SpPermutationInfo &permInfo = spPermutations[shader.vsIndex];

		for (uint32_t i = 0; i < SP_MAX_ATTRIBS_PER_SHADER; i++) {
			if (!permInfo.attribs[i]) break;
			const SpAttribInfo &attribInfo = spAttribs[permInfo.attribs[i]];
			d.attrs[i].name = attribInfo.name;
			d.attrs[i].sem_name = "TEXCOORD";
			d.attrs[i].sem_index = attribInfo.location;
		}
	}

	shader.handle = sg_make_shader(&d);

	return shader;
}

void bindImageVS(const Shader2 &shader, sg_bindings &bnd, uint32_t index, sg_image image)
{
	const SpPermutationInfo &info = spPermutations[shader.vsIndex];
	uint32_t loc = 0;
	for (uint16_t ix : info.samplers) {
		if (ix == 0) break;
		if (ix == index) {
			bnd.vs_images[loc] = image;
			break;
		}
		loc++;
	}
}

void bindImageFS(const Shader2 &shader, sg_bindings &bnd, uint32_t index, sg_image image)
{
	const SpPermutationInfo &info = spPermutations[shader.fsIndex];
	uint32_t loc = 0;
	for (uint16_t ix : info.samplers) {
		if (ix == 0) break;
		if (ix == index) {
			bnd.fs_images[loc] = image;
			break;
		}
		loc++;
	}
}

void bindImage(const Shader2 &shader, sg_bindings &bnd, uint32_t index, sg_image image)
{
	bindImageVS(shader, bnd, index, image);
	bindImageFS(shader, bnd, index, image);
}

void bindUniformVS(const Shader2 &shader, uint32_t index, const void *data, size_t size)
{
	const SpPermutationInfo &info = spPermutations[shader.vsIndex];
	uint32_t loc = 0;
	for (uint16_t ix : info.uniformBlocks) {
		if (ix == 0) break;
		if (ix == index) {
			sg_apply_uniforms(SG_SHADERSTAGE_VS, (int)loc, data, (int)size);
			break;
		}
		loc++;
	}
}

void bindUniformFS(const Shader2 &shader, uint32_t index, const void *data, size_t size)
{
	const SpPermutationInfo &info = spPermutations[shader.fsIndex];
	uint32_t loc = 0;
	for (uint16_t ix : info.uniformBlocks) {
		if (ix == 0) break;
		if (ix == index) {
			sg_apply_uniforms(SG_SHADERSTAGE_FS, (int)loc, data, (int)size);
			break;
		}
		loc++;
	}
}

void bindUniform(const Shader2 &shader, uint32_t index, const void *data, size_t size)
{
	bindUniformVS(shader, index, data, size);
	bindUniformFS(shader, index, data, size);
}
