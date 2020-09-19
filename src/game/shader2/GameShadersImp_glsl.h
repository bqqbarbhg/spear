#include "GameShaders_glsl.h"
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
			{ {0,2}, {2,2} },
			8, 4,
		} }
	},
	{
		"TestMesh",
		{ {
			{  },
			12, 1,
		}, {
			{ {0,2}, {1,2} },
			13, 4,
		} }
	},
	{
		"TestSkin",
		{ {
			{  },
			17, 1,
		}, {
			{ {0,2}, {1,2} },
			18, 4,
		} }
	},
};

const SpPermutationInfo spPermutations[] = {
	{ { 1 }, {  }, { 1 }, 0, 1103 },
	{ { 2 }, { 1,2 }, {  }, 1103, 3824 },
	{ { 3 }, {  }, { 1,2,3,4 }, 4927, 740 },
	{ { 4 }, { 3,1,2,4,5,6 }, {  }, 5667, 7871 },
	{ { 4 }, { 3,1,2,4,5,6 }, {  }, 13538, 7871 },
	{ { 4 }, { 7,1,2,4,5,6 }, {  }, 21409, 8094 },
	{ { 4 }, { 7,1,2,4,5,6 }, {  }, 29503, 8094 },
	{ { 5 }, {  }, { 5 }, 37597, 318 },
	{ { 6 }, { 3,2,8,9 }, {  }, 37915, 6332 },
	{ { 6 }, { 3,2,8,9 }, {  }, 44247, 6113 },
	{ { 6 }, { 7,2,8,9 }, {  }, 50360, 6555 },
	{ { 6 }, { 7,2,8,9 }, {  }, 56915, 6336 },
	{ { 7 }, {  }, { 1,2,3,4,6 }, 63251, 1317 },
	{ { 4 }, { 3,1,2,10,11,12 }, {  }, 64568, 7912 },
	{ { 4 }, { 3,1,2,10,11,12 }, {  }, 72480, 7912 },
	{ { 4 }, { 7,1,2,10,11,12 }, {  }, 80392, 8135 },
	{ { 4 }, { 7,1,2,10,11,12 }, {  }, 88527, 8135 },
	{ { 8,9 }, {  }, { 1,7,8,9,10,11 }, 96662, 1527 },
	{ { 4 }, { 3,1,2,4,5,6 }, {  }, 98189, 7871 },
	{ { 4 }, { 3,1,2,4,5,6 }, {  }, 106060, 7871 },
	{ { 4 }, { 7,1,2,4,5,6 }, {  }, 113931, 8094 },
	{ { 4 }, { 7,1,2,4,5,6 }, {  }, 122025, 8094 },
};

const SpUniformBlockInfo spUniformBlocks[] = {
	{ }, // Null uniform block
	{ "DebugEnvSphereVertex", 120 },
	{ "DebugEnvSpherePixel", 32 },
	{ "DynamicTransform", 128 },
	{ "Pixel", 1072 },
	{ "EnvmapVertex", 16 },
	{ "EnvmapPixel", 8320 },
	{ "Transform", 64 },
	{ "SkinTransform", 64 },
	{ "Bones", 3072 },
};
const SpSamplerInfo spSamplers[] = {
	{ }, // Null sampler
	{ "envmap", (uint32_t)SG_IMAGETYPE_CUBE },
	{ "diffuseEnvmapAtlas", (uint32_t)SG_IMAGETYPE_3D },
	{ "shadowGrid3D", (uint32_t)SG_IMAGETYPE_3D },
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
	"\x28\xb5\x2f\xfd\xa0\x47\xfc\x01\x00\x35\x7f\x00\x3a\x73\xa8\x15\x2b\xb0\x6e\xea\x0c\xf0\x92\x92\x2d\x4a\x88\xd0\x56\x32\x33\xbb"
	"\x91\xd6\x42\xb0\x04\xa9\x39\xcf\x7d\x32\xb1\xbb\x4e\x63\x13\x0e\xd4\x01\x1b\x60\xfe\x1f\xf4\x1f\x00\x7e\x60\x13\x49\x01\x4e\x01"
	"\x56\x01\x22\xee\x8f\x38\x1f\xde\xfa\x67\x18\xe2\x04\xc1\xcd\x40\xd2\x29\xfd\xe2\xe9\xac\x23\xf6\xfc\x45\x9c\x92\xb7\x1e\xfc\x32"
	"\x34\xf2\x7a\x88\x24\x0e\x24\xa2\xc0\x01\xee\xf7\xd3\x17\xc4\xdd\xd5\xdf\x85\x25\xb7\xc3\xdb\xbd\x32\xbc\xc6\x62\x96\x78\x57\xf7"
	"\x04\x0c\xb0\xf7\x65\x59\xc6\x1e\x22\x55\xdc\xdd\xc0\x8d\x1f\x57\xbe\x05\x1d\xd6\xdc\x76\xbd\xa6\xa9\x3a\xae\xf9\x45\xe1\x3f\xed"
	"\xbc\x33\x80\x66\x82\xce\xfa\xd1\xf3\x90\xe3\x38\x8e\x84\xa2\x35\xeb\xad\x1f\x67\x14\x89\xb3\x41\xda\x59\xcb\x1d\xbf\x40\x3f\x8a"
	"\xf3\x79\xf9\xd6\x37\x2c\x3e\x92\x52\x9c\x8e\xf5\x0b\xac\xe3\xfc\x0a\x2f\x76\xac\xd8\xd3\x58\x9c\xd6\x4b\x49\x9d\xcf\xb2\xa5\x92"
	"\x72\x40\xd3\x54\x12\xe2\x80\xa2\x35\xc7\x7a\x11\x63\xdb\x01\xbe\xf6\xed\xde\xce\xdf\x37\x4d\x86\x97\xce\xce\x77\x7b\x2c\x73\xb0"
	"\x69\xaa\x89\xa6\xf2\xbe\x8c\x5d\x08\x2f\xb2\x7d\xfb\xfb\xf6\xba\x45\xcb\xd8\xa1\xc3\xe3\xd5\x95\x62\x97\xa3\x50\x38\xef\xfb\xe5"
	"\x3a\x63\x9a\x0a\x5f\x72\x84\xbd\x30\x98\x99\xa6\xc2\xee\x03\x2c\x77\x0d\xbc\x9f\x2e\x0c\x32\x4d\xc5\x65\xbd\x80\x4b\x60\xba\xe3"
	"\xe1\x4f\xfd\x6e\x4c\x93\xe1\x45\xdc\x3b\x3c\xc1\x1d\xb6\x70\x11\xe0\x66\xa0\x7d\x15\x0c\xf9\xad\xe3\x59\xde\xdf\x4b\x3a\x55\xac"
	"\x7b\x95\xbe\x5b\xc7\x37\xd8\x3f\xde\x34\x55\xf6\x38\x47\xb9\x2f\x5d\x18\xfc\x9d\x5f\x52\x6b\xf3\xdc\xef\xc5\xb5\xd3\x3f\x3e\x5e"
	"\xdd\x34\x54\x92\x7e\xf3\xec\x75\x18\x14\x10\x95\x36\xac\x54\x2d\xe4\x68\x50\x8e\x87\x6d\x30\x1c\x8d\xe9\x31\xe9\xe3\xa0\x0e\xe9"
	"\xf1\x90\x1a\x0d\x3a\x29\x35\xd1\x29\xc2\x25\x2a\x5a\xe7\x49\x79\x19\x67\x24\xc5\x49\x51\x8a\x97\x2f\xa7\x69\x23\xd1\x09\x39\xf7"
	"\x50\x14\x9d\x4c\x53\x3d\xa6\xad\x4c\x23\x1d\x54\xaa\x12\xa5\x96\x81\x17\x19\x91\x71\x2a\x5a\x17\xc1\x25\xba\x8b\x77\xd6\xcf\xf3"
	"\xd0\xeb\xcf\xc3\xdd\xe4\xa3\x07\xa7\xf3\xe7\x9f\x8f\x20\x3e\x7a\xeb\x75\x25\xd3\x49\x61\x3c\x12\xb1\x7d\x0f\xdd\xdb\xfc\xba\x4c"
	"\x88\xe7\x32\x4e\x88\x67\x84\x20\x20\x1e\x71\x42\x30\xd4\xa4\x0f\x1f\x4d\x14\x4e\xbf\xb0\xce\x70\xf7\xd6\x43\x3c\x8a\xa7\x9e\x87"
	"\xb7\x45\xfb\x78\xf7\xd1\x44\x42\xd9\x38\x69\xf6\x0f\xf5\x28\xac\xc7\xfa\x5e\x1e\x03\xc3\x26\x09\xa3\xd8\xed\x1a\x57\x06\x63\xda"
	"\x87\xf4\xa8\x14\x56\x31\xc6\x94\x14\x98\xb5\x94\xc5\x1e\x0f\x46\xc5\xa8\x89\x32\x28\x83\x1a\xff\x81\x22\xf1\x1f\x5b\xf9\xc7\xd9"
	"\x79\x73\xb8\xed\xea\xeb\xad\xf7\x6e\x7e\x30\xe8\xa4\x18\x13\x9d\xa6\xe3\xad\xa7\x28\xc7\x9c\x26\x89\x3e\x1e\x93\xbe\xf5\x12\xf5"
	"\x70\xcc\xfa\xd6\xdb\x8a\xc3\xc1\xa8\x93\x2a\xde\xfa\x5a\xb1\x7e\x9c\x74\x0e\xd9\x9e\xc7\xb5\x16\xd1\x4c\xdf\xbc\xf1\x3d\x6a\x2e"
	"\x69\xd0\x00\x01\x37\x83\x89\xe7\x77\xf7\x75\xdc\x31\x0d\x35\x5d\x9e\x8b\x64\xc7\x67\x84\x2e\xa3\x38\x65\x24\xb1\x8f\xc6\x48\xd6"
	"\x2f\xf0\x74\x32\x16\x74\xd2\x47\xf0\x74\xd2\x03\xe2\x7c\x70\x1e\x7a\xd1\x71\x30\x1e\xc1\xdd\xd7\x03\x4f\xc1\x51\x3c\xf8\x39\xce"
	"\x43\xbf\xdb\xdb\x69\x63\xe9\xe0\x30\x99\x38\x7c\x34\x99\x3c\xf4\x11\x87\x09\xe6\x61\x18\x26\x52\x29\x5e\x42\x5f\x27\x75\xee\xb7"
	"\x71\xef\x07\xbe\x99\xa0\xd3\xb1\xa0\x26\x85\x7e\xbc\x88\x53\x3a\x1f\x31\xd0\xb3\x40\xc4\x12\x01\x5e\x9c\xd2\xcd\x0a\xac\xfe\x1e"
	"\x2c\x60\x0c\x3c\xd6\x63\x88\xe8\xe4\x22\x22\x23\x0c\x11\xdd\xcb\x92\x57\x84\xdd\xa2\x41\xd0\x4d\x65\x41\x92\xac\xad\xb5\xb2\x40"
	"\x1f\xfa\xdb\x2f\xf6\x13\xf6\x6e\xe5\x25\x0d\x0d\x93\x89\xf5\x1f\x51\x1a\x20\x10\x0a\x0a\x2c\x1e\xc1\x1f\x78\x50\x12\xc5\xb0\x0f"
	"\xaa\x4e\x75\x4a\x0f\x9d\x76\xe3\x1b\x63\x90\xf5\xd0\x2b\xde\x01\x12\x89\x58\x1f\x11\x35\x0d\xfa\x7f\xde\xe9\x1d\x06\x51\xd3\xec"
	"\xb5\x70\xfa\x3a\x4c\x3e\x8a\x74\x8e\x3d\x4c\x68\xd0\x80\x02\x99\x88\xcf\x24\xd2\xcc\xc9\xec\x24\x3a\x74\x8e\x4d\x2e\x5c\x59\x5b"
	"\xae\x08\x4a\x83\x85\x89\x51\xe1\xf6\x0b\xe9\x3b\x73\xee\xa7\x4a\xb2\xb5\x21\xb2\x7d\x17\xbc\xbb\x49\x70\x0c\x57\xf4\x7d\x88\xad"
	"\x20\x52\xc8\xb1\x6b\xf1\x1e\x22\xed\xbc\xf3\xce\x91\xe7\xf1\xee\xfa\xe7\xc3\x69\x7d\xdf\x0c\xc7\x31\x4d\xdf\x5c\x35\x51\x49\xe0"
	"\x12\x1a\x63\x7c\xf3\xf6\xed\x24\xae\xbc\xdd\x3d\x82\x6f\x73\x05\xfc\x28\xf7\xad\xd4\x37\xd6\xf6\xed\x94\xfb\x93\xad\xd6\x33\xd1"
	"\x24\x92\x30\x34\x93\xbc\xe2\x27\x0e\xbe\x64\xf0\xad\x97\xc8\xa9\xd6\x86\x83\x7a\x3c\xec\xa4\xe2\x54\x49\xb1\x92\xe0\x43\x8b\x07"
	"\x71\xf7\x79\x9c\x13\x31\x77\x37\xbc\x79\x8f\x1c\xd5\x4e\x01\x56\xdb\xbd\x6b\xbb\xdb\xad\xdb\x9b\x6e\xdd\x96\xb6\x46\x32\x62\x98"
	"\x86\x0b\xc3\xf0\x72\xa1\xd6\xd6\x3a\x8e\x90\xb5\xb5\x8a\xf3\x79\x0b\xf4\xc5\x1c\xf2\xce\x7a\xc9\x99\xc4\x9c\x19\x0b\xf6\x51\x2b"
	"\x54\x7e\xf2\xf0\xe6\x0a\x76\xbe\x10\x11\x52\x1e\xbe\x32\xb6\xf3\x24\xd9\x70\x0c\xc7\x9d\xe5\x65\xc2\xfb\x55\xa5\x2c\xfd\x0c\x3f"
	"\x89\xe0\x12\xb4\x46\xb8\x70\x5d\x6e\x7f\x60\xdf\x79\x5a\x58\x8f\x3c\x60\xe9\xda\xc6\x1a\x6b\xf9\x34\x13\x8d\x49\xe9\xac\x54\x9d"
	"\x94\xce\xaa\xf8\xe8\xa1\x92\x8b\x97\xe3\x6c\x4c\xc9\x91\xfc\x38\x09\x10\x50\x34\xd5\xc5\x0a\x17\xd2\x71\x6a\x04\x7e\x92\xe7\xad"
	"\xef\x5e\x66\x50\x36\x9c\x35\x9c\x54\xd3\x6e\xad\x91\xf6\x53\x78\x01\xbf\x2f\xc2\xef\x1f\xc7\xfe\x05\xbe\x2b\xac\x03\xe2\xdc\x97"
	"\xb9\x15\xfd\xe5\x8f\xf2\xbd\x4a\x96\x7d\xf6\x56\xdf\x0e\xc3\x96\xdb\x2f\x08\x18\xe3\x97\xad\x94\xed\x32\x64\xbc\x1d\xf8\x65\x08"
	"\x97\xb0\x74\x0e\x63\x5c\xd6\x57\xa5\x1d\xce\xb2\xaf\xa2\x51\x80\x14\x1f\x3d\xe2\xf4\x38\xe7\x70\xce\xe3\xdc\x5b\xc9\x52\xb0\xcb"
	"\x51\xa8\x26\x70\x59\x67\x36\x1c\xfb\x47\xd5\xfb\x88\x7a\xb5\xea\x8f\x2e\xe2\xe4\xa8\x7e\xc5\x8f\x54\x3b\xbb\xea\xc6\xa8\x9d\x8e"
	"\x73\x9d\x51\xed\xe0\x34\x17\x31\xde\xae\x4b\x00\xb1\xe1\x58\x45\x76\x7a\x0b\x32\x50\x39\xdf\xf1\x6c\x94\x97\xe7\xa1\xa6\x7a\x9e"
	"\xd0\xfa\x28\x92\x92\x23\xe6\x4d\x9c\xff\x3c\x05\xe8\x82\x77\x1f\xf4\xbb\x5d\x9c\x04\xfd\x6e\x32\xb2\x02\x84\x6b\xa8\x13\x78\xbb"
	"\x9a\x94\x4c\x8d\x88\x04\x40\x14\x04\x01\x53\x12\x60\x50\x78\x68\x30\x13\x8b\x65\x8b\xbe\x23\x1f\x13\x80\x08\x26\x93\x91\x48\x1a"
	"\x0f\xc3\x20\x87\x51\xc8\x18\x44\x88\x41\xc0\x08\x04\x00\x00\x00\x00\x90\x00\x00\x49\x83\x05\x7b\x7d\x5b\x5a\xef\x43\xd4\x05\xac"
	"\x35\xa9\x4a\x83\x28\x58\xd2\x4a\xff\x4b\x91\xc9\x6a\x63\x55\x81\xe4\x8d\x7b\x45\x20\x3c\xa0\xe2\x38\xab\x80\xcb\x82\xca\x8d\xb1"
	"\x52\xc7\x1c\xfd\x73\x7b\xd9\xf8\x1f\x4d\x50\x90\x7b\xfa\xbe\xbd\xb9\x86\x73\x52\x55\x03\x19\xc1\x64\x99\xfa\xd2\x49\xa0\x68\x85"
	"\x40\x06\xb1\x6b\x97\xe0\x86\x09\xbc\x79\x13\xfb\x13\x52\x7b\x14\x93\x1a\xcf\xb3\x84\x79\x45\x08\xb3\xd0\xe8\xe8\x06\x31\x31\xb1"
	"\x00\x47\x24\xb7\x62\xfa\x90\x94\xb7\x8e\xa8\xf4\x38\x1c\x43\x01\xe4\x8a\xd4\x9e\x88\x57\x9e\x04\x3c\xe0\x04\xbd\xd6\x46\x6e\x87"
	"\x5a\xe1\xc1\x85\x5e\xa2\x82\xff\x7c\xd7\xd4\xac\x6b\xe7\xca\xeb\xf9\xa1\x31\x5a\xa2\xb8\x12\xff\x4f\x05\x31\x6f\x47\xb0\xce\xd7"
	"\x50\x50\xfa\xa6\x82\x5e\x79\x95\x9f\x75\x31\x41\xd6\xab\xb8\x03\xb5\x36\x56\xb5\x59\x90\x17\xf3\x1e\xa2\x41\x6e\x23\x17\xae\x6f"
	"\xba\x98\x5d\xf7\xdd\xa5\x40\x7e\xba\x73\x71\xad\x82\x10\xeb\xc0\x86\xc2\x5c\x10\x77\x51\x19\x99\xfd\x66\x1d\x10\x8e\xfc\xb6\x02"
	"\x81\xf0\xa2\x8e\xa9\x39\x5e\xb4\x20\x0c\x8a\x9f\x60\x18\x3d\xd9\x01\xd1\x16\x1c\x20\x5c\xcf\x3e\xc7\x8e\x48\x57\x2b\x5e\x54\x0a"
	"\xad\x69\x59\x0b\x88\x10\xaf\x85\x25\xed\x28\x2e\xb4\xc6\x6f\x55\xfc\x08\x2d\x9b\x28\x75\x15\xde\xf1\x96\x85\xd7\xea\x80\x7d\x0b"
	"\xf7\xbf\xcb\x85\x6d\xfb\x99\x68\xb4\x4c\x74\x39\xce\x2b\xf3\xd8\x29\x34\xb6\x65\x51\x33\xa6\x5a\x39\xc8\x1b\xd0\xf2\xc2\x96\x99"
	"\x0b\x68\x41\xdc\xaf\x72\x2c\x47\xdb\x26\x5c\x9d\xbc\x6c\x62\x29\x9b\x9c\x6f\xe8\x09\x0a\x6f\xda\x5a\xef\xf3\xc0\x54\x4e\x5b\xa8"
	"\x10\x38\xb0\x0f\x76\x85\x1f\x2f\x2a\x01\xb9\x5a\x62\x72\xb1\xad\x97\x9b\x3e\x8d\x83\xfe\x58\x56\x15\x24\x44\x97\x83\x5c\x8f\x9d"
	"\x4b\x7a\x2f\xa9\x68\x41\xf3\x4c\x5b\x67\xe2\x5f\x2c\x1d\xcd\x86\x71\x88\xe7\x75\x8e\x28\x41\x65\x26\x56\xec\x97\x45\xeb\x12\xe5"
	"\x0e\x04\xf9\x15\x02\x82\x49\x01\x62\x9b\x57\xcc\x00\xb0\x5f\x8c\x62\x68\xaf\x97\xf6\x33\x5f\x17\x63\xab\x8d\xbd\x1c\x82\x02\x55"
	"\x5a\x50\x70\x12\x51\x66\x33\xf2\x6d\x5a\x4d\xd1\x3b\xa3\x09\x43\x15\x86\x26\xd2\x31\x72\x75\x99\x11\x0a\x7f\x70\xd8\xbd\xcb\xa8"
	"\x9f\xf4\x38\x24\x16\xa7\x50\xc4\x97\x2c\xab\x89\x80\xa4\x5b\xaf\x81\x59\x3a\x73\xf3\x87\xf4\x4c\xa9\xe7\xa6\x02\x14\x0c\x59\x07"
	"\xed\xc7\x92\x1f\xc1\x38\x8e\x29\x8d\x55\x7c\x9c\xa5\xc6\x1a\x01\x98\x8b\xe1\x4f\x57\x65\x10\x8e\xba\xb7\x64\x7c\x42\x31\xaf\x08"
	"\x69\x37\x0e\x31\x0a\xda\x70\xd8\x70\x74\x1e\x4b\xe4\xe7\xa0\x2c\x45\xd5\xeb\xc7\xd1\x31\xd2\xf5\x2e\xd8\x15\x57\x4b\xc3\x97\xc8"
	"\x3b\x4a\xb1\x3b\x87\x8f\xc4\x84\xd4\xf4\x01\xe1\xe8\x0a\x61\x9f\x6e\x57\x6e\x47\x6d\x38\x1f\x99\x07\x26\xd0\xe8\x3f\x7b\xf0\xa6"
	"\x84\x41\x45\x21\x9d\xc9\x32\xf2\xa9\x7b\x75\x70\x45\x52\x4d\x5a\xaa\x29\x02\x2d\x26\xb0\x1f\xec\xbf\x88\xd4\x13\x3a\x65\x21\xd4"
	"\x4d\x0d\x9d\xe6\x0b\x8d\x82\x97\x60\x6b\x01\x58\x0e\x10\xb5\xf9\x00\x73\x63\x92\x24\x00\x3c\x0d\xc0\x12\x09\xd8\xc5\xf9\x08\x1e"
	"\x77\xd6\x7f\xbe\xf8\x42\x15\x10\xaa\x96\x8c\xf5\xfd\x4b\x5b\xcc\x20\x2d\x22\xc7\x4e\xe2\x4e\xd6\x30\x49\xd6\x49\x9b\xa6\x0d\xe1"
	"\x52\xc4\x15\xa6\x4d\xcd\x8b\x9c\xa5\x22\x18\xce\xa8\xca\x98\x20\x90\x21\xc0\x06\x5f\x41\x8e\x85\x04\xa3\x79\x95\x4a\xbe\x3b\x73"
	"\x79\x0a\xf7\x03\x12\xac\x02\xa9\x08\x2f\xe3\x08\xe6\x48\x46\x54\xa1\x8c\xcc\xc8\xa7\x58\x2d\x8c\x5b\xc5\xa4\x17\xcc\x73\x00\x72"
	"\x59\x5e\xf5\x3b\xa6\xc0\xf1\x69\x17\x8f\xd2\xdc\x5e\x0e\x2f\x76\xa0\xe8\x40\xef\xa9\x15\xc4\x57\x19\x05\x01\xbe\x4c\xf6\xc0\x90"
	"\x0c\xf5\x0e\x30\x95\xd4\x4f\xe2\xe8\x5d\x40\x05\x1e\x5d\xcc\x4b\x40\x61\x42\xc0\xbd\x4a\x60\x02\xd1\x4b\x7c\xc7\xab\x39\xda\x00"
	"\xd4\xfe\xba\x7b\x56\xfe\x37\x58\xb4\x51\xff\x64\x1a\xa9\x07\x36\xc4\x3f\x00\x32\x37\x2b\x10\x95\xa9\x05\x34\x92\x42\x23\x4e\xbe"
	"\x55\x2d\x97\xea\xcf\x9e\xc3\xaa\xfb\xb8\x6c\xc2\xfb\x9c\xe9\x81\xe0\x16\xac\x82\xcf\xfc\x4a\xc8\xdb\x2b\x55\x8d\xfb\xc6\x0f\x5c"
	"\x12\xad\x86\xfe\x2a\x5e\x62\xd2\xc9\x6f\xe8\xcf\x6e\x23\x01\xf3\xeb\x22\x18\x40\x2e\xdb\x9d\x9f\xef\xe3\x65\x96\xce\x4e\x44\xe0"
	"\xfe\x27\x1a\xcd\xaf\x2e\x4b\xa0\x60\x9b\x09\x81\xf2\x70\x71\x5b\x14\xde\x52\x65\x3b\xb8\x69\xa6\x82\xd8\x09\x49\xd0\x13\x5b\x03"
	"\x1a\x7d\x6d\x91\x02\xa0\xe0\xf6\x55\xfa\xa7\x9e\x11\x0f\xe1\x4e\x6f\xe8\xc2\xb8\x00\x59\xd8\xd1\xac\x0d\xa7\xa2\x12\x64\x22\x61"
	"\x33\x4f\xd9\x27\x8e\x06\x38\x76\x54\x4b\x3a\x17\xa5\x2b\xec\xb0\x93\x69\x38\x81\x0c\xac\x96\xa5\xe9\xf0\x42\xd5\x74\x8f\xd8\xc2"
	"\x8d\x91\xc1\x2b\x87\xfb\xe0\x96\xb0\x8e\xe0\x50\xd1\x88\xcc\xc1\x9c\x0d\xed\x4b\x02\xf9\x0c\xca\x34\xc0\x94\xd4\x45\x40\x98\x4b"
	"\x64\xe5\xf2\xdb\x32\x39\xbb\xbc\x42\xf2\x78\xb8\x83\x77\x85\x20\x41\xe9\x74\xe0\x00\x15\xac\x45\xd1\xe4\x2d\xb6\x46\x45\xd1\xb2"
	"\xd0\xd0\x13\xc8\x54\x00\x2c\x84\x83\x4c\xc0\x56\x0a\xec\x38\x01\xb1\x17\x99\xd1\x08\x3f\x68\x48\xf0\x4e\xa3\xc1\xf4\x4b\x39\xb5"
	"\x70\x16\x68\x92\x6a\xc1\x05\x60\x91\xae\x99\x27\xb2\xce\x54\x35\x61\x3d\xf8\x5a\x84\x94\x59\x9d\x33\x1a\x2d\x64\x93\xa4\x30\x78"
	"\x61\x7c\x10\xad\x90\x7d\xd7\xed\xaf\x77\x33\x0c\xb6\x00\x28\x81\xaa\x12\xde\xdc\xdb\x1a\xdf\x42\xe8\x05\xe2\xa7\xc5\x51\x57\xc0"
	"\x0e\x9c\x95\x05\x7d\x6a\x9d\xe0\xcf\x29\x95\xdd\xcc\x51\xdd\x29\x61\x90\xec\x3c\x60\xff\x6e\xd9\xf4\x82\x77\x7c\xd4\xee\x27\x63"
	"\x36\xa3\xa7\x0d\x8b\xa8\x50\x7f\x6c\x4e\xf1\x85\x28\x88\x35\xd0\x13\x91\x67\x64\xc1\xec\x90\x8f\xcb\xd8\xe1\x51\x84\xc2\xfe\x44"
	"\x62\x7e\xdd\xb6\x27\x80\x98\x30\x84\xde\xe4\xa8\x69\xaa\xd4\xc3\x40\x21\x23\x01\xd7\x9b\x08\x83\x0c\xf8\x7e\x0b\x1c\x54\xaf\x4f"
	"\x6c\x39\x44\x90\xc5\x3c\x32\x0a\x0d\x41\x99\x2c\x0d\xcc\x0b\x96\x88\x2c\x9d\x93\xa3\xc4\xfe\x86\x92\x16\x81\x3c\xae\x2c\x92\x57"
	"\x92\x09\xd4\x23\x0d\x58\x74\x4a\x90\xf1\x95\xe0\x9a\x30\xb0\x08\xcb\x07\x6b\x00\xd1\x09\x83\xf0\x3e\x65\x2a\x8b\xfd\xaf\xaf\xc7"
	"\xc0\x12\x99\xba\xa1\xd5\xab\x84\x45\xfe\xeb\xac\x17\xaa\x08\x64\x2b\x17\x09\x8c\x72\x79\x36\xc5\x81\x10\x91\xee\xe4\x87\x80\xba"
	"\xb8\x62\xa1\xa0\x82\x31\xfe\x39\x35\xca\x60\x8f\x1c\x41\x2d\x12\x03\x0f\x9a\xce\xf7\x6a\xaf\x5e\x2b\xd3\x36\x57\x31\x7c\x02\x5b"
	"\xbc\x59\x6c\xe3\xe6\x65\x33\x30\x21\x38\xd2\xfd\x7a\x20\xf1\x19\xb0\x5d\xd1\x72\x96\x10\xdb\xcc\xcd\xc0\x7e\x04\xe8\x1d\x9e\xdf"
	"\x29\x2b\xac\xe7\x64\x7d\xf7\xd1\xf9\xe7\x5b\xe0\xb5\xb9\xf3\xce\xb5\xec\xf0\x3e\x16\x3a\xe2\xf3\xfb\x31\x5a\x84\xde\x82\xab\xbd"
	"\xe8\x53\x4a\xc6\xbb\xbb\x9b\xc4\x44\x00\x48\xf9\x1c\x10\xf3\x8d\x37\x50\x04\xce\x04\x9f\x00\x58\x39\xd8\x23\xbf\x85\xee\xe9\x97"
	"\x8d\xfd\xe0\x74\xe4\x35\x6f\xc1\x16\xfd\xf1\x4a\xb0\x7a\x2f\xd8\xd5\x30\x10\xc6\x85\xbb\x01\xf3\xef\x42\x06\xb8\xcd\x26\x39\x78"
	"\x2a\x1f\x43\xda\xaf\x7a\xb7\xe6\xeb\x94\xce\xc5\x9b\xee\x8b\x86\xf1\xc0\x2e\x76\x74\x6a\x70\x3f\xe1\xf8\xd7\x3a\x96\xa5\xa1\x07"
	"\xb7\x53\x5b\xcc\xe2\x51\x79\xd4\xae\x62\xc1\x41\x8d\x07\x3e\x57\x36\x70\xbc\x33\xbc\x90\xf7\xff\xac\xae\xd8\x4d\x46\x9f\x27\x41"
	"\x56\x67\x8a\x69\x6f\xab\x42\xa6\xbc\x96\x94\x43\xdd\x6c\xb8\x30\xd0\x2e\x0a\xa1\x6f\x17\xb1\x4b\xab\x23\xea\xa8\xc2\x61\xea\x11"
	"\x1b\x54\x97\x48\x1f\xad\xcf\x2c\xc8\xfc\x25\xc2\x0f\xb2\x1a\x04\x20\x3a\x10\x92\xbb\x73\x7f\x87\xbf\xcb\x21\xd4\x16\x20\x51\x7a"
	"\x29\x2e\x9d\xb6\x4b\xc0\x78\xf6\x5c\x52\xaf\x70\x5f\x94\x24\x53\xda\x48\x68\x78\x5e\x03\x94\x42\x35\xde\x62\x36\xa0\x0c\x06\x27"
	"\x9c\x16\x19\xed\xf4\x1b\x9b\xdc\x6b\x40\x42\x54\x1a\x5c\x41\x8b\x60\xe5\x4d\x63\xea\x1b\xe4\x8b\xe2\x84\x58\x7f\xd2\x64\xa0\xe8"
	"\x16\xaf\x03\xeb\x90\x21\x7d\xfd\xf1\xf3\xef\x75\x05\x1b\x5b\xe8\x86\x86\xdb\x61\x73\x9d\xf7\x4f\xde\x5f\x53\x2e\x41\x8e\x24\xb6"
	"\xe2\x7c\xe0\x0b\x30\x39\xa3\x46\xc3\xe5\x66\x78\xc4\x09\xe5\x3c\x5d\xfe\x19\x83\xf5\xca\x09\x08\x02\xb4\x67\x09\x51\x98\x91\xbb"
	"\x82\xb2\x42\xa0\x15\x0c\xb2\x42\x6a\xed\x7e\xc2\x87\x1c\x34\x8d\x0f\xd7\x0b\x95\x78\xd6\xc5\x2d\x1b\x4d\x9e\x38\x63\xa1\x0b\xd3"
	"\xc0\xc2\x75\x50\x27\x8f\x4d\xd1\xab\x21\x2c\x19\x27\x97\xea\xaf\x72\x84\xa6\xee\xe7\x9d\x6f\x14\xd9\xed\x4e\x42\x07\x87\xb3\xd2"
	"\xb1\x47\x87\x71\xf9\x41\x2f\x44\x2c\x60\x24\x1e\x7c\x75\xb8\xaf\x28\x7a\xdd\x8b\xdf\x03\x62\x79\x5a\x37\x58\x3f\xfa\x6a\xc2\xb2"
	"\xec\xe0\xb2\xc6\x5f\x10\xf8\xe2\xe5\xe9\x5d\x7b\x32\xe8\xcc\x9c\x3f\x71\xce\x70\xa4\xca\x53\xc6\xc0\x39\x27\xce\xaa\xb2\x85\x53"
	"\xf7\xdc\x02\x50\xbb\xae\x25\xa3\xdb\xff\x43\x3a\x45\xb3\x98\x48\x1a\xa3\x20\x90\x01\x52\x92\xeb\x7c\x1c\x39\x54\x66\xa3\x11\xee"
	"\x18\x12\x4a\x8b\xe8\xb8\xb5\x15\x9a\xd3\xd6\x03\x89\x16\x96\x10\x32\x4c\x47\x2d\x2c\x5d\xfc\xdf\xfd\x24\x51\x03\x8e\x9a\xef\xf0"
	"\x1b\x5b\x03\x29\xfe\xcf\xd2\x0e\x3c\xfa\xd3\x61\x02\x38\xd7\x14\x5c\xf0\x93\x2e\x0c\x26\xef\x97\x11\x28\x47\xd6\x25\x0a\x87\x1d"
	"\x5e\xa6\xfa\x39\xe4\x00\x2d\xf2\x41\x51\x5c\x55\x7d\xaa\x16\x27\xf5\x41\x9d\x69\xe3\xe4\xb7\x64\x2c\x05\x74\xfb\xb1\x95\x8f\xed"
	"\xb8\x07\xe5\x0c\x50\x02\xd0\xe7\xa8\xbd\x68\x3d\xac\x44\xc9\x13\xc0\x8f\xfb\x04\xd6\x89\x39\x21\xf7\xc5\xec\x9a\x84\x43\xa6\x2d"
	"\x9b\x9e\xfa\x41\xbc\x5b\x7c\x34\x90\x61\xda\xc1\xa2\x6b\x3f\xd0\xbd\x5c\x5c\x62\x30\xea\xfc\xb0\xf0\xbb\xa2\x71\x29\x31\x53\xe8"
	"\x54\x1b\x80\x1c\x91\x6d\xf4\x08\x0a\x6a\x50\xcb\xfc\x2a\x95\x56\xa4\xbd\xf3\xb4\x94\xa7\xb6\x6d\x09\x2c\x7d\x9f\xcc\x72\x03\x03"
	"\x5a\x61\x21\x51\x8b\x04\xd7\xea\x65\x91\x09\x94\xdf\x45\x4d\x34\xee\xab\x02\x6f\x26\x36\x3d\xe3\xe8\x87\x76\x8b\xf2\xc3\x21\xef"
	"\x8a\x78\xe0\x8d\x66\x30\x82\xb2\x4d\x71\x22\x77\xd6\x12\x69\x9c\xd3\x75\x11\x1f\x5e\x31\xa9\x34\xe6\x35\x66\x33\x2f\x8e\x2e\x4d"
	"\xb1\x1c\x64\x32\xca\x71\xdb\x19\x1e\x40\x58\xac\x02\x40\x05\x3a\xaa\xfb\x40\x6f\x90\x0e\x32\xbf\x81\x76\xc1\x50\x9b\xd4\x8e\xb6"
	"\xf4\x58\x6f\x50\xb0\x9e\x91\x5f\x52\x56\x3d\x95\xcd\x47\x56\x95\x23\xb5\x14\xe6\xbb\x5d\xfe\x22\x23\x08\xc1\x62\x0e\x9e\x27\xaf"
	"\x70\x8a\x54\xe8\xac\x01\xc4\x63\xec\xe6\x10\xec\xcc\x24\x10\x95\x3f\x32\xf5\x41\x3f\xe8\xb2\x0d\x5d\x2c\x4a\xf1\xbd\x1f\xcf\x77"
	"\x97\x43\xaa\xdc\x24\x7e\xac\x56\x00\x95\xea\x08\x07\x07\x78\x3a\xe4\x94\x51\x99\xca\xf7\x81\x4b\x81\x23\x89\x80\xf0\x0f\x89\xa0"
	"\x40\x08\x4a\xfc\x7c\x0b\x98\x8c\xd6\xa0\x4e\x19\x01\xd0\x9d\xdc\xce\x21\x4f\x17\xd0\x65\xbe\xaf\xa5\xea\x61\x92\x26\xab\xea\x43"
	"\x3a\x51\xf6\xa4\xc6\x23\x1a\x57\xc4\x49\x32\x0a\xc9\xa4\xd3\xb3\xc8\x60\x00\x0b\xb9\x6b\x82\xc4\xbd\x9f\xad\x64\x78\x77\x2d\xc8"
	"\x90\xa3\xa5\xef\x0d\x1c\x23\x28\x17\xb5\x02\x85\x22\x40\xaa\x2f\x19\xf9\x80\x97\x95\x91\xf0\xb6\xc8\x55\x2e\x3e\x97\xff\x87\xcf"
	"\x13\xfc\x79\xc4\x72\x51\xc0\x19\x29\x74\x30\x29\xe8\x96\x81\x10\xf1\xf5\x19\x30\xbd\x08\x3c\x08\xa0\xb5\xda\xbd\x35\x89\x45\xfc"
	"\xab\x8a\x4f\x3c\x6e\xc2\x84\x7c\xe2\x8b\x1d\xe8\xf9\xa6\x98\x80\x82\xf8\x23\x89\xba\xa3\x9b\x6a\x63\x86\x16\xb0\x8c\xb9\x0b\x7a"
	"\x36\xbe\xc7\xf0\x42\x92\xcc\x0d\xec\x28\x43\x9a\x74\x1b\xae\xeb\xbc\x20\xde\xb4\x2d\x9c\x00\xd8\x7a\xe4\x85\x7e\xce\x36\x11\x14"
	"\xe5\x8b\x07\xa4\x05\xe5\xd6\x96\xdf\xcd\xd9\x30\x4e\xff\x51\x22\x37\xb7\x65\xd6\xca\xfc\x9c\x65\xe6\x99\x77\x6c\xca\xd9\xc7\x8f"
	"\x5e\x1d\x0b\xd5\x47\xba\xed\x9d\xdb\xc2\xd4\x21\xbe\x27\xc0\x17\x5c\xc5\x8c\x9a\xc9\x17\xe1\x4e\xe5\xa1\xd0\xa2\xe5\x2b\x8c\xe3"
	"\x75\x25\xdf\x1f\x2c\x0a\x4e\x0d\x81\x4c\x2a\xe5\x0c\xfd\x14\x38\xcd\x0f"
;