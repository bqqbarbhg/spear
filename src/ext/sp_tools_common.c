#include "sf/Platform.h"

#define sp_malloc(size) sf_malloc(size)
#define sp_free(size) sf_free(size)

#include "sp_tools_common.h"
#include "ext/zstd.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

const sp_format_info sp_format_infos[SP_FORMAT_COUNT] = {
	{ SP_FORMAT_UNKNOWN, "SP_FORMAT_UNKNOWN", "(unknown)", 0, 0, 0, 0, 0 },
	{ SP_FORMAT_R8_UNORM, "SP_FORMAT_R8_UNORM", "r8", 1, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_R8_SNORM, "SP_FORMAT_R8_SNORM", "r8sn", 1, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_R8_UINT, "SP_FORMAT_R8_UINT", "r8u", 1, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_R8_SINT, "SP_FORMAT_R8_SINT", "r8i", 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RG8_UNORM, "SP_FORMAT_RG8_UNORM", "rg8", 2, 2, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RG8_SNORM, "SP_FORMAT_RG8_SNORM", "rg8sn", 2, 2, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RG8_UINT, "SP_FORMAT_RG8_UINT", "rg8u", 2, 2, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RG8_SINT, "SP_FORMAT_RG8_SINT", "rg8i", 2, 2, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGB8_UNORM, "SP_FORMAT_RGB8_UNORM", "rgb8", 3, 3, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGB8_SNORM, "SP_FORMAT_RGB8_SNORM", "rgb8sn", 3, 3, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGB8_UINT, "SP_FORMAT_RGB8_UINT", "rgb8u", 3, 3, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGB8_SINT, "SP_FORMAT_RGB8_SINT", "rgb8i", 3, 3, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGB8_SRGB, "SP_FORMAT_RGB8_SRGB", "rgb8_srgb", 3, 3, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_RGBA8_UNORM, "SP_FORMAT_RGBA8_UNORM", "rgba8", 4, 4, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGBA8_SNORM, "SP_FORMAT_RGBA8_SNORM", "rgba8sn", 4, 4, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGBA8_UINT, "SP_FORMAT_RGBA8_UINT", "rgba8u", 4, 4, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGBA8_SINT, "SP_FORMAT_RGBA8_SINT", "rgba8i", 4, 4, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGBA8_SRGB, "SP_FORMAT_RGBA8_SRGB", "rgba8_srgb", 4, 4, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_R16_UNORM, "SP_FORMAT_R16_UNORM", "r16", 1, 2, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_R16_SNORM, "SP_FORMAT_R16_SNORM", "r16sn", 1, 2, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_R16_UINT, "SP_FORMAT_R16_UINT", "r16u", 1, 2, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_R16_SINT, "SP_FORMAT_R16_SINT", "r16i", 1, 2, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_R16_FLOAT, "SP_FORMAT_R16_FLOAT", "r16f", 1, 2, 1, 1, SP_FORMAT_FLAG_FLOAT|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RG16_UNORM, "SP_FORMAT_RG16_UNORM", "rg16", 2, 4, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RG16_SNORM, "SP_FORMAT_RG16_SNORM", "rg16sn", 2, 4, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RG16_UINT, "SP_FORMAT_RG16_UINT", "rg16u", 2, 4, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RG16_SINT, "SP_FORMAT_RG16_SINT", "rg16i", 2, 4, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RG16_FLOAT, "SP_FORMAT_RG16_FLOAT", "rg16f", 2, 4, 1, 1, SP_FORMAT_FLAG_FLOAT|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGB16_UNORM, "SP_FORMAT_RGB16_UNORM", "rgb16", 3, 6, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGB16_SNORM, "SP_FORMAT_RGB16_SNORM", "rgb16sn", 3, 6, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGB16_UINT, "SP_FORMAT_RGB16_UINT", "rgb16u", 3, 6, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGB16_SINT, "SP_FORMAT_RGB16_SINT", "rgb16i", 3, 6, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGB16_FLOAT, "SP_FORMAT_RGB16_FLOAT", "rgb16f", 3, 6, 1, 1, SP_FORMAT_FLAG_FLOAT|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGBA16_UNORM, "SP_FORMAT_RGBA16_UNORM", "rgba16", 4, 8, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGBA16_SNORM, "SP_FORMAT_RGBA16_SNORM", "rgba16sn", 4, 8, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGBA16_UINT, "SP_FORMAT_RGBA16_UINT", "rgba16u", 4, 8, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGBA16_SINT, "SP_FORMAT_RGBA16_SINT", "rgba16i", 4, 8, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGBA16_FLOAT, "SP_FORMAT_RGBA16_FLOAT", "rgba16f", 4, 8, 1, 1, SP_FORMAT_FLAG_FLOAT|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_R32_UNORM, "SP_FORMAT_R32_UNORM", "r32", 1, 4, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_R32_SNORM, "SP_FORMAT_R32_SNORM", "r32sn", 1, 4, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_R32_UINT, "SP_FORMAT_R32_UINT", "r32u", 1, 4, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_R32_SINT, "SP_FORMAT_R32_SINT", "r32i", 1, 4, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_R32_FLOAT, "SP_FORMAT_R32_FLOAT", "r32f", 1, 4, 1, 1, SP_FORMAT_FLAG_FLOAT|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RG32_UNORM, "SP_FORMAT_RG32_UNORM", "rg32", 2, 8, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RG32_SNORM, "SP_FORMAT_RG32_SNORM", "rg32sn", 2, 8, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RG32_UINT, "SP_FORMAT_RG32_UINT", "rg32u", 2, 8, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RG32_SINT, "SP_FORMAT_RG32_SINT", "rg32i", 2, 8, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RG32_FLOAT, "SP_FORMAT_RG32_FLOAT", "rg32f", 2, 8, 1, 1, SP_FORMAT_FLAG_FLOAT|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGB32_UNORM, "SP_FORMAT_RGB32_UNORM", "rgb32", 3, 12, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGB32_SNORM, "SP_FORMAT_RGB32_SNORM", "rgb32sn", 3, 12, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGB32_UINT, "SP_FORMAT_RGB32_UINT", "rgb32u", 3, 12, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGB32_SINT, "SP_FORMAT_RGB32_SINT", "rgb32i", 3, 12, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGB32_FLOAT, "SP_FORMAT_RGB32_FLOAT", "rgb32f", 3, 12, 1, 1, SP_FORMAT_FLAG_FLOAT|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGBA32_UNORM, "SP_FORMAT_RGBA32_UNORM", "rgba32", 4, 16, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGBA32_SNORM, "SP_FORMAT_RGBA32_SNORM", "rgba32sn", 4, 16, 1, 1, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGBA32_UINT, "SP_FORMAT_RGBA32_UINT", "rgba32u", 4, 16, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGBA32_SINT, "SP_FORMAT_RGBA32_SINT", "rgba32i", 4, 16, 1, 1, SP_FORMAT_FLAG_INTEGER|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_RGBA32_FLOAT, "SP_FORMAT_RGBA32_FLOAT", "rgba32f", 4, 16, 1, 1, SP_FORMAT_FLAG_FLOAT|SP_FORMAT_FLAG_SIGNED|SP_FORMAT_FLAG_BASIC },
	{ SP_FORMAT_D16_UNORM, "SP_FORMAT_D16_UNORM", "d16", 1, 2, 1, 1, SP_FORMAT_FLAG_DEPTH },
	{ SP_FORMAT_D24S8_UNORM, "SP_FORMAT_D24S8_UNORM", "d24s8", 1, 4, 1, 1, SP_FORMAT_FLAG_DEPTH|SP_FORMAT_FLAG_STENCIL },
	{ SP_FORMAT_D32_FLOAT, "SP_FORMAT_D32_FLOAT", "d32f", 1, 4, 1, 1, SP_FORMAT_FLAG_DEPTH },
	{ SP_FORMAT_D32S8_FLOAT, "SP_FORMAT_D32S8_FLOAT", "d32s8f", 1, 8, 1, 1, SP_FORMAT_FLAG_DEPTH|SP_FORMAT_FLAG_STENCIL },
	{ SP_FORMAT_RGB10A2_UNORM, "SP_FORMAT_RGB10A2_UNORM", "rgb10a2", 4, 4, 1, 1, SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_RGB10A2_UINT, "SP_FORMAT_RGB10A2_UINT", "rgb10a2u", 4, 4, 1, 1, SP_FORMAT_FLAG_INTEGER },
	{ SP_FORMAT_R11G11B10_FLOAT, "SP_FORMAT_R11G11B10_FLOAT", "r11g11b10f", 3, 4, 1, 1, SP_FORMAT_FLAG_FLOAT|SP_FORMAT_FLAG_SIGNED },
	{ SP_FORMAT_BC1_UNORM, "SP_FORMAT_BC1_UNORM", "bc1", 4, 8, 4, 4, SP_FORMAT_FLAG_COMPRESSED|SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_BC1_SRGB, "SP_FORMAT_BC1_SRGB", "bc1_srgb", 4, 8, 4, 4, SP_FORMAT_FLAG_COMPRESSED|SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_BC2_UNORM, "SP_FORMAT_BC2_UNORM", "bc2", 4, 16, 4, 4, SP_FORMAT_FLAG_COMPRESSED|SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_BC2_SRGB, "SP_FORMAT_BC2_SRGB", "bc2_srgb", 4, 16, 4, 4, SP_FORMAT_FLAG_COMPRESSED|SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_BC3_UNORM, "SP_FORMAT_BC3_UNORM", "bc3", 4, 16, 4, 4, SP_FORMAT_FLAG_COMPRESSED|SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_BC3_SRGB, "SP_FORMAT_BC3_SRGB", "bc3_srgb", 4, 16, 4, 4, SP_FORMAT_FLAG_COMPRESSED|SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_BC4_UNORM, "SP_FORMAT_BC4_UNORM", "bc4", 1, 8, 4, 4, SP_FORMAT_FLAG_COMPRESSED|SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_BC4_SNORM, "SP_FORMAT_BC4_SNORM", "bc4sn", 1, 8, 4, 4, SP_FORMAT_FLAG_COMPRESSED|SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SIGNED },
	{ SP_FORMAT_BC5_UNORM, "SP_FORMAT_BC5_UNORM", "bc5", 2, 16, 4, 4, SP_FORMAT_FLAG_COMPRESSED|SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_BC5_SNORM, "SP_FORMAT_BC5_SNORM", "bc5sn", 2, 16, 4, 4, SP_FORMAT_FLAG_COMPRESSED|SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SIGNED },
	{ SP_FORMAT_BC6_UFLOAT, "SP_FORMAT_BC6_UFLOAT", "bc6_ufloat", 3, 16, 4, 4, SP_FORMAT_FLAG_COMPRESSED|SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_FLOAT },
	{ SP_FORMAT_BC6_SFLOAT, "SP_FORMAT_BC6_SFLOAT", "bc6_sfloat", 3, 16, 4, 4, SP_FORMAT_FLAG_COMPRESSED|SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_FLOAT|SP_FORMAT_FLAG_SIGNED },
	{ SP_FORMAT_BC7_UNORM, "SP_FORMAT_BC7_UNORM", "bc7", 4, 16, 4, 4, SP_FORMAT_FLAG_COMPRESSED|SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_BC7_SRGB, "SP_FORMAT_BC7_SRGB", "bc7_srgb", 4, 16, 4, 4, SP_FORMAT_FLAG_COMPRESSED|SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_ASTC4X4_UNORM, "SP_FORMAT_ASTC4X4_UNORM", "astc4x4", 4, 16, 4, 4, SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_ASTC4X4_SRGB, "SP_FORMAT_ASTC4X4_SRGB", "astc4x4_srgb", 4, 16, 4, 4, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_ASTC5X4_UNORM, "SP_FORMAT_ASTC5X4_UNORM", "astc5x4", 4, 16, 5, 4, SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_ASTC5X4_SRGB, "SP_FORMAT_ASTC5X4_SRGB", "astc5x4_srgb", 4, 16, 5, 4, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_ASTC5X5_UNORM, "SP_FORMAT_ASTC5X5_UNORM", "astc5x5", 4, 16, 5, 5, SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_ASTC5X5_SRGB, "SP_FORMAT_ASTC5X5_SRGB", "astc5x5_srgb", 4, 16, 5, 5, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_ASTC6X5_UNORM, "SP_FORMAT_ASTC6X5_UNORM", "astc6x5", 4, 16, 6, 5, SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_ASTC6X5_SRGB, "SP_FORMAT_ASTC6X5_SRGB", "astc6x5_srgb", 4, 16, 6, 5, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_ASTC6X6_UNORM, "SP_FORMAT_ASTC6X6_UNORM", "astc6x6", 4, 16, 6, 6, SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_ASTC6X6_SRGB, "SP_FORMAT_ASTC6X6_SRGB", "astc6x6_srgb", 4, 16, 6, 6, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_ASTC8X5_UNORM, "SP_FORMAT_ASTC8X5_UNORM", "astc8x5", 4, 16, 8, 5, SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_ASTC8X5_SRGB, "SP_FORMAT_ASTC8X5_SRGB", "astc8x5_srgb", 4, 16, 8, 5, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_ASTC8X6_UNORM, "SP_FORMAT_ASTC8X6_UNORM", "astc8x6", 4, 16, 8, 6, SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_ASTC8X6_SRGB, "SP_FORMAT_ASTC8X6_SRGB", "astc8x6_srgb", 4, 16, 8, 6, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_ASTC10X5_UNORM, "SP_FORMAT_ASTC10X5_UNORM", "astc10x5", 4, 16, 10, 5, SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_ASTC10X5_SRGB, "SP_FORMAT_ASTC10X5_SRGB", "astc10x5_srgb", 4, 16, 10, 5, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_ASTC10X6_UNORM, "SP_FORMAT_ASTC10X6_UNORM", "astc10x6", 4, 16, 10, 6, SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_ASTC10X6_SRGB, "SP_FORMAT_ASTC10X6_SRGB", "astc10x6_srgb", 4, 16, 10, 6, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_ASTC8X8_UNORM, "SP_FORMAT_ASTC8X8_UNORM", "astc8x8", 4, 16, 8, 8, SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_ASTC8X8_SRGB, "SP_FORMAT_ASTC8X8_SRGB", "astc8x8_srgb", 4, 16, 8, 8, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_ASTC10X8_UNORM, "SP_FORMAT_ASTC10X8_UNORM", "astc10x8", 4, 16, 10, 8, SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_ASTC10X8_SRGB, "SP_FORMAT_ASTC10X8_SRGB", "astc10x8_srgb", 4, 16, 10, 8, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_ASTC10X10_UNORM, "SP_FORMAT_ASTC10X10_UNORM", "astc10x10", 4, 16, 10, 10, SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_ASTC10X10_SRGB, "SP_FORMAT_ASTC10X10_SRGB", "astc10x10_srgb", 4, 16, 10, 10, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_ASTC12X10_UNORM, "SP_FORMAT_ASTC12X10_UNORM", "astc12x10", 4, 16, 12, 10, SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_ASTC12X10_SRGB, "SP_FORMAT_ASTC12X10_SRGB", "astc12x10_srgb", 4, 16, 12, 10, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
	{ SP_FORMAT_ASTC12X12_UNORM, "SP_FORMAT_ASTC12X12_UNORM", "astc12x12", 4, 16, 12, 12, SP_FORMAT_FLAG_NORMALIZED },
	{ SP_FORMAT_ASTC12X12_SRGB, "SP_FORMAT_ASTC12X12_SRGB", "astc12x12_srgb", 4, 16, 12, 12, SP_FORMAT_FLAG_NORMALIZED|SP_FORMAT_FLAG_SRGB },
};

sp_format sp_find_format(uint32_t num_components, uint32_t component_size, sp_format_flags flags)
{
	uint32_t key = component_size << 4 | num_components;
	if (flags & SP_FORMAT_FLAG_FLOAT) {
		switch (key) {
		case 0x11: return SP_FORMAT_UNKNOWN;
		case 0x12: return SP_FORMAT_UNKNOWN;
		case 0x13: return SP_FORMAT_UNKNOWN;
		case 0x14: return SP_FORMAT_UNKNOWN;
		case 0x21: return SP_FORMAT_R16_FLOAT;
		case 0x22: return SP_FORMAT_RG16_FLOAT;
		case 0x23: return SP_FORMAT_RGB16_FLOAT;
		case 0x24: return SP_FORMAT_RGBA16_FLOAT;
		case 0x41: return SP_FORMAT_R32_FLOAT;
		case 0x42: return SP_FORMAT_RG32_FLOAT;
		case 0x43: return SP_FORMAT_RGB32_FLOAT;
		case 0x44: return SP_FORMAT_RGBA32_FLOAT;
		default: return SP_FORMAT_UNKNOWN;
		}
	} else if (flags & SP_FORMAT_FLAG_NORMALIZED) {
		if (flags & SP_FORMAT_FLAG_SIGNED) {
			switch (key) {
			case 0x11: return SP_FORMAT_R8_SNORM;
			case 0x12: return SP_FORMAT_RG8_SNORM;
			case 0x13: return SP_FORMAT_RGB8_SNORM;
			case 0x14: return SP_FORMAT_RGBA8_SNORM;
			case 0x21: return SP_FORMAT_R16_SNORM;
			case 0x22: return SP_FORMAT_RG16_SNORM;
			case 0x23: return SP_FORMAT_RGB16_SNORM;
			case 0x24: return SP_FORMAT_RGBA16_SNORM;
			case 0x41: return SP_FORMAT_R32_SNORM;
			case 0x42: return SP_FORMAT_RG32_SNORM;
			case 0x43: return SP_FORMAT_RGB32_SNORM;
			case 0x44: return SP_FORMAT_RGBA32_SNORM;
			default: return SP_FORMAT_UNKNOWN;
			}
		} else {
			switch (key) {
			case 0x11: return SP_FORMAT_R8_UNORM;
			case 0x12: return SP_FORMAT_RG8_UNORM;
			case 0x13: return SP_FORMAT_RGB8_UNORM;
			case 0x14: return SP_FORMAT_RGBA8_UNORM;
			case 0x21: return SP_FORMAT_R16_UNORM;
			case 0x22: return SP_FORMAT_RG16_UNORM;
			case 0x23: return SP_FORMAT_RGB16_UNORM;
			case 0x24: return SP_FORMAT_RGBA16_UNORM;
			case 0x41: return SP_FORMAT_R32_UNORM;
			case 0x42: return SP_FORMAT_RG32_UNORM;
			case 0x43: return SP_FORMAT_RGB32_UNORM;
			case 0x44: return SP_FORMAT_RGBA32_UNORM;
			default: return SP_FORMAT_UNKNOWN;
			}
		}
	} else if (flags & SP_FORMAT_FLAG_INTEGER) {
		if (flags & SP_FORMAT_FLAG_SIGNED) {
			switch (key) {
			case 0x11: return SP_FORMAT_R8_SINT;
			case 0x12: return SP_FORMAT_RG8_SINT;
			case 0x13: return SP_FORMAT_RGB8_SINT;
			case 0x14: return SP_FORMAT_RGBA8_SINT;
			case 0x21: return SP_FORMAT_R16_SINT;
			case 0x22: return SP_FORMAT_RG16_SINT;
			case 0x23: return SP_FORMAT_RGB16_SINT;
			case 0x24: return SP_FORMAT_RGBA16_SINT;
			case 0x41: return SP_FORMAT_R32_SINT;
			case 0x42: return SP_FORMAT_RG32_SINT;
			case 0x43: return SP_FORMAT_RGB32_SINT;
			case 0x44: return SP_FORMAT_RGBA32_SINT;
			default: return SP_FORMAT_UNKNOWN;
			}
		} else {
			switch (key) {
			case 0x11: return SP_FORMAT_R8_UINT;
			case 0x12: return SP_FORMAT_RG8_UINT;
			case 0x13: return SP_FORMAT_RGB8_UINT;
			case 0x14: return SP_FORMAT_RGBA8_UINT;
			case 0x21: return SP_FORMAT_R16_UINT;
			case 0x22: return SP_FORMAT_RG16_UINT;
			case 0x23: return SP_FORMAT_RGB16_UINT;
			case 0x24: return SP_FORMAT_RGBA16_UINT;
			case 0x41: return SP_FORMAT_R32_UINT;
			case 0x42: return SP_FORMAT_RG32_UINT;
			case 0x43: return SP_FORMAT_RGB32_UINT;
			case 0x44: return SP_FORMAT_RGBA32_UINT;
			default: return SP_FORMAT_UNKNOWN;
			}
		}
	}
	return SP_FORMAT_UNKNOWN;
}

size_t sp_get_compression_bound(sp_compression_type type, size_t src_size)
{
	switch (type)
	{
	case SP_COMPRESSION_NONE: return src_size;
	case SP_COMPRESSION_ZSTD: return ZSTD_compressBound(src_size);
	default: return 0;
	}
}

size_t sp_compress_buffer(sp_compression_type type, void *dst, size_t dst_size, const void *src, size_t src_size, int level)
{
	if (level < 1) level = 1;
	if (level > 20) level = 20;
	switch (type)
	{
	case SP_COMPRESSION_NONE:
		assert(dst_size >= src_size);
		memcpy(dst, src, src_size);
		return src_size;
	case SP_COMPRESSION_ZSTD:
		return ZSTD_compress(dst, dst_size, src, src_size, level - 1);
	default: return 0;
	}
}

size_t sp_decompress_buffer(sp_compression_type type, void *dst, size_t dst_size, const void *src, size_t src_size)
{
	switch (type)
	{
	case SP_COMPRESSION_NONE:
		assert(dst_size >= src_size);
		memcpy(dst, src, src_size);
		return src_size;
	case SP_COMPRESSION_ZSTD:
		return ZSTD_decompress(dst, dst_size, src, src_size);
	default: return 0;
	}
}

static bool spfile_fail(spfile_util *su)
{
	su->failed = true;
	return false;
}

#define spfile_check(cond) if (!(cond)) { return spfile_fail((spfile_util*)(su)); }
#define spfile_check_ret(cond, ret) if (!(cond)) { spfile_fail((spfile_util*)(su)); return ret; }

static bool spfile_check_string(spfile_util *su, const spfile_string *str)
{
	spfile_check(su->strings);
	spfile_check(str->offset <= su->strings_size && su->strings_size - str->offset > str->length);
	spfile_check(su->strings[str->offset + str->length] == '\0');
	return true;
}

bool spfile_util_init(spfile_util *su, const void *data, size_t size)
{
	su->data = data;
	su->size = size;
	su->page_to_free = NULL;
	su->failed = false;
	su->strings = NULL;
	su->strings_size = 0;
	spfile_check(size >= sizeof(spfile_header));
	spfile_header *header = (spfile_header*)data;

	size_t offset = sizeof(spfile_header);
	spfile_check(size - offset >= header->header_info_size);
	offset += header->header_info_size;

	for (uint32_t i = 0; i < header->num_sections; i++) {
		spfile_check(size - offset >= sizeof(spfile_section));
		const spfile_section *section = (const spfile_section*)((const char*)data + offset);
		spfile_check(section->offset <= size);
		spfile_check(size - section->offset >= section->compressed_size);
		spfile_check(section->offset % 16 == 0);
		spfile_check(section->compression_type >= SP_COMPRESSION_TYPE_FIRST && section->compression_type <= SP_COMPRESSION_TYPE_LAST);
		offset += sizeof(spfile_section);
	}
	return true;
}

bool spfile_decode_section_to(spfile_util *su, const spfile_section *s, void *buffer)
{
	if (su->failed) return false;
	spfile_check(s->offset % 16 == 0);
	if (s->uncompressed_size == 0) return true;
	
	void *src = (char*)su->data + s->offset;
	size_t size = sp_decompress_buffer(s->compression_type, buffer, s->uncompressed_size, src, s->compressed_size);
	spfile_check(size == s->uncompressed_size);
	return true;
}

void *spfile_decode_section(spfile_util *su, const spfile_section *s)
{
	if (su->failed) return NULL;

	if (s->compression_type == SP_COMPRESSION_NONE) {
		return (char*)su->data + s->offset;
	} else {
		void *data = sp_malloc(s->uncompressed_size + 16);
		spfile_check_ret(data, NULL);

		*(void**)data = su->page_to_free;
		su->page_to_free = data;

		char *dst = (char*)data + 16;
		spfile_decode_section_to(su, s, dst);
		return su->failed ? NULL : dst;
	}
}

bool spfile_decode_strings_to(spfile_util *su, const spfile_section *s, char *buffer)
{
	spfile_check(s->magic == SPFILE_SECTION_STRINGS);
	if (spfile_decode_section_to(su, s, buffer)) {
		su->strings = (char*)buffer;
		su->strings_size = s->uncompressed_size;
	}
	return true;
}

char *spfile_decode_strings(spfile_util *su, const spfile_section *s)
{
	spfile_check_ret(s->magic == SPFILE_SECTION_STRINGS, NULL);
	void *section = spfile_decode_section(su, s);
	if (section) {
		su->strings = (char*)section;
		su->strings_size = s->uncompressed_size;
	}
	return section;
}

bool spfile_util_failed(spfile_util *su)
{
	return su->failed;
}

void spfile_util_free(spfile_util *su)
{
	void *to_free = su->page_to_free;
	while (to_free) {
		void *next = *(void**)to_free;
		sp_free(to_free);

		to_free = next;
	}
	memset(su, 0, sizeof(spfile_util));
}

bool spanim_check_bones(spanim_util *su, spanim_bone *bones)
{
	if (su->file.failed) return false;
	spanim_header *header = (spanim_header*)su->file.data;
	if (!su->file.failed) {
		uint32_t ix = 0;
		for (spanim_bone *b = bones, *end = b + header->info.num_bones; b != end; b++) {
			spfile_check_string(&su->file, &b->name);
			if (ix > 0) {
				spfile_check(b->parent < ix);
			} else {
				spfile_check(b->parent == ~0u);
			}
			ix++;
		}
	}
	return true;
}

bool spanim_util_init(spanim_util *su, const void *data, size_t size)
{
	spfile_util_init(&su->file, data, size);
	spfile_check(size >= sizeof(spanim_header));
	if (su->file.failed) return false;

	spanim_header *header = (spanim_header*)data;
	spfile_check(header->header.magic == SPFILE_HEADER_SPANIM);
	spfile_check(header->header.header_info_size == sizeof(spanim_info));
	spfile_check(header->header.version == 1);

	spfile_check(header->s_bones.uncompressed_size / sizeof(spanim_bone) == header->info.num_bones);
	return true;
}

bool spanim_decode_strings_to(spanim_util *su, char *buffer)
{
	if (su->file.failed) return false;
	spanim_header *header = (spanim_header*)su->file.data;
	return spfile_decode_strings_to(&su->file, &header->s_strings, buffer);
}

bool spanim_decode_bones_to(spanim_util *su, spanim_bone *buffer)
{
	if (su->file.failed) return false;
	spanim_header *header = (spanim_header*)su->file.data;
	if (spfile_decode_section_to(&su->file, &header->s_bones, buffer)) {
		return spanim_check_bones(su, buffer);
	} else {
		return false;
	}
}

bool spanim_decode_animation_to(spanim_util *su, char *buffer)
{
	if (su->file.failed) return false;
	spanim_header *header = (spanim_header*)su->file.data;
	return spfile_decode_section_to(&su->file, &header->s_animation, buffer);
}

spanim_header spanim_decode_header(spanim_util *su)
{
	if (su->file.failed) {
		spanim_header zero = { 0 };
		return zero;
	}
	return *(spanim_header*)su->file.data;
}

char *spanim_decode_strings(spanim_util *su)
{
	if (su->file.failed) return false;
	spanim_header *header = (spanim_header*)su->file.data;
	return spfile_decode_strings(&su->file, &header->s_strings);
}

spanim_bone *spanim_decode_bones(spanim_util *su)
{
	if (su->file.failed) return NULL;
	spanim_header *header = (spanim_header*)su->file.data;
	spanim_bone *buffer = spfile_decode_section(&su->file, &header->s_bones);
	if (buffer) {
		if (!spanim_check_bones(su, buffer)) return NULL;
		return buffer;
	} else {
		return NULL;
	}
}

char *spanim_decode_animation(spanim_util *su)
{
	if (su->file.failed) return NULL;
	spanim_header *header = (spanim_header*)su->file.data;
	return (char*)spfile_decode_section(&su->file, &header->s_animation);
}

bool spmdl_util_init(spmdl_util *su, const void *data, size_t size)
{
	spfile_util_init(&su->file, data, size);
	spfile_check(size >= sizeof(spmdl_header));
	if (su->file.failed) return false;

	spmdl_header *header = (spmdl_header*)data;
	spfile_check(header->header.magic == SPFILE_HEADER_SPMDL);
	spfile_check(header->header.header_info_size == sizeof(spmdl_info));
	spfile_check(header->header.version == 1);

	spfile_check(header->s_nodes.uncompressed_size / sizeof(spmdl_node) == header->info.num_nodes);
	spfile_check(header->s_bones.uncompressed_size / sizeof(spmdl_bone) == header->info.num_bones);
	spfile_check(header->s_materials.uncompressed_size / sizeof(spmdl_material) == header->info.num_materials);
	spfile_check(header->s_meshes.uncompressed_size / sizeof(spmdl_mesh) == header->info.num_meshes);
	spfile_check(header->s_bvh_nodes.uncompressed_size / sizeof(spmdl_bvh_node) == header->info.num_bvh_nodes);
	spfile_check(header->s_bvh_tris.uncompressed_size / sizeof(uint32_t) == header->info.num_bvh_tris * 3);
	return true;
}

bool spmdl_check_nodes(spmdl_util *su, spmdl_node *nodes)
{
	spmdl_header *header = (spmdl_header*)su->file.data;
	uint32_t ix = 0;
	for (spmdl_node *n = nodes, *end = n + header->info.num_nodes; n != end; n++) {
		if (!spfile_check_string(&su->file, &n->name)) return false;
		if (ix > 0) {
			spfile_check(n->parent < ix);
		} else {
			spfile_check(n->parent == ~0u);
		}
		ix++;
	}
	return true;
}

bool spmdl_check_bones(spmdl_util *su, spmdl_bone *bones)
{
	if (su->file.failed) return false;
	spmdl_header *header = (spmdl_header*)su->file.data;
	if (!su->file.failed) {
		for (spmdl_bone *b = bones, *end = b + header->info.num_bones; b != end; b++) {
			spfile_check(b->node < header->info.num_nodes);
		}
	}
	return true;
}

bool spmdl_check_materials(spmdl_util *su, spmdl_material *materials)
{
	if (su->file.failed) return false;
	spmdl_header *header = (spmdl_header*)su->file.data;
	if (!su->file.failed) {
		for (spmdl_material *b = materials, *end = b + header->info.num_materials; b != end; b++) {
			spfile_check_string(&su->file, &b->name);
		}
	}
	return true;
}

bool spmdl_check_meshes(spmdl_util *su, spmdl_mesh *meshes)
{
	if (su->file.failed) return false;
	spmdl_header *header = (spmdl_header*)su->file.data;
	if (!su->file.failed) {
		for (spmdl_mesh *m = meshes, *end = m + header->info.num_meshes; m != end; m++) {
			spfile_check(m->bone_offset <= header->info.num_bones && header->info.num_bones - m->bone_offset >= m->num_bones);
			spfile_check(m->node < header->info.num_nodes);
			spfile_check(m->num_vertex_buffers < SPMDL_MAX_VERTEX_BUFFERS);
			spfile_check(m->num_attribs < SPMDL_MAX_VERTEX_ATTRIBS);
			uint32_t index_size = header->s_index.uncompressed_size;
			spfile_check(m->index_buffer.stride == 2 || m->index_buffer.stride == 4);
			spfile_check(m->index_buffer.offset <= index_size && index_size - m->index_buffer.offset >= m->index_buffer.encoded_size);
			for (uint32_t i = 0; i < m->num_vertex_buffers; i++) {
				const spmdl_buffer *buf = (const spmdl_buffer*)&m->vertex_buffers[i];
				uint32_t vertex_size = header->s_vertex.uncompressed_size;
				spfile_check(buf->offset <= vertex_size && vertex_size - buf->offset >= buf->encoded_size);
			}
		}
	}
	return true;
}

bool spmdl_check_bvh_nodes(spmdl_util *su, spmdl_bvh_node *nodes)
{
	if (su->file.failed) return false;
	spmdl_header *header = (spmdl_header*)su->file.data;
	if (!su->file.failed) {
		for (spmdl_bvh_node *n = nodes, *end = n + header->info.num_bvh_nodes; n != end; n++) {
			for (uint32_t i = 0; i < 2; i++) {
				spmdl_bvh_split *s = &n->splits[i];
				if (s->num_triangles >= 0) {
					spfile_check((uint32_t)s->num_triangles <= header->info.num_bvh_tris);
					spfile_check(s->data_index <= header->info.num_bvh_tris - s->num_triangles);
				} else {
					spfile_check(s->num_triangles == -1);
					spfile_check(s->data_index >= (uint32_t)(n - nodes));
					spfile_check(s->data_index < header->info.num_bvh_nodes);
				}
			}
		}
	}
	return true;
}

bool spmdl_decode_strings_to(spmdl_util *su, char *buffer)
{
	if (su->file.failed) return false;
	spmdl_header *header = (spmdl_header*)su->file.data;
	return spfile_decode_strings_to(&su->file, &header->s_strings, buffer);
}

bool spmdl_decode_nodes_to(spmdl_util *su, spmdl_node *buffer)
{
	if (su->file.failed) return false;
	spmdl_header *header = (spmdl_header*)su->file.data;
	if (spfile_decode_section_to(&su->file, &header->s_nodes, buffer)) {
		return spmdl_check_nodes(su, buffer);
	} else {
		return false;
	}
}

bool spmdl_decode_bones_to(spmdl_util *su, spmdl_bone *buffer)
{
	if (su->file.failed) return false;
	spmdl_header *header = (spmdl_header*)su->file.data;
	if (spfile_decode_section_to(&su->file, &header->s_bones, buffer)) {
		return spmdl_check_bones(su, buffer);
	} else {
		return false;
	}
}

bool spmdl_decode_materials_to(spmdl_util *su, spmdl_material *buffer)
{
	if (su->file.failed) return false;
	spmdl_header *header = (spmdl_header*)su->file.data;
	if (spfile_decode_section_to(&su->file, &header->s_materials, buffer)) {
		return spmdl_check_materials(su, buffer);
	} else {
		return false;
	}
}

bool spmdl_decode_meshes_to(spmdl_util *su, spmdl_mesh *buffer)
{
	if (su->file.failed) return false;
	spmdl_header *header = (spmdl_header*)su->file.data;
	if (spfile_decode_section_to(&su->file, &header->s_meshes, buffer)) {
		return spmdl_check_meshes(su, buffer);
	} else {
		return false;
	}
}

bool spmdl_decode_bvh_nodes_to(spmdl_util *su, spmdl_bvh_node *buffer)
{
	if (su->file.failed) return false;
	spmdl_header *header = (spmdl_header*)su->file.data;
	if (spfile_decode_section_to(&su->file, &header->s_bvh_nodes, buffer)) {
		return spmdl_check_bvh_nodes(su, buffer);
	} else {
		return false;
	}
}

bool spmdl_decode_bvh_tris_to(spmdl_util *su, uint32_t *buffer)
{
	if (su->file.failed) return false;
	spmdl_header *header = (spmdl_header*)su->file.data;
	if (spfile_decode_section_to(&su->file, &header->s_bvh_tris, buffer)) {
		return true;
	} else {
		return false;
	}
}

bool spmdl_decode_vertex_to(spmdl_util *su, char *buffer)
{
	if (su->file.failed) return false;
	spmdl_header *header = (spmdl_header*)su->file.data;
	return spfile_decode_section_to(&su->file, &header->s_vertex, buffer);
}

bool spmdl_decode_index_to(spmdl_util *su, char *buffer)
{
	if (su->file.failed) return false;
	spmdl_header *header = (spmdl_header*)su->file.data;
	return spfile_decode_section_to(&su->file, &header->s_index, buffer);
}

spmdl_header spmdl_decode_header(spmdl_util *su)
{
	if (su->file.failed) {
		spmdl_header zero = { 0 };
		return zero;
	}
	return *(spmdl_header*)su->file.data;
}

char *spmdl_decode_strings(spmdl_util *su)
{
	if (su->file.failed) return false;
	spmdl_header *header = (spmdl_header*)su->file.data;
	return spfile_decode_strings(&su->file, &header->s_strings);
}

spmdl_node *spmdl_decode_nodes(spmdl_util *su)
{
	if (su->file.failed) return NULL;
	spmdl_header *header = (spmdl_header*)su->file.data;
	spmdl_node *buffer = spfile_decode_section(&su->file, &header->s_nodes);
	if (buffer) {
		if (!spmdl_check_nodes(su, buffer)) return NULL;
		return buffer;
	} else {
		return NULL;
	}
}

spmdl_bone *spmdl_decode_bones(spmdl_util *su)
{
	if (su->file.failed) return NULL;
	spmdl_header *header = (spmdl_header*)su->file.data;
	spmdl_bone *buffer = spfile_decode_section(&su->file, &header->s_bones);
	if (buffer) {
		if (!spmdl_check_bones(su, buffer)) return NULL;
		return buffer;
	} else {
		return NULL;
	}
}

spmdl_material *spmdl_decode_materials(spmdl_util *su)
{
	if (su->file.failed) return NULL;
	spmdl_header *header = (spmdl_header*)su->file.data;
	spmdl_material *buffer = spfile_decode_section(&su->file, &header->s_materials);
	if (buffer) {
		if (!spmdl_check_materials(su, buffer)) return NULL;
		return buffer;
	} else {
		return NULL;
	}
}


spmdl_mesh *spmdl_decode_meshes(spmdl_util *su)
{
	if (su->file.failed) return NULL;
	spmdl_header *header = (spmdl_header*)su->file.data;
	spmdl_mesh *buffer = spfile_decode_section(&su->file, &header->s_meshes);
	if (buffer) {
		if (!spmdl_check_meshes(su, buffer)) return NULL;
		return buffer;
	} else {
		return NULL;
	}
}

spmdl_bvh_node *spmdl_decode_bvh_nodes(spmdl_util *su)
{
	if (su->file.failed) return NULL;
	spmdl_header *header = (spmdl_header*)su->file.data;
	spmdl_bvh_node *buffer = spfile_decode_section(&su->file, &header->s_bvh_nodes);
	if (buffer) {
		if (!spmdl_check_bvh_nodes(su, buffer)) return NULL;
		return buffer;
	} else {
		return NULL;
	}
}

uint32_t *spmdl_decode_bvh_tris(spmdl_util *su)
{
	if (su->file.failed) return NULL;
	spmdl_header *header = (spmdl_header*)su->file.data;
	uint32_t *buffer = spfile_decode_section(&su->file, &header->s_bvh_tris);
	if (buffer) {
		return buffer;
	} else {
		return NULL;
	}
}

char *spmdl_decode_vertex(spmdl_util *su)
{
	if (su->file.failed) return NULL;
	spmdl_header *header = (spmdl_header*)su->file.data;
	return (char*)spfile_decode_section(&su->file, &header->s_vertex);
}

char *spmdl_decode_index(spmdl_util *su)
{
	if (su->file.failed) return NULL;
	spmdl_header *header = (spmdl_header*)su->file.data;
	return (char*)spfile_decode_section(&su->file, &header->s_index);
}

bool sptex_util_init(sptex_util *su, const void *data, size_t size)
{
	spfile_util_init(&su->file, data, size);
	if (su->file.failed) return false;

	sptex_header *header = (sptex_header*)data;
	spfile_check(header->header.magic == SPFILE_HEADER_SPTEX);
	spfile_check(header->header.header_info_size == sizeof(sptex_info));
	spfile_check(header->header.version == 1);
	spfile_check(header->info.num_mips == header->header.num_sections);
	spfile_check(header->info.num_mips <= 16);

	return true;
}

bool sptex_decode_mip_to(sptex_util *su, uint32_t index, char *buffer)
{
	if (su->file.failed) return false;
	sptex_header *header = (sptex_header*)su->file.data;
	return spfile_decode_section_to(&su->file, &header->s_mips[index], buffer);
}

sptex_header sptex_decode_header(sptex_util *su)
{
	sptex_header header = { 0 };
	if (su->file.failed) return header;
	memcpy(&header, su->file.data, sizeof(spfile_header) + sizeof(sptex_info));
	memcpy(header.s_mips, (char*)su->file.data + sizeof(spfile_header) + sizeof(sptex_info), sizeof(spfile_section) * header.info.num_mips);
	return header;
}

char *sptex_decode_mip(sptex_util *su, uint32_t index)
{
	if (su->file.failed) return NULL;
	sptex_header *header = (sptex_header*)su->file.data;
	return (char*)spfile_decode_section(&su->file, &header->s_mips[index]);
}

bool spsound_util_init(spsound_util *su, const void *data, size_t size)
{
	spfile_util_init(&su->file, data, size);
	if (su->file.failed) return false;

	spsound_header *header = (spsound_header*)data;
	spfile_check(header->header.magic == SPFILE_HEADER_SPSOUND);
	spfile_check(header->header.header_info_size == sizeof(spsound_info));
	spfile_check(header->header.version == 1);
	spfile_check(header->header.num_sections == 2);

	return true;
}

bool spsound_decode_takes_to(spsound_util *su, spsound_take *takes)
{
	if (su->file.failed) return false;
	spsound_header *header = (spsound_header*)su->file.data;
	return spfile_decode_section_to(&su->file, &header->s_takes, takes);
}

bool spsound_decode_audio_to(spsound_util *su, void *buffer)
{
	if (su->file.failed) return false;
	spsound_header *header = (spsound_header*)su->file.data;
	return spfile_decode_section_to(&su->file, &header->s_audio, buffer);
}

spsound_header spsound_decode_header(spsound_util *su)
{
	spsound_header header = { 0 };
	if (su->file.failed) return header;
	memcpy(&header, su->file.data, sizeof(spsound_header));
	return header;
}

spsound_take *spsound_decode_takes(spsound_util *su)
{
	if (su->file.failed) return NULL;
	spsound_header *header = (spsound_header*)su->file.data;
	return (spsound_take*)spfile_decode_section(&su->file, &header->s_takes);
}

void *spsound_decode_audio(spsound_util *su)
{
	if (su->file.failed) return NULL;
	spsound_header *header = (spsound_header*)su->file.data;
	return (char*)spfile_decode_section(&su->file, &header->s_audio);
}
