#include "GameShaders_hlsl.h"
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
	{ { 1 }, {  }, { 1 }, 0, 1325 },
	{ { 2 }, { 1 }, {  }, 1325, 1991 },
	{ { 3 }, {  }, { 1,2,3,4 }, 3316, 1030 },
	{ { 4 }, { 2,3,1,4,5,6 }, {  }, 4346, 8992 },
	{ { 4 }, { 2,3,1,4,5,6 }, {  }, 13338, 8992 },
	{ { 4 }, { 7,3,1,4,5,6 }, {  }, 22330, 9227 },
	{ { 4 }, { 7,3,1,4,5,6 }, {  }, 31557, 9227 },
	{ { 5 }, {  }, { 5 }, 40784, 477 },
	{ { 6 }, { 2,1,8,9 }, {  }, 41261, 6982 },
	{ { 6 }, { 2,1,8,9 }, {  }, 48243, 6765 },
	{ { 6 }, { 7,1,8,9 }, {  }, 55008, 7217 },
	{ { 6 }, { 7,1,8,9 }, {  }, 62225, 7000 },
	{ { 7 }, {  }, { 1,2,3,4,6 }, 69225, 1628 },
	{ { 4 }, { 2,3,1,10,11,12 }, {  }, 70853, 9039 },
	{ { 4 }, { 2,3,1,10,11,12 }, {  }, 79892, 9039 },
	{ { 4 }, { 7,3,1,10,11,12 }, {  }, 88931, 9274 },
	{ { 4 }, { 7,3,1,10,11,12 }, {  }, 98205, 9274 },
	{ { 8,9 }, {  }, { 1,7,8,9,10,11 }, 107479, 1825 },
	{ { 4 }, { 2,3,1,4,5,6 }, {  }, 109304, 8992 },
	{ { 4 }, { 2,3,1,4,5,6 }, {  }, 118296, 8992 },
	{ { 4 }, { 7,3,1,4,5,6 }, {  }, 127288, 9227 },
	{ { 4 }, { 7,3,1,4,5,6 }, {  }, 136515, 9227 },
};

const SpUniformBlockInfo spUniformBlocks[] = {
	{ }, // Null uniform block
	{ "DebugEnvSphereVertex", 120 },
	{ "DebugEnvSpherePixel", 16 },
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
	{ "diffuseEnvmapAtlas", (uint32_t)SG_IMAGETYPE_3D },
	{ "shadowGrid3D", (uint32_t)SG_IMAGETYPE_3D },
	{ "envmap", (uint32_t)SG_IMAGETYPE_CUBE },
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
	"\x28\xb5\x2f\xfd\xa0\x4e\x39\x02\x00\x8c\x87\x00\x8a\x77\x74\x16\x2c\xb0\x6e\xca\x0c\x70\xbb\x6d\x4a\xb0\x24\x25\x50\xd7\xa0\xdd"
	"\x23\x76\x78\xdd\xc9\x49\x16\x08\xbb\x5c\x75\x52\x86\x1b\x28\xa2\x58\x20\x68\x09\xef\x9f\xd2\x9f\x22\x3d\xaa\xb9\x02\x5a\x01\x5a"
	"\x01\x5e\x01\x5e\x8c\x58\x38\xbc\xd8\x75\x79\x01\x94\x51\xfa\xc6\xcb\x38\x9d\x7f\xfe\xc3\xfe\xf9\x8c\x17\x25\x6f\x7d\xc7\x65\x68"
	"\xe0\x15\x11\x49\x9c\x08\x44\x82\x06\xb6\x57\x0e\xd7\xce\x62\xeb\x5d\x09\xdf\x78\xc5\xf6\xda\x27\xdb\xe2\x4d\xab\x93\xa6\x82\xb2"
	"\xf3\x8a\x3b\x82\xad\x6a\x5d\x5e\x60\x39\x77\xb9\x5c\xec\x0d\x14\x88\x48\x76\x73\xdd\xb3\x13\x72\x6b\xac\xaf\x8a\xfe\xa9\xdb\x6c"
	"\xd7\x09\x00\x4d\x45\xde\x5c\xb7\x7b\xd1\x54\x30\x82\x32\xce\x8f\x9e\x87\x1c\x07\x73\xda\x5b\x1f\x2d\x2b\x7e\xd1\xc1\x41\xca\x37"
	"\xbb\x5b\x67\xa0\xff\xbc\xf8\x3c\x74\xac\x77\x80\x8d\xc8\x8b\x02\x38\x87\xf6\x06\xb6\xb5\xbe\x30\x19\xfa\x4d\xcb\xb1\xf5\xa2\xf5"
	"\x50\xbe\xe5\xfc\xe5\x52\x54\x50\x80\xf5\x10\x46\xc7\x7a\xcf\x62\xf9\x9b\xa6\xea\x9c\xae\x6c\xdc\xc9\xb7\x6a\x9c\xbb\x58\x95\xe0"
	"\x10\x1a\x97\x9f\x77\x8a\xec\xda\x87\x6d\xbf\x8a\x25\x56\xb7\x8a\xcd\xdc\x09\xed\xf9\x2e\x9a\x0a\xab\x0e\xb0\xbb\x19\x6c\x17\x2b"
	"\xd0\x13\x18\xde\x1a\xf3\x3f\x79\xbf\xb5\x85\xaa\x55\x30\x7a\x43\x69\x71\xb6\xed\xfc\x66\x76\x3f\x81\xa7\xd8\x6d\x29\xb6\x49\x97"
	"\x17\x64\x9c\xc5\x46\x7e\xdb\xcb\x85\xb3\x5c\x15\xca\x44\xe2\xd9\xbe\x6d\x37\x4d\x5c\xb5\xad\x73\x97\xe5\xfb\xed\x4a\x69\x9a\xaa"
	"\xa2\xa9\x1a\x54\x28\x93\x27\xf0\x91\x06\xf6\xca\xb5\xf3\x7c\x2b\x1b\xff\x9e\x2f\xb7\x68\x2a\x19\x91\x78\xf0\x40\x81\x84\x05\x13"
	"\x4f\xe0\xb3\xbe\xfd\x72\xbd\x55\xea\x4b\xbd\x6a\x7c\x59\x34\x19\x9e\x5b\xee\xee\x1d\x41\x45\x32\xf9\x7e\x71\xbb\xf3\xff\xbc\xd3"
	"\x37\x06\xeb\x29\x5e\x96\xd1\x4a\xb1\xe1\xfa\x40\xf9\x08\xa2\x6b\x2c\x42\x02\x03\x06\x95\x08\xc5\x7b\x26\xcf\x91\x97\x67\xbc\x43"
	"\x27\xef\xad\xa7\x8c\x87\xae\x31\xca\xcb\xcb\x85\xeb\x62\x8a\x6b\x52\x71\xa8\x30\x39\x28\xd4\x5e\x23\x3d\x5f\xbd\x55\x52\xdd\x56"
	"\xab\xd7\x74\xa0\xa8\x0b\xbc\x6a\xd3\x24\x49\x1a\xb3\x16\x5c\x29\xa5\x73\x5a\x6b\xe1\x14\x44\x24\x8c\xc6\x2a\x05\xef\xb8\x8e\xfb"
	"\xc8\xe1\x5c\xef\xeb\x9f\xc7\xbc\xd5\xf3\xb6\x2d\x9a\x9e\xb7\x2a\x82\x9e\xc0\x16\xd3\x9a\xb6\xf5\x85\xe7\xba\x27\xae\xab\xd5\xfc"
	"\x4d\xdd\x53\xe9\x5a\xa4\x8e\xab\xd4\xa7\xb4\x67\x9b\xe5\xda\x2b\xd5\x8a\x24\xd0\x82\x8e\xcd\xdc\xb7\xbe\x93\x48\x07\x73\xdc\x6e"
	"\x38\xcd\xd9\xf0\xa0\x20\x10\x1a\xa5\xe7\x74\x8d\x4d\x15\xf4\x2a\xd8\x3d\xd4\xf8\xce\x7a\xbe\x5b\x34\x95\xf5\x76\x73\x5c\x6f\x9e"
	"\xdd\xaa\x66\xb3\xee\x3f\x45\x8f\x49\xa7\x29\x1b\xa8\x34\x29\xf4\x68\xa0\x1e\x10\xda\xe0\x78\x34\x4c\x90\x28\x9f\x87\x7c\x48\x10"
	"\x88\xd4\xd0\x20\xa3\xa4\x26\x39\x41\xe8\x09\x4b\xb6\x39\x84\x99\xef\x23\xe8\xc5\xae\x7b\x49\xa2\x28\xc6\xc3\x87\x53\xa4\x91\xe7"
	"\x60\xf4\xf6\x50\x94\x8c\x8b\xa6\xf2\xfe\x79\x0a\x89\x74\x2e\x1a\xf8\x90\xd2\x94\xa4\xb4\x0a\xbc\xe7\xfc\xc5\x73\x36\xad\xe2\x6b"
	"\xe1\x58\xb2\x3d\xd1\x13\xbb\xda\x7c\xf1\x57\x3c\x4f\x4d\x99\x27\x23\xd1\xef\xf9\xa0\xcc\xe7\x45\x18\x41\xec\xa3\x2f\x82\xf3\x09"
	"\xbc\x8c\x8e\x86\x8c\xf2\x0f\xbc\x8c\xd2\x80\x17\x23\x9c\x87\xde\x73\x1c\x6c\x3b\x0b\x11\x9f\x11\xbd\x6a\x4f\x27\x27\xdb\x2a\x92"
	"\x0f\xf4\xcf\x47\x12\x1f\x59\x8f\x2d\x57\x46\x0a\xd3\x10\x13\x8d\xeb\x3c\xeb\xeb\xc9\x44\x89\x27\xf3\x45\x89\xe7\x83\x24\x24\x1e"
	"\x2f\x4a\x2c\x5e\xa2\xa2\x64\x38\xfd\xb2\xf8\x62\xf7\x5b\x2f\xf1\xd2\xc3\xe0\xdc\x66\x8a\x8c\xd3\x1b\x3a\x0f\x3d\x5f\x94\x97\x7f"
	"\xa8\x87\x61\x71\xbd\xb8\x03\xc3\x26\x09\xc3\xb0\xe9\x70\x44\x0a\x91\x20\x93\xc2\x26\xe6\x88\x12\x03\xa3\x94\xa2\x18\x04\xe2\x98"
	"\x18\x35\x49\x87\xc4\xfe\xa1\x83\xfa\xfe\x81\x22\xef\x1f\x6a\xf9\xc7\xc9\x57\x7b\x88\x43\x46\xca\x31\xc9\x29\x3e\xde\x7a\x8a\x7a"
	"\xc4\x69\x92\x24\x04\x12\x25\x05\x79\xc4\x69\x79\x3c\x1c\x33\x4a\x86\xc5\xfa\x8f\xd2\x75\x24\x73\xae\x71\x86\xeb\xd5\x74\xfe\xb0"
	"\x96\x30\x70\x88\x74\x79\xc1\x4b\x30\x92\x0f\x0f\x14\x0a\x7c\xf8\x88\x42\x79\xe8\x21\x1e\x3e\xa2\x54\x2c\x9e\xc5\x02\xe2\xa3\x08"
	"\xc7\xf6\xba\xb9\x5e\xf9\x5b\x15\xe3\x21\xf4\x33\x4a\xa7\x72\xd9\xf6\x5e\xdf\xc0\x37\x15\x32\x3e\x1a\x54\x94\x19\x2f\x42\xe7\xa3"
	"\x05\xf4\x28\xf0\xec\x44\x80\xf7\xa2\x54\x6f\x02\x8a\x39\x07\x0a\x0a\x3c\xd6\x63\x98\x6c\xd8\x78\x29\x72\xba\xc6\x30\xe9\xfc\x4a"
	"\x9e\x61\x7d\xed\xd8\x62\x98\x07\xb8\xa9\x2a\x48\x12\xa5\x73\x56\x15\xe4\x43\x6f\xbd\xac\xbd\x62\x3f\x59\x5e\x2d\x1f\x49\x1c\x1c"
	"\x28\x94\x48\xbe\xf4\xd1\x47\xbb\x62\xee\xb2\x1b\xaf\x38\x44\x24\x12\x10\x68\xfc\x81\x37\x00\xa1\x24\xca\x41\x21\xd4\x9c\x66\x94"
	"\x1e\x19\xa9\xa6\xb3\xc5\x20\xeb\xa1\x67\xfc\x43\x04\x02\xc2\x7a\x08\x2f\xcb\xa0\x0f\x34\xcb\xbd\x67\xb9\x53\x6d\xe7\x2c\xb5\x9d"
	"\x21\x9d\x11\x8c\x16\x8b\x66\xc3\x60\x30\x99\x8c\xa4\x74\xce\xef\x83\x28\x9d\xd3\x8b\xcf\x57\x90\xef\x5d\x8c\xde\x92\x77\x17\x15"
	"\x28\x64\x52\xb0\xfc\x34\xb9\xcd\x3a\x41\xbe\x17\x26\x98\x4a\xe5\xf6\xba\x58\xbe\x12\x9d\x98\x0f\xf3\x65\x9c\x7b\x21\xa6\xe2\xab"
	"\x49\x51\xd9\x65\x78\xe9\xb2\x94\x89\x9e\x90\x13\xc2\x85\xab\x6e\x99\xa3\xba\x9e\x6f\x05\x25\xcf\x68\xaa\x86\xf5\x40\x03\x14\x3d"
	"\xcb\x16\x5b\xfc\x3c\x94\x97\xa7\xa9\x68\x88\x52\xc6\x49\xcd\x28\x65\x9c\x0c\xeb\x23\x2a\xd8\x78\xf8\xc5\x86\x28\x39\x92\xff\xe2"
	"\x80\x01\x8c\xa6\xca\x58\x98\x46\x14\x75\x5c\xc7\x49\x9e\xb7\xde\x41\x51\x4c\x9c\x98\x28\xb3\xac\xce\x39\xa3\xec\x27\x4c\xa6\xe3"
	"\x38\x08\xfc\xf3\x7f\x5f\x67\x58\x06\xbc\x98\x33\x31\x33\x3a\x7c\x6e\xb9\x5c\xfe\x29\xae\x1b\xb3\x14\xb5\x5f\x0f\x2c\xa6\x2f\x3a"
	"\x29\xba\x6f\xe4\xab\xfb\xa5\x8e\xfb\x69\xde\x88\x9e\xa0\xe8\x9b\xc5\xb6\x8b\xaf\x29\xb3\x39\x21\x86\x17\xb9\xde\xf4\xc6\xf5\xf6"
	"\x54\xa2\xb3\x82\xd5\x6d\xc2\x76\xf1\x45\x52\xcc\xd7\x3f\x9a\xfc\x23\xc9\xe7\xc4\x1f\x65\xbc\xb8\x59\x2f\xf1\x33\xfe\x93\x18\x37"
	"\x7c\xd8\xd9\x5a\xe3\x8b\xc4\x8f\x0d\x6f\x93\xaf\xee\x8d\x20\x42\x31\x5f\x15\xd1\xe9\x2d\xa8\x40\xe5\xfc\xd6\x97\x0f\x7e\x44\xc9"
	"\x3c\x0f\x3d\xd6\xd3\x07\x63\x7d\x14\x41\xc9\xd9\xb6\xc5\x17\x2f\x32\x9e\x32\xe1\x5a\xbe\x08\x59\x7f\x81\x57\x07\x93\xf5\x08\x7a"
	"\xd5\x60\x64\xfd\x04\xe7\x19\xff\x7c\xc7\x45\x8e\x13\x84\xb5\xa8\xf3\xa7\xab\x1c\xa4\x0c\xc9\x0c\x04\x08\x00\x0d\x53\x12\x60\x50"
	"\x60\x6c\x30\x94\x0b\xa5\xc2\x2e\x37\x7b\x13\x00\xc9\xc4\x72\x81\x3c\x1e\x0d\xe6\x41\x8e\xc3\x20\x65\x90\x41\xc6\x00\x62\x00\x01"
	"\x00\x01\x00\xc0\x0c\xcc\x00\x23\xc2\x02\x24\x56\x80\xe6\x11\xb6\xb0\x35\x06\x4c\xe1\x24\xf0\xdf\xc9\x99\x55\xeb\x79\x3c\x2e\xd4"
	"\x46\x35\x95\xd3\xde\x06\x35\xe5\x43\x5b\xd1\xdf\xbe\x06\x6e\x52\xb3\xc8\xd6\x0f\x3e\x2f\xec\x8b\xfd\x23\x49\xf0\x0a\xec\xa9\xd8"
	"\xc6\x84\xb6\xf3\x03\x7a\x21\xd0\xb8\x11\x3f\x58\x00\x52\xdb\x58\x90\x91\x76\xd2\x2b\x37\x9e\xac\x18\x40\xab\x71\x79\x06\x0e\x00"
	"\x23\x02\xcb\x64\x80\x00\x21\xe9\x11\xc0\x4c\x92\x06\x8b\x7e\x15\xe1\x99\xc0\x17\x2d\x65\xc7\x50\x35\x4e\x37\x76\xe2\x21\xaa\xc3"
	"\x44\x24\xc4\xe7\xf5\x0f\xac\x91\x7b\xba\xc6\xbe\x42\xc3\x24\xd9\xa4\x22\xe3\x1e\xc3\xef\x10\xd2\x13\x0b\x86\xaf\x85\x8a\x46\x42"
	"\x21\x53\xc1\xd6\xab\xb8\xb9\xd5\x91\xcb\x43\xe7\x21\xdb\x65\x51\x68\xeb\x11\x39\x24\x36\x14\x21\x86\xf4\x49\x92\xe2\x62\x45\x2a"
	"\x30\xd3\x54\x7c\xa2\xb8\x0a\xac\x29\x5e\xe8\xda\xbe\x87\x54\xba\x2a\xc4\x78\xcc\xf4\xf9\x24\x12\xbf\x31\xe0\xd8\xd5\xbc\x6b\xb0"
	"\x4b\xa2\x1e\x30\x5c\xb6\x98\x32\x77\xed\x52\xa3\xda\xf5\x1f\xfe\x4a\xf3\x16\x71\xc1\xad\x4d\x81\x95\x44\x3c\x65\xaf\x18\xa3\x4f"
	"\xa0\x76\xff\xf2\x47\x8c\xdd\x4e\xfd\x3b\xfc\x7f\x12\x13\xd7\x75\x54\x03\x82\x70\xd7\x5f\x38\x78\x00\xb6\xef\xb4\xbc\x78\xe1\x91"
	"\x67\x34\xa5\x7c\x1a\x19\x0b\x42\xc4\xb8\xa0\xf5\x2f\x7a\xbf\xbe\xe0\x76\x81\x82\xa0\x98\xd9\x1b\x31\x85\x30\x66\xf2\x77\xcf\x1c"
	"\x33\x66\x45\x16\x64\x26\x89\x48\x23\x48\x66\xfe\x69\x7d\x42\xf4\x94\xc1\x76\x59\xd1\x36\x17\xbc\x0d\x85\x76\xe6\x13\xd2\xa2\xc6"
	"\xe5\xa2\x04\xf4\xc0\xa9\x4c\x91\xb4\x14\x0b\x1c\x78\x99\x13\x4e\xc2\x0b\x7e\x6e\x40\xdf\x29\x04\x14\x2b\x40\xf6\xa4\x06\x2e\xc6"
	"\x85\xe0\x57\x13\xd7\x16\x9b\x01\x7a\x2f\x84\xa0\x93\x53\x9a\x43\xc7\xc1\x18\xe8\xde\x38\x73\xf2\xe4\xf7\x63\xfa\xf9\x3d\x92\x24"
	"\x88\x57\x03\xa0\x28\x60\xb3\x6b\x7f\x2c\x2b\x44\xe6\xb7\x42\x80\xd0\x88\x61\xc0\x05\x57\x6a\xc0\xc5\xd2\x14\x78\xf6\xcf\x0d\x94"
	"\xf0\x44\x51\x06\x68\xb4\x85\x4c\x78\xf2\x74\xaa\xc7\x0b\x20\x3a\x06\x8c\xf7\x2f\xc0\x91\x86\xb3\x6b\x55\x42\x6d\x48\x9e\x04\x2c"
	"\x35\xd0\x0d\xb2\xa7\xda\x8b\x0b\x64\x1b\x7a\xe4\x02\xa7\x7c\x85\xfd\x2e\x91\x98\x80\x98\xb4\xac\x2a\x71\xec\xe6\x22\x8d\x9c\x82"
	"\xab\x6d\xc9\x43\x30\x0b\x07\xdd\xb9\x28\xe1\x80\x05\xb4\x9a\x66\x8d\x31\x8c\x70\x4a\x57\x8d\x94\xcc\x8d\x99\xcc\x4a\x35\x2d\xd6"
	"\x2f\x0b\xa8\xdc\xc6\xfd\x52\x32\xdb\x94\xe0\x0d\xa4\x73\x99\xea\xc9\x0f\x8a\xa1\x22\xf2\x16\x46\xa3\x4c\x1e\xbb\x16\x48\xde\x0a"
	"\x24\x7f\xb1\x1d\x41\x9f\x4a\x47\xee\x1d\xf2\x22\x00\x13\x0d\xff\x9c\x15\xe8\x70\x48\x7d\x66\x6c\x5b\xae\xc5\xab\x24\xb6\xd1\x82"
	"\xcd\xfc\xb5\x08\x5e\x6a\xae\xd4\x28\x5b\x9a\xae\xbd\xb1\x00\xce\x18\x14\x0c\xbd\xab\x4f\x29\x5c\xca\xcd\xeb\xed\x04\x47\x72\xeb"
	"\xee\x30\x8a\x83\x65\x59\x2d\x33\x1b\x31\x0c\xe5\x54\x53\x1b\x8a\xf3\xbc\x55\x62\x6d\x62\xbc\x8f\x46\xd9\xc2\xd1\xea\x2a\xc0\x24"
	"\x64\x17\x68\xc9\x74\x94\xdb\xa8\x4e\x24\xa8\x2f\x6a\x31\xb9\x3d\x0d\x54\xa4\x97\x6c\x7c\x2d\x19\xf9\x72\x26\x5f\xff\x4f\xc5\x29"
	"\x61\xbc\x66\x4b\x32\xf4\x22\xad\xa4\x18\xc1\x82\xaa\x81\xa0\x26\x40\x6e\x04\xd4\x5f\xb2\x2e\x84\xc2\x08\x21\x6f\xad\x7b\x9d\x34"
	"\x89\x7f\x86\x81\xf0\xb2\x72\x33\xeb\x49\x5c\xae\xea\x5a\xcf\x24\xe1\x25\xb8\x8e\xb8\x23\xcb\xb3\x8a\x0a\x63\x40\x0d\x50\x3e\xfc"
	"\xff\x94\xfa\xeb\xc0\x4a\x92\x46\x5d\x2e\x3b\xce\xcb\x5c\xb2\xca\x75\xe1\xf7\xd9\xc2\xcf\x76\x4b\x39\x43\x43\xa2\x52\xd7\x16\x13"
	"\x4d\xc5\x05\x76\xee\x95\x8d\x0f\x76\xd8\x35\x22\x1c\xb5\x13\x9a\xdf\x2d\x89\x6a\x6c\xe1\x8e\x55\x4b\x12\xc5\x84\xf8\xda\x94\x14"
	"\x0c\xcc\x46\xb0\x4f\xb0\x2a\xea\x6a\x45\x63\x02\x9a\xc6\x98\xb2\x02\xae\x3a\xfd\xeb\x69\x89\xa3\x44\xe7\x96\x5d\x34\xb4\x36\x07"
	"\x0c\x1d\xd8\x6c\x9f\x4e\xa0\xee\xc0\x42\x79\x71\x4d\x68\xaf\xe6\x29\xac\x48\x82\xfa\xc5\x9e\x0c\xda\x84\xea\x16\xa2\x1e\x61\x13"
	"\x40\x40\xe8\x87\x97\x54\x45\x47\x85\xfc\xa5\x70\x86\x4e\x8a\x51\x6a\x92\xad\x7d\xd6\x1b\xad\xa2\x98\x40\xeb\x5f\x85\x0a\x93\x87"
	"\x64\x05\x5b\x32\x1c\xdd\x0a\xa9\x99\xa4\x3d\xd0\xed\xf6\x26\x94\xf3\x7d\x08\x93\x05\x34\x54\x89\x77\xbd\x44\x8b\xbc\x02\x85\x10"
	"\xfa\x1d\x38\x81\x57\xd8\xbc\x5c\x9b\xe3\xcc\x10\x96\x79\x44\x90\xfd\x66\xd0\x31\x84\xa7\xc9\x25\xae\xf6\x43\x30\xb5\x7e\xc3\xb5"
	"\x58\x5e\xc4\x4a\xea\x8f\x28\x5c\x3f\x77\x1e\x96\x05\x3e\xdd\x1c\x61\x05\x2a\x46\xb6\xa7\x39\x1b\xf2\x08\xcd\x11\x24\xec\xfb\xf8"
	"\x2f\x09\x02\x11\x5c\xa2\x3b\xdf\x81\x54\x74\x29\xc9\xe5\x98\xb2\x2e\x01\x06\x8f\x0e\xbf\x8a\xd0\x1d\x02\xf4\x7d\xb9\x23\x16\x91"
	"\x93\xe4\xdc\xbb\x4b\x87\xde\xe1\xc6\x5c\xf4\x41\x69\x83\x16\x3e\x4c\x1a\x18\x0b\xea\xbd\x6b\xd8\x94\xc6\x60\x68\xf1\xeb\x65\xe4"
	"\xae\xd8\x2c\x03\x44\xe3\x81\x98\x34\xe7\xb3\x65\x3e\x3e\x43\xc1\xc3\xea\xce\x45\xd2\x4a\x42\x03\xd3\x3e\x23\xd8\x0e\x02\x4e\x1e"
	"\x61\x66\xe1\xf0\x9e\xf8\xb2\x9d\xcb\x08\x81\x38\xe9\x95\xc1\x68\x8f\xcf\xa1\x8c\x9c\xc5\x8c\x34\x6f\xf9\x23\xa3\x75\x99\x72\x24"
	"\x15\x12\xb2\xa5\xed\xdf\xf1\x63\xff\x06\xe5\x18\xd9\xff\xc6\xee\x63\xef\xda\xe7\xee\xa2\xda\x64\xe9\x6c\x3d\xe7\x50\x1f\x3c\x8f"
	"\xc1\x9c\x66\x35\x82\x34\x36\x26\x37\xdd\x49\x4a\xf6\x67\x92\x24\x78\x1c\x65\x1c\x1d\xac\xf2\x72\xbf\x5e\xb1\x7a\x34\xb6\x81\x4f"
	"\x28\x02\x94\x4e\xc0\xd8\x50\xe9\x09\x4c\xea\x5c\x6a\x47\x22\xbe\x08\x75\xc8\x28\x7b\x75\xb2\xb9\x93\xe4\x5e\x74\x8b\x8a\x09\xf2"
	"\x2d\x5d\x1f\x22\x35\xec\x7b\x03\xec\x39\x3a\x5f\x80\xda\xf3\x5a\x2b\x25\x81\x86\xdc\x0a\x1a\x20\x1f\x10\x08\xce\x1a\x95\x14\x43"
	"\x52\x35\xae\x95\xf4\x6e\x20\xc6\x13\xbc\x8d\xc5\x0c\x73\xb9\x77\xa3\x70\x9f\x58\x3b\x02\x4b\xd9\x40\x9a\x45\x56\x42\xca\x13\x1f"
	"\x0c\xa6\xfb\x24\x65\xce\xa0\xbb\xa9\x15\x37\x0c\x48\xc8\x22\x97\xdb\xcc\x90\x54\xb8\x8b\xcb\xd6\x8f\x0e\xd0\xdf\x3a\xfe\xfc\x0d"
	"\x3c\x7c\x6f\xde\x27\xab\xb3\x73\xab\xb6\x8d\x2e\x83\x87\x40\x1a\x2d\xda\xe8\xcd\x01\x3b\x79\x60\x7f\x43\x04\x2e\xc5\xdd\x2b\x87"
	"\x3d\x78\xaa\x27\xc0\x49\x3b\xd0\x9d\x79\x7a\xcf\x1a\x2c\x85\x3a\x1b\x4d\x46\x69\xb0\xe9\xce\x38\xc3\x8c\x94\xd3\x3d\xca\x67\x5b"
	"\x07\xc0\x12\xeb\xf2\x6a\x6a\xa0\x08\x2e\xef\x9e\xb6\xdc\x52\xf9\x21\xc0\x18\x16\x4b\x65\x0b\xda\x85\xe5\xa7\x95\x93\xc4\xa1\xbf"
	"\xe5\x24\x0a\x16\xb3\x2e\x27\xab\x2a\xa8\xdc\x29\x1c\xf3\xa7\x63\x28\xfe\xe2\x19\xee\xf6\xc1\x4e\x6e\x94\xa1\xf7\xf5\x8a\x5e\x92"
	"\x48\x09\x7a\xa8\x0c\xae\x4d\x8d\x90\x5b\xb0\xe9\xa4\x9b\xbd\x6a\x57\xe0\xf1\xc9\xb9\xca\xf5\x77\xdf\xf1\x01\xd0\x0b\xdc\x33\xc9"
	"\x4d\x87\xba\xe2\x95\x23\xe3\x4b\xdc\xc1\xd5\x36\xb6\x5a\x33\x84\x05\x5f\x54\x21\x9b\x80\x07\x07\x07\x22\x26\x65\xc4\x2e\x8d\xb0"
	"\x3a\x1b\xfc\x20\x04\x73\x61\xbc\x65\x18\xc2\xd8\xc5\xad\xee\x6d\x3c\x0f\xff\xb1\x41\x98\xcd\xfa\x3d\x60\x81\x83\xf0\x28\x40\xe6"
	"\x90\xde\xb7\x0e\x66\x72\xc5\xc3\x60\x81\xc3\x8d\xbb\xb9\x13\xb9\x29\x17\xbd\xa1\xa8\xe8\x80\x15\x4e\x25\x85\x73\x01\x62\xb5\x4e"
	"\x3d\x4e\xed\x22\xfd\x82\xda\x95\xdd\x10\x24\x4d\xde\xc4\x35\xbe\xb0\x38\x95\x47\x63\x7d\x58\x86\x4c\x9e\xcf\x2e\x24\xd5\xa9\xe9"
	"\x06\x7d\x56\x88\x3a\x3e\x5b\x32\xda\xbe\x10\x45\xdb\x16\x0c\x06\x3c\xa4\xee\xc3\xb8\x22\xd8\xea\xc7\x60\xda\x78\xc2\xf2\xa4\xed"
	"\xd6\x6f\x74\x99\xe2\xc4\x30\xe5\xb7\x21\x54\xbc\x28\x40\x2c\x41\x80\x89\x17\x9b\x7e\xfd\x22\xa3\x1b\x5b\x8b\xc7\xc3\xcf\x0d\x91"
	"\x3a\x3d\xc9\xe6\x98\x24\x5b\x0d\xf1\xb5\x41\xc1\x06\x19\x57\x60\x21\x78\x60\xf7\x0c\x1a\x09\x34\xfd\x6e\xd2\xce\x9c\x56\x96\x62"
	"\xcf\x34\x4a\x6e\xae\x9a\x05\xaa\x80\x4d\x58\x48\x46\xc9\x80\x5f\x9c\x85\xe6\xc0\x2a\xab\x9f\x95\x5a\x31\xc7\x0f\x81\xde\xe2\x21"
	"\x49\xab\x14\xc0\xc8\xe8\xfc\x49\x99\xbd\xa2\x6b\x12\x5a\x57\x97\xb6\x0e\x1e\x0e\x44\x08\x8a\xb6\xed\xdd\x8a\x8b\xdb\x25\xf5\x6a"
	"\x82\xaf\xeb\x94\x45\xab\xf0\xd5\x71\x61\x52\x20\x15\x84\x1f\xd2\x48\xa8\x43\x1e\xa1\x91\xb0\x6b\x19\xdb\xb5\x1a\x0a\xa7\x22\xfc"
	"\x6b\xd7\x33\x02\xfd\xdf\xbc\x43\xf6\x71\x6b\xb1\xd0\xa9\x24\x1e\xb1\x6f\x5b\xab\x6f\x2c\xcf\x64\xf2\xc2\xcf\xe4\x5d\xf1\xeb\xdd"
	"\x4d\x43\x4d\x78\x9f\x66\xa6\x03\x2f\x7f\x0c\x4d\x2f\x91\x54\x72\x78\x1d\xf1\x5c\xc4\x9a\x6e\x98\x99\x4a\x0c\x10\xff\x31\x39\xdd"
	"\x8a\x49\xf1\x43\x22\x16\x35\x91\x0a\x5f\x6d\x3e\x15\xcd\xb0\x11\xc5\xa5\xf0\x47\x60\xfb\xb6\x55\x6f\x15\x43\x46\x26\x32\x46\x11"
	"\x81\xe8\xb1\x58\x2e\xec\x44\xc1\x40\x4c\xdf\xff\x52\xb1\x46\x49\xe5\xdb\x60\x1c\x81\xf3\x34\x18\x70\x41\x5d\x13\x1e\x1b\x75\x3e"
	"\x8c\xb6\x95\x64\xa4\x2f\x87\x83\xb3\x20\x4c\x14\xd3\xf1\x97\xf7\x4b\x2d\xaf\xa4\xb4\xfb\x54\xbb\xb8\x66\x7b\xfa\xe4\x3a\x44\xf5"
	"\x50\x33\x71\x43\x19\xc3\x38\x65\x68\x93\xe3\x10\xec\xfa\x25\x07\x56\xe1\x72\xd1\x54\xec\x33\x8f\xec\x11\xab\xda\xd0\x45\x41\x7a"
	"\x9e\xfb\xcc\xc3\xf4\x2a\xd8\xb9\xd7\x4a\xaa\xfb\xa2\x27\x38\x7f\xbf\x45\xa4\xb9\x9e\x49\x8e\x70\x38\x84\x33\xfb\x93\x92\x89\xfb"
	"\x07\x38\x51\x73\x4d\x39\xe8\xe0\x46\x1a\x97\x2e\x82\xf6\x10\x93\xef\x93\x9a\x41\x42\x1b\xc6\xfa\x7f\x35\xb1\x6f\x34\x06\x77\xa4"
	"\xae\x93\x09\x62\xc4\xa6\x7f\x40\xfb\x85\x95\x06\x9f\xec\x68\x51\x33\xd6\xe8\xb3\xcd\x84\x37\xf4\x43\xf1\xdd\x72\xd3\xf8\xd0\x52"
	"\xc1\xc9\xec\x10\x0c\xc1\x65\xc2\x5d\x2c\x6f\x04\x7c\x51\xa1\xd2\x01\x9b\x27\xc2\x62\x93\x7c\x58\xb8\xec\xa9\x66\x71\xba\x31\x73"
	"\xf8\xcc\x03\x42\x13\x76\x1b\x08\x5a\xee\xf4\x2d\xd7\xa2\x86\xcd\xe1\x92\x2c\x35\xcc\xf6\x9a\xd3\x3b\x35\xb9\xa6\x23\xaa\xbd\xa8"
	"\x4a\x6d\xfe\x13\x4d\x67\x16\x84\x07\x25\x6c\xee\x01\x12\xb6\x0e\xec\xe2\x2b\x55\x48\x45\x4e\xaf\xd4\x4b\x81\x9e\x68\x20\x9e\x17"
	"\x88\x29\x1d\xde\x35\x30\x2f\x37\x32\x34\x3d\x65\x51\x2b\xa6\x58\xb2\xe6\x7d\x55\xac\xdb\xcf\x1e\x0b\x4b\x8b\xc6\xad\xcb\x2d\x14"
	"\x16\x49\x79\x63\x0b\x04\x30\xd4\x18\xbe\x28\x6f\xd9\x3b\xc8\x44\x1a\x5c\x79\x33\x2a\x39\xa3\x0c\x84\xc4\x98\x25\x85\xb7\x64\xa4"
	"\xc1\xb3\x72\x25\x5a\xa9\x4d\x33\xd6\xbb\xe8\xaf\xc2\xaf\x83\x35\x7b\x68\x08\x2b\x24\x51\x08\x85\x31\x41\x86\xb7\x83\x53\x0a\xb5"
	"\x6b\xcf\x36\x8d\x40\xe4\x4c\xfe\x1c\x28\xca\x3d\xd3\x42\xf9\xe0\x88\x37\xa3\x7e\x52\x2e\x6f\xc0\xc2\x63\x3a\x88\x61\x85\x0b\x1e"
	"\xf1\x78\x67\x9b\x79\x73\x12\x43\x81\xb1\x69\xe7\x15\x0c\x2e\x56\xe7\xe5\xb9\x9b\x0f\x18\xfb\x9c\x05\x2f\x7a\x3d\x39\x76\xdc\x87"
	"\x6b\xba\xc6\x69\xef\x3e\x9a\x86\xc7\xdf\x3e\xab\x9f\x5c\x8e\xd8\x29\x20\x75\x00\xcd\xce\xbe\x79\x6b\x03\x4a\x28\x6d\x51\x15\xa3"
	"\x26\xa7\x07\x71\xcf\x66\x06\x33\xc7\x19\x11\x39\x0c\x36\x22\xe1\xd5\x16\xc1\xbe\x34\xcd\xdc\x72\xa1\x70\xcc\x4f\x28\x25\x96\x33"
	"\x1c\x71\x5d\x02\x7b\x06\x5c\x0e\x48\x10\xd0\x02\x2f\x99\x66\x1b\x64\x55\xa0\x09\xfd\x64\x83\x58\x6e\x1c\x09\x51\xdc\x6c\x7a\x53"
	"\x83\x0b\x19\xcc\xd0\x61\x2c\x5a\x59\xf3\xa6\x01\x48\x72\xd2\xb5\x1f\x8d\x40\x98\x9f\x1a\x91\xfb\xb6\x9e\x11\x30\x0d\xa1\x9c\xf9"
	"\x47\x6b\xaa\x99\xec\x4e\x5d\xd6\x2e\xcf\x05\xef\xa3\x38\x05\x1a\xf5\xdf\x1e\x69\x8b\xd2\xc6\x92\x7c\xda\x3a\x29\x01\x5a\x78\x82"
	"\xc4\xff\xb7\x2b\xc5\x5f\x0a\x80\x18\xec\x6c\xee\x69\x2e\xa5\x73\x30\x12\x78\x2f\x6a\x42\x57\x08\xc4\x42\x43\xb9\x18\xcc\x30\xd4"
	"\x8d\x9a\x88\x0c\x19\xec\xe6\x68\xb1\x02\x05\xae\xc1\xff\x8c\xff\xb0\xc9\xa2\xc0\x99\x06\xed\xa1\xb6\x2c\x2a\x60\x3e\x18\x30\x01"
	"\x9a\xac\x85\xb9\xc3\x53\xb9\x82\x6a\xf9\xbf\x27\xb8\x52\x67\xe2\x2c\xe2\x87\x0e\x5a\xef\xf8\x64\xb1\x88\xbf\x84\x43\x8d\xb2\xc0"
	"\x55\x5a\xda\x83\x69\xbe\xde\x20\x56\xe8\xf7\xb8\xa8\x72\x14\x93\x40\xa3\xc2\x77\xe6\x48\x1e\x60\xb1\xb6\x65\xe4\xc1\x28\xe3\xfe"
	"\xb0\x1a\x56\x48\xf5\xfd\xf9\x6d\xe9\xd0\x42\xfd\xe6\xeb\x07\x6d\xd2\x81\x27\x68\x72\x4a\xe3\x01\xfe\x8a\x18\xdf\x66\x4b\x6f\x23"
	"\x0d\x2f\xac\x02\xd7\x1e\x62\x36\xd9\xe9\xb6\x19\xa3\x49\x9c\x66\xa6\x0a\x68\x0d\x4d\x36\x3b\x03\x80\x39\x94\x89\xd3\x5e\x8f\x65"
	"\x33\x60\x29\x8f\x15\xbc\xb2\x0a\x18\x78\x17\x0a\x70\xfa\x24\xd3\xe8\x57\x4c\xd4\x84\x94\x2f\xd4\x68\x8c\xd7\xff\x5f\x88\xf0\xce"
	"\x7b\x9c\x05\x07\x4f\xe8\x47\xcc\xf3\x87\x03\x59\xc7\xe3\xd2\x93\x2c\xcf\x7b\xc2\x0c\xa0\x87\x4c\xba\xf0\x0a\x0b\xae\x6c\x42\xe0"
	"\x90\xb2\x63\xba\x93\xd9\xca\x2b\xba\xc3\xc1\x47\xab\x27\xc8\x64\xe9\x32\x5c\x00\x45\x4b\x4b\xb3\x00\x40\xdc\x45\xe7\x17\x9c\x77"
	"\xd7\xad\x6a\x80\x1e\x04\x53\x40\x6a\x08\x20\x98\xea\x50\x32\x04\xe8\x65\x13\xca\x8b\xe0\x0d\x97\x33\x51\xbe\xf7\x01\x45\x00\x00"
	"\x08\x68\x01\x00\x4a\x99\x07\x42"
;