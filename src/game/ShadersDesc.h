#pragma once

#include <stdint.h>

#define SP_MAX_PERMUTATIONS_PER_SHADER 8
#define SP_MAX_UNIFORMS_PER_SHADER 4
#define SP_MAX_SAMPLERS_PER_SHADER 16
#define SP_MAX_ATTRIBS_PER_SHADER 16

struct SpPermutationMapping
{
	uint8_t sharedIndex;
	uint8_t numValues;
};

struct SpUniformBlockInfo
{
	const char *name;
	uint32_t size;
};

struct SpSamplerInfo
{
	const char *name;
	uint32_t type;
};

struct SpAttribInfo
{
	const char *name;
	uint32_t location;
};

struct SpShaderStageInfo
{
	SpPermutationMapping permutationMapping[SP_MAX_PERMUTATIONS_PER_SHADER];
	uint32_t permutationOffset;
	uint32_t permutationCount;
};

struct SpShaderInfo
{
	const char *name;
	SpShaderStageInfo stages[2];
};

struct SpPermutationInfo
{
	uint16_t uniformBlocks[SP_MAX_UNIFORMS_PER_SHADER];
	uint16_t samplers[SP_MAX_SAMPLERS_PER_SHADER];
	uint16_t attribs[SP_MAX_ATTRIBS_PER_SHADER];
	uint32_t dataOffset;
	uint32_t dataSize;
};
