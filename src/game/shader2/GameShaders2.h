#pragma once

#include "sf/Base.h"

#include "ext/sokol/sokol_config.h"
#include "ext/sokol/sokol_defs.h"

#if defined(SOKOL_GLCORE33) || defined(SOKOL_DUMMY_BACKEND)
	#include "GameShaders_glsl.h"
#elif defined(SOKOL_GLES3)
	#include "GameShaders_gles.h"
#elif defined(SOKOL_D3D11)
	#include "GameShaders_hlsl.h"
#endif

struct Shader2
{
	sg_shader handle;
	uint16_t vsIndex;
	uint16_t fsIndex;
};

Shader2 getShader2(uint32_t index, sf::Slice<const uint8_t> permutations);

struct sg_bindings;
void bindImageVS(const Shader2 &shader, sg_bindings &bnd, uint32_t index, sg_image image);
void bindImageFS(const Shader2 &shader, sg_bindings &bnd, uint32_t index, sg_image image);
void bindUniformVS(const Shader2 &shader, uint32_t index, const void *data, size_t size);
void bindUniformFS(const Shader2 &shader, uint32_t index, const void *data, size_t size);

template <typename T>
inline void bindUniformVS(const Shader2 &shader, const T &t)
{
	bindUniformVS(shader, T::UboIndex, &t, sizeof(T));
}

template <typename T>
inline void bindUniformFS(const Shader2 &shader, const T &t)
{
	bindUniformFS(shader, T::UboIndex, &t, sizeof(T));
}

