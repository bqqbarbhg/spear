#include "GameShaders_gles.h"
#include "game/ShadersDesc.h"

const SpShaderInfo spShaders[] = {
	{
		"DebugEnvSphere",
		{ {
			{  },
			0, 1,
		}, {
			{  },
			1, 1,
		} }
	},
	{
		"DynamicMesh",
		{ {
			{  },
			2, 1,
		}, {
			{ {0,2}, {1,2} },
			3, 4,
		} }
	},
	{
		"EnvmapLighting",
		{ {
			{  },
			7, 1,
		}, {
			{ {0,2}, {2,2}, {3,2} },
			8, 8,
		} }
	},
	{
		"TestDepthPrepass",
		{ {
			{  },
			16, 1,
		}, {
			{  },
			17, 1,
		} }
	},
	{
		"TestMesh",
		{ {
			{  },
			18, 1,
		}, {
			{ {0,2}, {1,2} },
			19, 4,
		} }
	},
	{
		"TestSkin",
		{ {
			{  },
			23, 1,
		}, {
			{ {0,2}, {1,2} },
			24, 4,
		} }
	},
};

const SpPermutationInfo spPermutations[] = {
	{ { 1 }, {  }, { 1 }, 0, 1132 },
	{ { 2 }, { 1,2 }, {  }, 1132, 3879 },
	{ { 3 }, {  }, { 1,2,3,4 }, 5011, 769 },
	{ { 4 }, { 3,1,2,4,5,6,7 }, {  }, 5780, 8400 },
	{ { 4 }, { 3,1,2,4,5,6,7 }, {  }, 14180, 8400 },
	{ { 4 }, { 8,1,2,4,5,6,7 }, {  }, 22580, 8654 },
	{ { 4 }, { 8,1,2,4,5,6,7 }, {  }, 31234, 8654 },
	{ { 5 }, {  }, { 5 }, 39888, 347 },
	{ { 6 }, { 3,2,9,10 }, {  }, 40235, 6387 },
	{ { 6 }, { 3,2,9,10 }, {  }, 46622, 6642 },
	{ { 6 }, { 3,2,9,10 }, {  }, 53264, 6168 },
	{ { 6 }, { 3,2,9,10 }, {  }, 59432, 6423 },
	{ { 6 }, { 8,2,9,10 }, {  }, 65855, 6641 },
	{ { 6 }, { 8,2,9,10 }, {  }, 72496, 6896 },
	{ { 6 }, { 8,2,9,10 }, {  }, 79392, 6422 },
	{ { 6 }, { 8,2,9,10 }, {  }, 85814, 6677 },
	{ { 7 }, {  }, { 1 }, 92491, 301 },
	{ {  }, {  }, {  }, 92792, 60 },
	{ { 7 }, {  }, { 1,2,3,4,6 }, 92852, 1346 },
	{ { 4 }, { 3,1,2,4,11,12,13 }, {  }, 94198, 8441 },
	{ { 4 }, { 3,1,2,4,11,12,13 }, {  }, 102639, 8441 },
	{ { 4 }, { 8,1,2,4,11,12,13 }, {  }, 111080, 8695 },
	{ { 4 }, { 8,1,2,4,11,12,13 }, {  }, 119775, 8695 },
	{ { 8,9 }, {  }, { 1,7,8,9,10,11 }, 128470, 1556 },
	{ { 10 }, { 3,1,2,4,5,6,7 }, {  }, 130026, 8685 },
	{ { 10 }, { 3,1,2,4,5,6,7 }, {  }, 138711, 8685 },
	{ { 10 }, { 8,1,2,4,5,6,7 }, {  }, 147396, 8939 },
	{ { 10 }, { 8,1,2,4,5,6,7 }, {  }, 156335, 8939 },
};

const SpUniformBlockInfo spUniformBlocks[] = {
	{ }, // Null uniform block
	{ "DebugEnvSphereVertex", 120 },
	{ "DebugEnvSpherePixel", 32 },
	{ "DynamicTransform", 128 },
	{ "Pixel", 1088 },
	{ "EnvmapVertex", 16 },
	{ "EnvmapPixel", 8320 },
	{ "Transform", 64 },
	{ "SkinTransform", 64 },
	{ "Bones", 3072 },
	{ "SkinPixel", 1120 },
};
const SpSamplerInfo spSamplers[] = {
	{ }, // Null sampler
	{ "envmap", (uint32_t)SG_IMAGETYPE_CUBE },
	{ "diffuseEnvmapAtlas", (uint32_t)SG_IMAGETYPE_3D },
	{ "shadowGrid3D", (uint32_t)SG_IMAGETYPE_3D },
	{ "visFogTexture", (uint32_t)SG_IMAGETYPE_2D },
	{ "albedoTexture", (uint32_t)SG_IMAGETYPE_2D },
	{ "normalTexture", (uint32_t)SG_IMAGETYPE_2D },
	{ "maskTexture", (uint32_t)SG_IMAGETYPE_2D },
	{ "shadowGridArray", (uint32_t)SG_IMAGETYPE_ARRAY },
	{ "gbuffer0", (uint32_t)SG_IMAGETYPE_2D },
	{ "gbuffer1", (uint32_t)SG_IMAGETYPE_2D },
	{ "albedoAtlas", (uint32_t)SG_IMAGETYPE_2D },
	{ "normalAtlas", (uint32_t)SG_IMAGETYPE_2D },
	{ "maskAtlas", (uint32_t)SG_IMAGETYPE_2D },
};
const SpAttribInfo spAttribs[] = {
	{ }, // Null attrib
	{ "a_position", 0 },
	{ "a_normal", 1 },
	{ "a_tangent", 2 },
	{ "a_uv", 3 },
	{ "a_position", 0 },
	{ "a_tint", 4 },
	{ "a_uv", 1 },
	{ "a_normal", 2 },
	{ "a_tangent", 3 },
	{ "a_indices", 4 },
	{ "a_weights", 5 },
};

const char spShaderData[] = 
	"\x28\xb5\x2f\xfd\xa0\x9a\x85\x02\x00\xd4\x8a\x00\xda\x79\xdc\x16\x2b\xb0\x6e\xea\x0c\xf0\x92\x92\x2d\x4a\x88\xd0\x56\x32\x33\xbb"
	"\x91\xd6\x00\x54\xa3\xd4\x9e\xe7\x3e\x99\xd8\x35\x4c\x63\x13\x0e\xd4\x01\x1b\x60\xfe\x1f\xf4\x1f\x00\x7e\x60\x13\x5b\x01\x61\x01"
	"\x69\x01\xb8\x3f\xe2\x84\x78\xeb\x9f\x61\x88\x13\x04\x37\x03\x49\xa7\xf4\x8b\xa7\xb3\x8e\xd8\xf3\x17\x71\x46\xde\x7a\xf0\xcb\xd0"
	"\xc8\x0b\x22\x8a\x38\x0f\x88\x02\x07\xb8\xdf\x4f\x5f\x10\x77\x57\x7f\x17\x8e\xdc\x0e\x6f\xf7\xca\xf0\x1a\x8b\x39\xe2\x5d\xdd\x13"
	"\x30\xc0\xde\x97\x65\x19\xfb\x87\x54\x71\x77\x03\x37\x7e\x5c\xf9\x16\x34\x4d\xd5\x71\xcd\x2f\x0a\xff\x69\xe7\x9d\x9b\xa6\x7a\x48"
	"\x1d\xd6\xdc\x76\xbd\xe7\x42\xae\x01\x34\x13\x74\xd6\x8f\x9e\x87\x1c\xc7\x71\x22\x14\xad\x59\x6f\xfd\x38\xa3\x48\x9c\x0e\xd2\xce"
	"\x5a\xee\xf8\x05\xfa\x51\x9c\xcf\xcb\xb7\xde\x61\xf1\x91\x94\xe2\x74\xac\x5f\x60\x1d\xe7\x57\x78\xb1\x63\xc5\x9e\xc6\xe2\xb4\x5e"
	"\x4a\xea\x7c\x96\x2d\x95\x94\x03\x9a\xa6\x92\x0f\x07\x14\xad\x39\xd6\x8b\x18\xdb\x0e\xf0\xb5\x6f\xf7\x76\xfe\xbe\x69\x32\xbc\x74"
	"\x76\xbe\xdb\x63\x99\x83\x4d\x53\x4d\x34\x95\xf7\x65\xec\x46\x78\x90\xed\xdb\xdf\xb7\xd7\x2d\x5a\xc6\x0f\x1d\x1e\xaf\xae\x14\xbb"
	"\x1c\x85\xc2\x79\xcf\x34\x15\xbe\x64\x89\xbd\x30\x98\x99\xa6\xc2\xee\x03\x2c\x77\x0d\xbc\x9f\x2e\x0c\x32\x4d\xc5\x65\xbd\x80\x4b"
	"\x60\xba\xe3\xe1\x4f\xfd\x6e\x4c\x93\xe1\x45\xdc\x3b\x3c\xc1\x1d\xb6\x70\x12\x70\x33\xd0\xbe\x0a\x7e\xfc\xd6\xf1\x2c\xef\xef\x25"
	"\x9d\x2a\xd6\x3d\x7d\xb7\x8e\x6f\xb0\x7f\xbc\x69\xaa\xec\x71\x8e\x72\x5f\xba\x30\xf8\x3b\xbf\xa4\xd6\xe6\xb9\xdf\x8b\x6b\xa7\x7f"
	"\x7c\xbc\xba\x69\xaa\x7e\xb9\xce\x9e\xdb\xbe\x8c\xe1\x2d\x5f\xb8\xe3\x9a\xc6\xc3\x4f\xa9\xf4\x9b\x67\xaf\xc3\xa0\x80\xe0\x43\x8b"
	"\x07\x71\xf7\x79\x9c\x13\x31\x77\x37\xbc\x79\x8f\x1a\xd5\x4e\x95\x36\x58\xa9\x5a\xa8\xd1\x40\x35\x1c\xb6\x81\xd1\x68\x98\x1c\x93"
	"\x3e\x0d\xda\x90\x1c\x0e\xa9\xa1\x81\x4e\x4a\x4d\x74\x82\x70\x89\x8a\xd6\x79\x52\x5e\xc6\x19\x49\x71\x52\x94\xe2\xe5\xcb\x69\xda"
	"\x48\x74\x42\xce\x3d\x14\x45\x27\xd3\x54\x8e\x69\x2b\xd3\xc8\x06\x95\xaa\x44\xa9\x65\xe0\x45\x46\x64\x9c\x8a\xd6\x49\xb8\x44\x77"
	"\xf1\xce\xfa\x79\x1e\x7a\xfd\x79\xb8\x9b\x7c\x04\xe1\x74\xfe\xfc\xf3\x51\xc4\x47\x6f\xbd\xae\x64\x3a\x29\x8c\x43\x24\xdb\x07\xd1"
	"\xbd\xcd\xaf\xcb\x8c\x78\x2e\xe3\x8c\x78\x46\x28\x22\xe2\x11\x67\x04\x43\x4d\x0a\xf1\xd1\x44\xe1\xf4\x0b\xeb\x0c\x77\x6f\x7d\xc4"
	"\xa3\x78\xea\x79\x78\x5b\xb4\x8f\x77\x1f\x4d\x24\x94\x8d\x93\x66\xff\x50\x8f\xc2\x7a\xac\xef\xe5\x31\x30\x6c\x92\x30\x8a\xdd\xae"
	"\x71\x65\x30\xa6\x75\x48\x8e\x4a\x61\x15\x63\x4c\x49\x81\x59\x4b\x59\xcc\xe1\x60\x54\x8c\x9a\x28\x83\x32\xa8\xf1\x1f\x28\x12\xff"
	"\xb1\x95\x7f\x9c\x9d\x37\x87\xdb\xae\xbe\xde\x7a\xef\x66\x07\x83\x4e\x8a\x31\xd1\x69\x36\xde\x7a\x8a\x6a\xcc\x69\x92\xa8\xc3\x31"
	"\xe9\x5b\x2f\x51\x8e\xc6\xac\x6f\xbd\xad\x34\x1a\x8c\x3a\xa9\xe2\xad\xaf\x15\xeb\xc7\x49\xe7\x8f\xed\x79\x5c\x6b\x11\xcd\xf4\xcd"
	"\x1b\xdf\xa3\xe6\x91\x06\x0e\x0f\x70\x33\x98\x78\x7e\x77\x5f\xc7\x1d\xd3\x50\xd3\xe5\xb9\x48\x76\x7c\x46\xe8\x32\x8a\x53\x46\x12"
	"\xfb\x68\x8c\x64\xfd\x02\x4f\x27\x63\x41\x27\x7d\x04\x4f\x27\x3d\x20\x4e\x08\xe7\xa1\x17\x1d\x07\xe3\x10\xdc\x7d\x5d\x0c\x32\xb2"
	"\x9e\x82\xe2\xc1\xcf\x71\x20\xfa\xdd\xde\x4e\x1b\xcb\xcf\xf9\xe7\x8b\x1e\x47\xcc\xdb\xf5\xf0\xdd\x3e\xfd\x8c\xd3\xfa\xcf\x52\x93"
	"\x38\x9d\xde\xf1\x7e\xdd\x2c\xbc\x5c\x26\xc7\x71\x13\xf9\xf0\x30\x99\x3c\x7c\x34\x99\x3c\xf4\x90\x87\x09\xe6\x61\x18\x06\x52\x29"
	"\x5e\x42\x5f\x27\x75\xee\xb7\x71\xef\x07\xbe\x99\xa0\xb3\xb1\xa0\x26\x85\x7e\xbc\x88\x53\x3a\x1f\x31\xd0\xb3\x40\xc4\x12\x01\x5e"
	"\x9c\xd2\xcd\x0a\xac\xfe\x1e\x2c\x60\x0c\x3c\xd6\x63\x90\x74\x72\x01\x91\x11\x06\x49\xf7\x72\xe4\x15\x61\xb7\x68\x10\x74\x53\x59"
	"\x90\x24\x6b\x6b\xad\x2c\xd0\x87\xfe\xf6\x8b\xfd\x84\xbd\x5b\xf9\x88\x83\xc3\x64\x62\xfd\x47\x14\x87\x47\x84\x82\x02\x8b\x47\xf0"
	"\x07\x1c\x94\x44\x31\xac\x83\xaa\x53\x9d\xd2\x43\xa7\xdd\xf8\xc6\x18\x64\x3d\xf4\x8a\x7f\x78\x40\x20\xd6\x43\x44\x4d\x83\xfe\x9f"
	"\x77\x7a\x87\x41\xd4\x34\x7b\x2d\x9c\xbe\x0f\x93\x8f\x20\x9d\x63\x10\x13\x1a\x34\xa0\x3c\x26\xe2\x33\x89\x34\x73\x32\x3b\x89\x0f"
	"\x9d\x63\x93\x0b\x57\xd6\x96\x4b\x42\x71\xb0\x30\x31\x2a\xdc\x7e\x1f\x7d\x67\xce\x4d\x95\x64\xab\x03\x64\xfb\x2e\x78\x77\xe3\x18"
	"\xae\xe8\x2b\xa1\x71\x07\x2f\x83\xff\xb0\xd5\x43\x0a\x39\x76\x2d\xde\x3f\xa4\x9d\x77\xde\x39\xf2\x3c\xde\x5d\xff\x7c\x38\xad\xef"
	"\x9b\xe1\x38\xa6\xe9\x9b\xab\x26\x2a\x09\x5c\x42\x63\x8c\x6f\xde\xbe\x9d\xc4\x95\xb7\xbb\x47\xf0\x6d\xae\x80\x1f\xe5\xbe\x95\xfa"
	"\xc6\xda\xbe\x9d\x72\x7f\xb2\xd5\x7a\x26\x9a\x44\x11\x86\x66\x91\x57\xfc\xc4\xc1\x97\x0c\xbe\xf5\x11\x39\xd5\xda\xd0\xa0\x1c\x0e"
	"\x3b\xa9\x38\x55\x52\xac\x64\x0c\x0a\xbb\x0f\xec\x64\xd1\xb2\x45\xaa\x56\xdb\xbd\x6b\xbb\xdb\xad\xdb\x9b\x6e\xdd\x96\xb6\x46\x32"
	"\x62\x98\x86\x0b\xc3\xf0\x72\xa1\xd6\xd6\x3a\x8e\x90\xb5\xb5\x8a\xf3\x79\x0b\xf4\xc5\x1c\xf2\xce\x7a\xc9\x99\xc4\x9c\x19\x0b\xd6"
	"\x51\x2b\x54\x7e\xf2\xf0\xe6\x0a\x76\xbe\x20\x09\x29\x0f\x5f\x19\xdb\x79\x92\x6c\x38\x86\xa3\xf5\xec\x2c\x2f\x13\x5e\xb0\xef\x3c"
	"\x4d\x7c\x55\x29\x4b\xc1\x0c\x3f\x91\x70\x09\x5a\x23\x5c\xb8\x2e\xb7\xbf\x28\x22\x91\x48\x24\x12\x89\x42\xb2\xb0\x1e\x79\xc0\xd2"
	"\xb5\x8d\x35\xd6\xf2\x69\x26\x1a\x26\xa5\xb3\x52\x75\x52\x3a\xab\xe2\x23\x88\x4a\x2e\x5e\x8e\xb3\x61\x4a\x4e\xe4\xc7\x49\x80\x80"
	"\xa2\xa9\x2e\x56\xb8\x70\x9c\x1a\x81\x5f\xe4\x79\xeb\xbb\x97\x19\x94\x0d\x67\x0d\x27\xd5\xb4\x5b\x6b\xa4\xfd\x04\x7e\xe0\xf7\x45"
	"\x00\x3f\xca\xef\x1f\xc7\xfe\x05\xbe\x2b\xac\x03\xe2\xdc\x97\xb9\x15\xf2\xe5\x8f\xf2\xbd\x4a\x96\x7d\xf6\x56\xdf\x0e\xc3\x96\xdb"
	"\x2f\x08\x18\xe3\x97\xad\x94\xed\xf2\x63\xbc\x1d\xf8\xe5\x07\x97\xb0\x74\x0e\x63\x5c\xd6\x57\xa5\x1d\xce\xb2\xaf\xa2\x51\x80\x14"
	"\x1f\x3d\xe2\xf4\x38\xe7\x70\xce\xe3\xdc\x5b\xc9\x52\xb0\xcb\x51\xa8\x26\x70\x59\x67\x36\x1c\xfb\x47\xd5\xfb\x88\x7a\xb5\xea\x8f"
	"\x2e\xe2\xe4\xa8\x7e\xc5\x8f\x54\x3b\xbb\xea\x86\x51\x3b\x1d\xe7\x3a\xa3\xba\xc1\x39\x92\xf1\x76\x5d\x82\x87\x0d\xc7\xc8\x47\x55"
	"\x64\xa7\xb7\x20\x03\x95\xf3\x1d\xcf\x46\x79\x79\x1e\x6a\xaa\xe7\x09\xad\x8f\x22\x29\xd1\x4c\x9c\xff\x3c\x05\xe8\x82\x77\x1f\xf4"
	"\xbb\x5d\x9c\x04\xfd\x6e\x01\x84\xd2\xa8\x13\x68\xb3\x9c\x94\x0c\x8d\x88\x0c\x00\x21\x00\x05\x33\x12\x60\x50\x70\x60\x30\x14\x4b"
	"\x45\x93\xbc\x35\x79\x13\xc0\xc7\x05\x93\x91\x44\x1c\x0f\xa6\x41\x0e\xa3\x90\x31\xc6\x10\x83\x80\x11\x10\x00\x08\x00\x01\x10\x1a"
	"\x00\x81\x19\x11\x0e\x73\x9c\x53\x45\x58\x1a\xc2\x39\x9a\x6c\xc4\x89\x0d\xc8\x13\x61\xed\x3a\x70\xd6\x1c\x10\xb4\x9f\x12\xcd\xdb"
	"\x83\x2e\x3f\x3e\x98\x60\x28\xfc\x54\x5d\x49\x04\x0a\x65\x95\x9f\x60\x52\xb3\xf6\x3a\x66\x74\xe8\x66\xc1\x28\x17\x84\x61\x84\xa6"
	"\x55\x18\x37\xb6\x90\xaf\xe2\xca\x8a\x84\x76\x45\xcc\xd7\x48\xcf\x28\x46\x3e\x1d\xc3\x14\xdd\x3b\x8b\xff\x56\x9f\xad\x38\x58\xf0"
	"\xbb\xd3\xa5\xbe\x7d\x76\x00\xf2\x90\x4b\xe3\x19\x11\xef\xea\x47\xb1\xf8\x08\x77\x61\x60\x8c\x8a\xa9\xa1\xe9\x06\x57\x3a\xf1\x2b"
	"\x58\x52\xe8\xf5\xd7\x52\x16\x0f\x5f\x06\x50\x75\x93\x2e\xb0\x75\xef\x1d\xdf\x56\xca\x3a\x0c\x20\x7f\x58\xa9\x37\x29\xad\x3a\x10"
	"\x51\x2d\xb2\x4d\x8e\xfe\x15\xef\x96\x4c\xef\x48\x42\xc3\x35\xc9\xfe\x18\x42\x99\x86\x09\xdf\xc0\x0e\x29\x41\x8b\x42\x74\x25\xc5"
	"\x75\x96\xb3\xfe\x29\xc9\x0c\x49\x69\xca\x7b\xfa\x68\xb4\x28\x44\xda\x0a\x9e\xc8\x25\x0b\xde\x59\x39\x5c\x44\xae\x1f\xfc\x1d\x04"
	"\x98\x17\xa0\xba\xba\x1e\x35\x02\xd9\xb8\x9d\xa2\x8c\x04\x7f\x21\x92\x7f\xe3\xe8\x66\x23\x9c\x39\x3d\xfd\xc6\x22\x5b\x3d\x2e\xae"
	"\xe6\x07\xa1\x4b\x88\xd4\x93\xc9\xd5\x28\x17\x36\x16\xea\x05\x82\x29\xd4\x39\x97\x14\x54\xfb\xcd\xaa\xd3\xbd\xfa\x7d\xc2\x4e\x9f"
	"\xb2\x9a\x26\x2f\xef\x78\xed\x1d\x94\x24\x61\x78\xb5\x68\x11\x2e\xe1\x18\xa7\xa3\x7b\x89\xfb\x89\xe3\x2e\xbc\xb1\xd1\x36\x85\xdd"
	"\x80\xb0\x60\xb9\xc2\xf0\xe5\x6e\xa5\x38\x2d\xba\x28\x89\x76\xa8\x12\xc7\x65\x77\xa2\x2c\x1c\xf5\xfe\x37\x0b\xfe\x8f\x04\xdf\x31"
	"\xf0\x2c\x59\xf7\xcc\xa6\xd4\x67\xec\x18\x8f\xac\x48\x92\x66\x79\x9d\xa2\x65\x20\xeb\x32\x54\x55\x27\x05\x37\xde\x8e\x65\xd9\x46"
	"\x08\xe7\x8a\x55\xbb\xb8\x6d\xac\xa6\xc0\x39\xd2\xee\x00\xe0\xc8\xc5\xdd\x08\xb5\x63\xfb\xb0\x32\x72\x38\xba\x22\x51\xe5\x11\x19"
	"\x4d\x3e\x92\xab\x40\xf1\x4d\x09\xa6\xb6\xa2\x54\xb9\x4f\xb4\xa8\xa8\x1e\x30\x9b\xeb\xca\x56\x39\xa3\x31\x47\x26\x5e\xe8\x57\x4e"
	"\xd9\x26\xf8\x3e\x1d\xfd\x96\xca\x86\xcd\xec\x21\x48\x69\xaa\xa8\xdc\x9c\xca\x42\x97\x32\x0f\xe5\x94\xa6\x3b\x1f\xf3\x80\x42\xb7"
	"\xe5\x25\x50\x79\xd7\x44\xc4\x39\xa3\xd5\x8d\x9c\xb6\xae\x54\x27\xcc\x49\x4f\x4d\xa0\x2d\xfe\x1a\x7d\x66\x75\x59\x56\x94\x73\xf9"
	"\x16\x86\x82\xfa\x5e\x77\x35\x24\xad\xcc\xd4\x2a\x28\xf5\xe2\x90\x37\x70\xd1\x81\x0e\xfc\x5c\x2c\x13\x8b\x3e\x45\xa2\x70\xc7\x68"
	"\x17\x53\xb1\x41\xee\xe7\x9a\xd2\x15\x32\xf3\x57\x24\x93\x94\x50\x4c\xb4\xfa\x84\x1f\x49\xb9\x12\xe0\x00\xd1\xe8\xa6\xed\xa9\x13"
	"\xf6\x88\xa6\x86\xe0\xd4\xf8\x4a\xe4\xe5\x84\xee\x75\xc2\xc5\x7f\xb7\xf9\x33\x9a\xfa\xf9\x2e\x05\xb3\x47\x6b\x36\x35\xfb\x5a\xeb"
	"\x49\x6f\x3a\x8b\x36\x69\x63\x49\x89\x08\xa2\x8a\x1f\x8a\x4f\x1a\xe7\x81\x2e\x70\x1b\x3b\x53\xbc\x67\x65\xc2\x44\x85\x1b\x8a\x77"
	"\xcc\x7e\x45\xe8\x94\xc2\x19\x59\x66\xef\x12\xe8\x27\x7e\x88\xf9\xcb\x29\xc7\xb1\xcf\xa7\xd4\x68\x20\x79\xf6\xe5\x7e\x6e\x9e\x81"
	"\xd9\x31\x34\x53\xea\x33\xca\xdf\x80\x72\xde\x71\x6d\x77\x5e\x03\x60\xc2\x71\xf2\xb9\xa7\x0f\x21\xc8\x30\x98\x1e\x64\x99\xa4\x05"
	"\x54\xf5\xe6\x50\xcb\x6d\x05\x9f\xdf\xcc\xb3\x42\x7e\x98\xe8\x18\xd7\x0f\xd9\x39\x9c\xc4\xc5\x52\xfe\x39\x8b\x25\xba\x7a\x3d\xf0"
	"\x63\x8c\x44\x57\x97\x04\x0a\xdc\xff\xcb\x84\x08\xfd\x43\x3b\x19\xa8\x1f\xbb\xcf\xe8\x60\xee\xf4\x40\xf9\x96\x79\x51\x99\x02\xaf"
	"\x5a\xce\x16\x9b\x75\xcc\x63\x00\x34\x0a\x64\x22\xd1\x76\xb0\x55\x09\x9f\x1e\x61\xd9\xd4\x95\x2f\x75\x4e\x07\x58\x3b\x05\x2b\x81"
	"\x40\x70\x13\xa6\xc7\xea\xdf\xdb\x08\x3a\xf1\xf3\x33\xd5\x22\x1c\x3a\x22\xd8\x44\xd2\x7c\xd8\x98\xf2\x2e\x87\x6c\xb5\x81\x81\xb9"
	"\x95\x24\x70\x04\x6f\xa6\xe5\x06\x35\xe3\x7a\x3e\xd1\xe3\x6e\xee\x90\x3f\x1f\x30\xb9\xf0\x50\x9b\x9b\x35\xf9\x25\x29\x66\x66\x2e"
	"\xe2\x09\x6c\xb2\xbc\x15\x92\xee\x15\x35\xb0\xf6\x0c\x97\xad\x21\x64\x51\x58\x39\xaa\xc6\x89\x89\x8c\x26\x8c\x01\x88\xab\x60\x30"
	"\xb2\x0b\x72\xe3\x55\x67\x34\x13\xcf\xc8\xfd\x84\x95\xae\x51\x02\xe7\xa0\x0e\xe1\x43\x1d\x7d\x9f\x7b\xe7\x90\x62\xf9\x22\x9f\x83"
	"\x61\x4c\xab\x8b\x52\x2f\xd0\xe6\xa0\x84\x08\x53\xd5\x4f\x43\x81\x20\xbc\x45\x3f\x4a\xe3\x94\x40\x6f\xc8\xc1\x6a\x50\x60\xc8\xd2"
	"\x14\x9d\x94\x29\xf0\xb1\xc3\xdc\x87\x58\x44\xc1\x9c\x09\x41\x45\xc8\x3b\xa2\x6e\x56\x84\x9a\x20\x65\x57\x5a\xaf\x2d\x51\x5a\x2b"
	"\xc1\xf3\x95\x68\x1a\xf4\x2e\xd4\x7e\xc2\x6d\x53\x84\xce\x2d\x5f\x09\xb5\xbf\xca\x25\xe4\x3c\x6b\x31\xb4\xb5\x8d\xd0\x9e\xaa\x3b"
	"\xa0\x73\xc3\xa8\x7c\x27\x0c\x12\x9c\x73\x80\xd9\x6e\x78\xef\x56\xd9\xf0\x31\x20\xe5\x5b\x31\xff\xed\xe3\xe1\x25\xd7\x15\x54\x07"
	"\xbf\xf8\x0e\xd9\x8d\x7c\xd0\x98\x0e\x82\x07\x11\x7a\x7a\x93\x6f\x03\x21\xf2\x27\xaa\xce\xc5\xe5\xd1\x81\xd8\x75\x9c\x50\x61\x3a"
	"\x82\x01\x02\xda\x76\xff\x20\xf7\xbe\xdb\x43\xe7\x43\x44\xe0\x8a\xb8\x8a\x11\x8a\x2d\xb0\x8e\xb3\x8f\xfe\x73\xa2\x06\x9d\xca\xe8"
	"\xca\x89\xb6\x68\x73\xc9\xc6\x4e\xd9\x70\x8c\xc1\x19\x83\x69\xcc\x47\xeb\x9f\x09\x0b\x86\x31\x04\x9d\xba\xbd\xd4\xb5\x01\xb7\xb5"
	"\x6f\xbb\xd0\x6d\x47\xb0\x34\x11\x6c\x45\x99\xdb\x79\xf6\x1f\xba\xbc\x47\x39\x55\x1f\xdc\x4e\xda\x73\x0f\xc8\x72\xda\xc7\x58\xf2"
	"\x20\x05\xab\xbd\xd1\x74\x3d\x0d\xfa\x02\x95\x7a\x9c\xb4\x74\xcb\x1b\xf7\x3d\xca\xd2\x46\x98\x69\x2c\x3b\x38\xe4\x95\x51\x23\xa6"
	"\xc4\xc3\x38\xef\x0c\xc3\x09\xb0\x85\x2e\x2d\x4f\xeb\xb2\x65\xd2\xe0\x97\x29\x25\x2d\x63\x01\x83\xd3\xa6\x39\x4b\xea\x48\x18\xe1"
	"\x4a\x95\xc8\x1c\xcf\x15\x4f\x0d\x55\x51\x74\x4c\x9d\xb2\x4e\xe5\x79\xa0\xd9\x01\x7d\x0a\x4d\x88\x52\xcf\xe7\xb7\x90\x1e\x7b\x6c"
	"\x81\xc7\x54\x70\x54\xe8\x2c\x9f\x44\xa8\x68\x2b\x29\x3b\x52\x41\xeb\x33\x12\xbc\x5d\x70\xda\x10\x44\x2f\x78\x54\xbb\x38\x3b\x21"
	"\x37\x4c\xf8\x48\x96\x90\x09\x0c\xa4\x65\x46\xd5\x72\x7d\x6a\xa5\x6b\x9b\x9b\x2d\x5f\x8a\xf9\xb5\x14\xbc\x40\xcb\x4a\xe9\x04\x05"
	"\x4f\xe5\x10\x75\x31\x7e\xf9\xe2\xa5\x15\xe8\x69\x74\xf1\x4f\x99\x6f\xd1\x03\xe9\xa7\x2f\x0c\xac\x77\xfb\xc3\x08\xee\x5c\x82\xdb"
	"\xcb\x6f\x3c\xdb\x01\xc1\x9a\xf4\x71\x51\x27\x44\x42\x39\x87\x35\xa0\x12\x90\x40\x63\xff\x35\x57\x55\x50\x8c\xa8\x88\x43\x2f\x2a"
	"\x73\xa1\x10\x78\xf4\x99\xd6\x78\xe4\xfe\x71\x26\x99\x23\xeb\xbf\x1a\xeb\xee\xea\x18\xd3\x89\xce\x67\x71\xd3\xfb\x2d\x29\x4d\x63"
	"\x36\x1d\x80\x60\x03\xd0\x1e\xa1\xf7\xa7\x70\x09\x4d\x54\xfd\x61\xd1\x1e\xe1\x43\xea\x48\x61\x28\x99\x5f\xc2\x87\x48\x9b\xb7\x3d"
	"\xb5\x58\x0f\x25\xb0\x3e\x2a\x76\x27\x0e\x34\xa1\x81\x94\x39\x08\x1d\x15\x35\x33\x03\x72\x44\x00\x4d\x3f\xe8\x8d\x4f\x9c\xfd\x64"
	"\xf1\x66\xe8\xdb\xb0\x7c\x1d\x53\x8a\x4d\x26\xbe\x70\xc2\x58\x83\xfb\x1d\x50\x9c\x0b\xb0\x1c\xf2\xd1\x96\x70\x70\x14\x21\xb3\x9e"
	"\xe0\xf4\x79\x0f\xed\x23\x8a\x09\xc3\xe8\x4d\x08\x35\xd3\x25\x7c\x18\x70\xe8\xce\x70\xfc\xb6\x60\x20\x11\x7e\x6a\x01\x80\xd5\x8b"
	"\x14\x1b\x22\x08\x1e\x98\x03\x4d\x81\x17\x6b\x5b\xd1\x85\x78\x04\x03\x26\x0b\xd3\x84\x5f\xa2\xdc\xd3\x20\x81\x00\x81\x2b\xc6\x3a"
	"\x0e\x61\x02\x2e\xa7\xc9\x80\x8e\x11\x44\x6e\xa1\x61\x2c\x0c\x97\x8e\x25\xc4\xda\x76\xea\x41\x58\x42\x30\x25\x25\xc4\xee\xd1\x3b"
	"\x60\x04\x8a\xf8\x06\x00\x2f\xac\xb8\x23\x79\xba\xbd\xd0\x15\x29\x04\x9c\x38\xe1\x06\xa8\x7d\x27\x85\xf8\x14\x11\xf4\x4c\x42\x4c"
	"\x8e\x4f\x42\xa7\x40\x81\xb1\x72\xbe\x28\xdb\x20\xac\x29\x40\x2d\x30\xe3\xdb\xaa\xf0\x87\xf1\xe7\x38\x56\x66\x36\xf0\x62\x82\x22"
	"\xae\x88\xa6\xc9\xe6\xce\x4b\x2f\x71\xc0\xb5\x94\x9d\xab\x86\x0e\x3e\xd1\x8b\x45\x6b\x32\x89\xc8\x3b\x22\x03\x2a\xa3\xd5\x34\x29"
	"\x34\xb8\x19\x36\x57\x93\x2d\x8d\x9b\x23\xbe\xf2\x03\x4b\xae\x67\x51\x5c\x80\xd8\x33\xb6\xb0\x91\x8f\xdf\xd7\xce\x19\x92\x0c\x76"
	"\xd7\x4b\xcb\x84\x36\xe3\xde\xd5\xfe\x36\xee\x47\xca\x11\x05\x94\xf5\x74\x11\x85\xf8\x4c\x50\x09\x22\x95\x67\x1f\x4b\xb5\xd8\x4f"
	"\x41\x19\x85\x1f\xb4\x47\xe5\xf2\x9f\xb6\x7a\xdf\xa8\x0a\x36\xd5\xcb\xb1\x1a\x37\x64\x5c\x9a\x9b\x2b\x7f\x2a\x7c\x42\xd5\x6c\x66"
	"\x3d\xc2\xa5\x0a\x41\xd6\x53\x75\xae\x26\x77\x22\xe3\xfd\x4c\xbb\x2e\xa6\xb1\xe5\x15\x9b\x77\xea\x62\xe3\xe0\xa4\xe1\x3a\x2d\x2b"
	"\x03\x10\x14\x51\x5c\x3c\xc1\xa3\xe6\x20\x5a\xc5\xc8\x66\x02\x6f\x0f\x33\x1b\xbe\xbe\x0a\x78\xc8\x9b\xe8\x94\x55\xe1\x4f\x56\x58"
	"\x27\x40\x94\xff\xf4\x96\xef\x77\x21\x5b\x85\x6e\x68\x86\xd9\x52\x75\x6a\x80\xc7\x2d\x64\xcc\x46\x64\x93\xbf\x37\xe1\x72\xa7\xa8"
	"\x13\x2e\xf7\x0d\xf7\x60\xc0\x78\xdf\x59\x4e\xd8\x00\x54\x80\x44\x66\xfe\x10\xd3\x07\xe7\x49\x59\x7d\x68\x00\x75\x1f\xac\xe0\x09"
	"\x9e\x4b\x6f\x32\x96\x2b\x06\x05\x91\xa1\xb0\xdb\x0d\xd7\x43\xe6\x32\x53\x7e\xfc\x26\x5d\x7e\x37\x88\x40\x45\xbb\x22\xb7\x2d\x12"
	"\x96\x84\xd7\x4b\xa4\x41\x6e\x4a\xa1\x7f\x6f\x8f\xf5\x54\x2b\x08\x87\x34\x39\x52\x26\xbc\x49\x08\x94\x0a\x05\xf6\x70\xea\x30\x89"
	"\xd6\x0b\x35\x5e\x86\x60\xcd\x21\x4d\xae\x15\x79\x1e\x32\xf8\x62\x50\x0e\x80\xf7\x42\x23\xaa\x2d\x71\x03\xd4\x6d\x30\xbc\x8e\xff"
	"\x9f\xd6\x6b\x46\x6d\xc2\x21\x92\x5f\x23\xc7\x81\x8e\x74\xc0\x47\xbb\xc3\xeb\x66\xf1\xd8\x87\xd9\xf8\xc8\x0c\x83\x19\xae\x59\x37"
	"\xe0\xe8\x54\x57\x1d\xac\x35\x4b\x13\x88\x17\xc5\xc3\x80\x10\x70\x44\xf5\xd9\x12\xd1\x02\x63\x48\xba\xa1\xa4\x74\xb2\x24\xbb\x57"
	"\x56\x69\xa6\x13\xdb\x73\x17\xda\x32\x98\x08\x02\x64\xb5\xcf\xa9\x64\x91\x44\xc4\xd7\xe6\x39\xef\x51\x6b\xd6\x72\x3c\x3f\x95\x7f"
	"\x1a\x51\x7a\x0b\x93\xd0\xde\x61\x84\xf4\xdb\xa3\x03\x13\x7e\x20\x1a\x11\x5d\x33\x22\x84\x13\x9f\x1d\xd7\x9c\x5e\x69\xc4\x6f\x42"
	"\x48\x3c\x9d\xe4\xd6\x2f\xbe\x1a\x7a\xd9\x30\xb8\x56\xf3\x57\x3c\x7c\x83\xf3\x14\xd1\x9e\x85\xf7\xf4\x31\x9f\x88\x1c\x64\x42\xe5"
	"\x44\x3f\x3c\xbf\x13\x64\xa0\xef\x84\xdb\xf4\x1c\x1f\xe0\x9e\x2e\x46\xe9\x28\xf9\x13\x94\xa2\x09\x36\xd1\x38\x66\x3f\x90\x5a\x92"
	"\x23\xd7\xce\x38\x5e\x00\x66\x47\x23\xe2\x18\x64\x72\x2d\x76\x2a\x07\x58\x6d\x61\x4e\x0e\x0c\xed\x28\x39\x63\x90\xc3\x2c\x4a\x17"
	"\xf8\xef\xfd\xb4\xa3\x03\x7e\x97\x5f\xb0\x6b\x44\x53\xe2\x7f\x33\xdf\xf8\x29\x3d\xf9\x38\x02\x90\xd2\xe0\x2e\xd4\x49\x47\x0e\xe3"
	"\xc6\x65\xf2\xa5\x63\x46\x8f\x42\x82\x66\x6b\xb0\xf3\x16\xb2\xeb\x99\x5c\x8d\x48\x9e\xb1\x33\x51\xcd\x89\xfa\x56\x9d\x39\xfc\xf2"
	"\x03\x65\x98\x02\x12\xf8\x99\x95\x2f\xec\xb8\x13\xe5\x3b\x50\xf0\xd5\xe7\xff\xbe\x04\x28\x2c\x9c\x92\x1b\x71\x2d\x95\x08\xac\x41"
	"\xe6\xf8\xee\x3b\xb4\x2b\x3d\x0e\x7e\x76\x5b\x3c\x73\x4c\xfc\xde\xf8\x1c\x95\x4c\x48\x43\x3e\xaf\x8d\x5e\xb7\xbd\xb8\x44\x6f\xd4"
	"\x25\x85\xb5\xc1\x60\xe1\xa0\x04\x21\xad\x54\xd5\x4d\xf9\xca\xcf\x6c\x6a\x80\x0a\x55\x6b\x4d\x94\x4a\xf3\x86\xdc\x59\x5f\x76\x52"
	"\x5b\x41\x05\xb6\xbe\x4d\xf6\xbc\xc1\x31\xac\xb0\xb1\xc0\x45\x72\x6b\x63\xb2\xb8\x00\x97\x4d\x6e\x02\x7a\xed\x0b\x06\xcf\x00\x9b"
	"\x1e\x74\xee\x27\xa9\x45\xb0\xe9\x90\x41\x45\xfb\xf7\x36\x32\x12\xa2\x0c\x4c\xa1\x11\x7d\x6b\x9d\x52\x67\xd0\xba\xe1\x1f\x39\x98"
	"\x35\x34\x42\x37\x6e\x76\xde\x94\x86\x47\x01\x3f\x68\x2a\x25\x5d\x03\xc9\x17\x84\x6f\xc1\x04\xc0\x0e\x13\x4d\xf7\xc7\xa0\x41\x9c"
	"\x4e\xc8\x06\x25\x4c\x6b\xdd\x23\x7b\x6a\x52\x9c\xd0\xf0\xa8\x55\x1b\x91\x55\xd1\xea\xd5\xce\x0b\xf2\x77\x8e\x45\x6e\xec\xdd\x2d"
	"\xfc\xbc\x5e\x20\xbd\xc1\xe2\xeb\x19\x12\x05\x42\x46\x86\x9f\x65\x25\x78\x30\x7a\x32\x8c\xd4\xcc\xd6\x9c\xb2\x23\x5d\x04\xd5\x47"
	"\x74\xb2\x45\xe3\xbc\x14\xf4\x7e\x72\x7f\xab\xcf\x33\xca\xb1\xe2\x87\x47\x85\xa9\x54\xbb\x59\x70\x84\xa7\x83\xbd\x8c\x8e\xaa\x7c"
	"\x70\xfc\x95\x42\x42\x50\xcb\x1d\x93\x6b\x78\x8f\x63\x50\x3d\xe1\x26\x64\x9a\x12\xfa\xca\x3d\xd7\xde\xb7\xed\x45\x79\xba\x7a\xd1"
	"\x37\xfc\x4c\xca\x00\x9e\x46\xa3\xec\xeb\x38\x11\x73\xff\x42\xcd\x83\xc0\x98\x30\x20\x01\xfb\x73\x19\xc9\xe1\x73\xfa\xd1\xe2\x93"
	"\x08\x35\x75\xa7\x62\x75\x61\xc0\x6d\x8f\x52\x08\xda\xc4\x8b\x88\x43\x24\xac\x5d\xda\x58\xa1\xc4\x2d\x07\x97\x85\xec\x03\x26\x79"
	"\x64\x98\x5b\xe4\x85\xf4\xa7\x92\x9d\x85\x1d\xe2\x86\x38\x86\x65\xd9\xfd\xe2\x09\xc3\x60\x98\x60\xc0\x9c\xdf\xf8\x2d\x03\x04\xe2"
	"\xdb\x5a\x90\x6e\x51\xed\xe0\x18\x2d\xac\x7b\x31\x02\x5c\xe5\x88\x50\xbc\xfd\x18\x3d\x1e\xf1\x87\x59\xbc\xf1\x3e\x45\xda\x2a\x94"
	"\x66\x52\x25\x69\x7c\xb4\x42\x9b\x04\x68\xf3\xc1\xd8\xa1\xb1\xa5\x74\x6f\x0b\x46\x39\x49\xaa\xb6\x52\x94\x62\x13\xee\xe6\x77\xc5"
	"\x32\x7a\x36\x9b\x16\x15\x00\x04\xf6\x01\x3a\x3f\x65\x9b\x80\xd3\xa2\x12\x41\x65\x89\x37\xf0\x94\xdb\x51\x2d\x5b\xd2\x72\x17\x95"
	"\xcd\xcd\x99\xa2\x75\x6c\xee\x60\x06\xbc\xfd\x80\x62\xc8\xc6\x47\xa8\x16\x0a\xf2\x2b\xd5\xe5\xfa\x92\x43\xae\x43\xf9\x3b\x90\x0d"
	"\x86\xc2\xdc\xcf\xc4\xe5\xe2\xc7\x79\xe5\x67\xe4\xc4\x3d\x7a\x68\x4a\x25\xd1\x9c\xfb\x83\x95\xe6\xa6\xa0\x63\x0c\x17\xd6\xb4\x44"
	"\x2a\xe3\xab\x99\xbc\xfd\xc5\x02\x00\x40\x7b\x68\x68\x79\x79\x77\x79\x77\x17\xf0\xbb\xa1\x8a\xee\xee\x22\xd4\x73\x62\x9b\x80\xfe"
	"\x12\x57\x8d\x85\xee\x01\xd5\x9d\x75\x21\xb6\x0a\xb5\xb8\x58\x2a\xf6\xbb\x8a\x19\xc3\x07\xd2\x63\x60\x67\xae\x85\xb1\x85\xe5\x55"
	"\xb6\x05\xf6\x4b\x97\xda\x15\x27\xba\x12\x7b\xe3\xae\x37\xb4\x9c\xe4\x82\xa6\x80\xc0\x0e\xa1\xdf\xa1\xa4\xf8\xcf\xe5\xe5\xfd\xc5"
	"\x58"
;