#include "GameShaders_macos.h"
#include "game/ShadersDesc.h"

const SpShaderInfo spShaders[] = {
	{
		"DynamicMesh",
		{ {
			{  },
			0, 1,
		}, {
			{ {0,2}, {1,2} },
			1, 4,
		} }
	},
	{
		"EnvmapLighting",
		{ {
			{  },
			5, 1,
		}, {
			{ {0,2}, {2,2} },
			6, 4,
		} }
	},
	{
		"TestMesh",
		{ {
			{  },
			10, 1,
		}, {
			{ {0,2}, {1,2} },
			11, 4,
		} }
	},
	{
		"TestSkin",
		{ {
			{  },
			15, 1,
		}, {
			{ {0,2}, {1,2} },
			16, 4,
		} }
	},
};

const SpPermutationInfo spPermutations[] = {
	{ { 1 }, {  }, { 1,2,3,4 }, 0, 1167 },
	{ { 2 }, { 1,2,3,4,5,6 }, {  }, 1167, 10881 },
	{ { 2 }, { 1,2,3,4,5,6 }, {  }, 12048, 10881 },
	{ { 2 }, { 7,2,3,4,5,6 }, {  }, 22929, 11558 },
	{ { 2 }, { 7,2,3,4,5,6 }, {  }, 34487, 11558 },
	{ { 3 }, {  }, { 5 }, 46045, 558 },
	{ { 4 }, { 1,3,8,9,10 }, {  }, 46603, 8557 },
	{ { 4 }, { 1,3,8,9,10 }, {  }, 55160, 8188 },
	{ { 4 }, { 7,3,8,9,10 }, {  }, 63348, 9231 },
	{ { 4 }, { 7,3,8,9,10 }, {  }, 72579, 8862 },
	{ { 5 }, {  }, { 1,2,3,4,6 }, 81441, 1910 },
	{ { 2 }, { 1,2,3,11,12,13 }, {  }, 83351, 10933 },
	{ { 2 }, { 1,2,3,11,12,13 }, {  }, 94284, 10933 },
	{ { 2 }, { 7,2,3,11,12,13 }, {  }, 105217, 11610 },
	{ { 2 }, { 7,2,3,11,12,13 }, {  }, 116827, 11610 },
	{ { 6,7 }, {  }, { 1,7,8,9,10,11 }, 128437, 2089 },
	{ { 2 }, { 1,2,3,4,5,6 }, {  }, 130526, 10881 },
	{ { 2 }, { 1,2,3,4,5,6 }, {  }, 141407, 10881 },
	{ { 2 }, { 7,2,3,4,5,6 }, {  }, 152288, 11558 },
	{ { 2 }, { 7,2,3,4,5,6 }, {  }, 163846, 11558 },
};

const SpUniformBlockInfo spUniformBlocks[] = {
	{ }, // Null uniform block
	{ "DynamicTransform", 128 },
	{ "Pixel", 1072 },
	{ "EnvmapVertex", 4 },
	{ "EnvmapPixel", 2176 },
	{ "Transform", 64 },
	{ "SkinTransform", 64 },
	{ "Bones", 3072 },
};
const SpSamplerInfo spSamplers[] = {
	{ }, // Null sampler
	{ "shadowGrid3D", (uint32_t)SG_IMAGETYPE_3D },
	{ "envmap", (uint32_t)SG_IMAGETYPE_CUBE },
	{ "diffuseEnvmapAtlas", (uint32_t)SG_IMAGETYPE_3D },
	{ "albedoTexture", (uint32_t)SG_IMAGETYPE_2D },
	{ "normalTexture", (uint32_t)SG_IMAGETYPE_2D },
	{ "maskTexture", (uint32_t)SG_IMAGETYPE_2D },
	{ "shadowGridArray", (uint32_t)SG_IMAGETYPE_ARRAY },
	{ "gbuffer0", (uint32_t)SG_IMAGETYPE_2D },
	{ "gbuffer1", (uint32_t)SG_IMAGETYPE_2D },
	{ "gbuffer2", (uint32_t)SG_IMAGETYPE_2D },
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
	"\x28\xb5\x2f\xfd\xa0\x2c\xad\x02\x00\xec\xa5\x00\xfa\x9d\x88\x1c\x30\xc0\x6c\x88\x6c\x03\x30\x86\xe5\x02\x6f\x1d\xcf\xc3\x9b\x4d"
	"\x0d\x2e\x31\x33\x2a\x88\x86\x98\xda\x01\x63\xe7\x08\x80\xef\xca\xee\xde\x29\x53\xbe\x2a\xf1\x13\xc9\xcc\x6e\x9b\xa2\x90\x29\x76"
	"\x22\xd6\x01\xb9\x01\xbb\x01\xf0\x65\x7d\x31\x76\xa5\x50\x42\x4a\x6d\xfd\xa6\xd6\xa1\xb5\x0f\xf2\xc3\x4f\x6b\xd3\xf9\x2d\xa7\x5b"
	"\x58\xa3\xad\xaf\xf9\x61\x97\x3e\xe5\xa4\x56\xd2\x87\x93\xfe\xb4\xd6\x5a\x5a\x67\x3f\xb8\xc0\x0f\xb7\xe3\xe1\xee\x61\x8e\x69\x68"
	"\x56\x43\x95\x65\x16\x15\xe1\x04\x76\x20\x84\x13\xa2\x2b\x61\x0f\xb4\x85\xba\x93\xc4\xaa\x3a\xca\x62\x38\x68\xd0\x30\xa9\x10\x58"
	"\x30\x0c\x94\x75\x97\x42\xdd\x65\x81\x20\x08\x62\xa9\x9a\xa8\xea\x32\x70\xa1\xa0\x60\xcc\xa2\xac\x07\x64\x71\xac\x88\x93\xe2\x6a"
	"\xb7\x41\xab\x26\xeb\x8a\xae\x02\xd7\x95\x4c\x2a\x04\xa4\x6c\x83\x40\xa6\xe8\x2f\x26\x49\xee\x03\x8a\x24\x08\x8f\x7b\x79\xc3\xe8"
	"\x5e\x10\x74\x65\xdc\x20\xd3\x1e\x44\xa6\x07\xc9\x56\x31\x25\x71\x13\x63\xb1\x10\x20\x93\xae\x4e\xb3\x34\x0b\xbb\x16\xb2\x49\xd2"
	"\xc0\x82\x00\x1c\x49\x98\x55\x45\x26\x7e\x4c\x5a\x66\x55\x91\x3b\x45\x1c\xa9\x2e\xf5\xe0\x2b\x89\x5c\xec\x1a\x00\x80\xc5\x22\xc2"
	"\x82\xa1\xc2\x36\x4d\xc2\x26\xdb\xc3\x93\x8d\xc2\x0d\x33\xce\x92\x65\x16\xf6\xe0\x47\xab\x16\xa9\x32\xf1\x73\xbd\xe7\xe2\xe7\x69"
	"\x9b\xa5\x41\x76\xaa\x4a\xc7\x3d\x86\x81\xaf\xd9\x79\xd5\xb4\x34\xfc\x62\x43\x0b\x3f\x7c\x45\x88\x80\x1f\x06\x93\x20\x90\xcd\x41"
	"\x41\xc1\xb1\x21\x4e\xa8\xae\x96\x55\xd6\x83\x9d\x8a\x8a\x6a\x82\xa4\xc8\xd5\xb2\x89\x97\xcd\xe2\x24\x0d\x93\x26\x7f\xd9\xfd\x72"
	"\xec\xbb\x48\xda\xe6\x98\x30\x4d\xbb\xb7\x9a\x2a\x2a\x18\x10\x15\xe1\x18\x27\x1b\x34\x61\xff\xc9\xe1\x5a\x25\x45\xaa\xab\x35\x75"
	"\x25\x98\x55\x31\x54\x54\x30\xb0\x7a\x58\xd4\x82\x49\xd9\x46\x41\xc1\x95\x35\x69\x53\x66\x5d\xd0\x84\x5f\xdb\x2c\x08\x56\xd3\x95"
	"\x44\x58\x30\x5c\x13\x75\x80\x35\x41\x61\x88\x28\xcc\x6a\xc0\x9a\xa0\xa4\xc8\x92\x70\xe4\xa2\x76\x2d\x0a\xc2\xcc\xbc\x88\x60\x21"
	"\xc1\x2c\x48\x56\x5d\x6c\xe2\x31\x49\x6e\xd5\x35\xb0\xf6\xd8\x30\x2d\x5b\x2c\x18\x58\x7a\xdc\x2b\x93\x24\xb7\xe2\x48\x35\x65\xdb"
	"\x62\xb1\xe0\xc1\x4b\x10\x49\xa6\x6c\xb1\xc0\x31\x51\x91\x96\x41\xd4\xd5\x24\x89\x02\x96\xa2\x8d\x7a\x54\xd3\x00\x01\x90\x69\xc6"
	"\x86\x98\x26\xce\xd8\xd2\xa1\xd7\x28\x6b\x7f\xf5\x37\x25\x56\xa5\xf0\xb0\x02\x2f\xb0\x83\x43\xc3\xc3\x43\xe3\x81\x8b\x44\x1a\x5e"
	"\x34\x0c\x02\x2e\xe2\xf7\xc1\x45\x5d\x87\x06\x3b\x0f\x9a\x15\x41\x2c\x0e\x38\x70\x00\x35\x34\x98\x5b\x20\x84\x17\x78\x81\x78\xe4"
	"\x38\x07\xcd\x8a\x1a\x10\x1e\x13\xf5\xe0\x21\x01\x35\x60\xdc\x69\x1a\xa6\xb6\x4c\xc2\xf8\xbd\x21\xc9\xc5\xee\xd5\x60\x07\x62\xd1"
	"\x2c\xc9\x0d\x47\xb0\xf3\xf0\x43\xba\x24\x28\xc7\x14\x11\x11\x11\x11\x11\x11\x11\xf1\x90\xae\xac\x52\xa9\x30\xd6\x6e\x39\xe3\x77"
	"\x6d\x2b\xbb\xd6\xf8\x72\xd6\x59\x61\xba\x4e\xd2\x8f\xd5\x56\x69\x29\x8c\x91\x4e\xff\xf8\xdd\xc2\x8f\x02\xf7\x20\xb8\xe1\x86\x17"
	"\x4a\x9a\x48\x14\x3b\x4c\xc4\x4e\xe7\x30\x04\x70\x01\xbc\x80\xb9\x07\xc1\x2d\xaf\x42\xe0\x6d\xb4\xd4\x3e\x74\xfa\x3f\x9b\xce\xe6"
	"\xde\x64\x7b\x90\x1b\xee\x1e\x83\x1f\xa6\x54\x3a\xee\xbd\x86\x2b\xdc\x63\x2c\xec\x10\xec\x34\x16\x16\xed\x21\x51\x10\x09\xe3\x87"
	"\x23\xf8\x9b\x05\x86\x92\xe4\xd8\x1e\x18\x75\x49\x24\x30\x89\xba\x24\x37\xbb\x94\x66\xc0\x02\x05\xd1\x4b\xd8\xa6\x71\x92\xd6\xb6"
	"\xa1\x09\x27\x64\x99\x54\x3d\x88\x6c\x16\xc6\xce\xa3\x01\xc1\x0e\xe3\x81\x41\x02\x92\x76\x37\xea\x4e\x24\x66\x81\x2c\xdd\x83\xf0"
	"\x41\xd9\x2c\x3c\x2c\x51\xf7\x91\xcd\xa8\x8b\x1f\x31\x11\x8c\xae\x46\x49\x91\x47\x18\x63\xcb\xff\xee\x39\x9f\xd7\x59\xed\x53\xb7"
	"\x0e\xe3\xdb\x96\x51\x94\xb2\x36\xb4\xb3\x7e\x84\x1d\x3d\xf4\x7f\x18\xbf\xa7\xf4\xf9\xfe\x23\xa7\xb6\xa9\x77\xf4\xaf\xee\xf2\x41"
	"\x8f\x3e\xd2\xf9\xd2\x42\x6b\xe7\xeb\xa8\x27\x95\x72\xc6\x0a\xa9\x8c\x70\x3e\x18\x4d\xc7\x22\x99\x8e\x1b\x8e\x8e\x77\xdc\xb1\x2c"
	"\xac\xc0\x0c\xc7\x0d\xb7\x63\x59\xaa\x49\xb2\x5c\x92\x4b\x3d\x08\x20\x40\xcb\x2e\x84\x1f\x86\xd9\x9d\xa6\xa5\x73\xc6\x87\x4f\x67"
	"\x9d\xf0\xc9\x59\xdd\xab\x43\x08\x23\x8c\xaf\xf2\xc3\xd2\xa3\x57\xdb\x0f\xe3\x6c\x08\x03\x1d\xb5\x60\x96\x45\x51\xf5\xa2\x0c\xe8"
	"\x2e\x86\x5d\x28\xf2\x4b\x62\x46\x9f\xd1\x29\xdd\x43\x2b\xe4\x0d\xdb\x56\x5a\x4d\x0d\x6d\xa5\x0c\x8c\xc8\x8f\x82\x02\x06\x0d\xb3"
	"\xba\xf8\x0d\x08\x46\x65\xc1\xd0\x70\xc3\x94\x87\x1d\x66\x52\x9f\xd1\xfa\x5b\x5f\xb8\x09\x42\xb8\xf7\x34\x1e\x60\xba\x6a\x7a\x4d"
	"\x35\x2d\x0b\x04\xff\xf5\x30\xe3\x61\x08\xd9\x55\x20\x3e\x6c\xc0\x33\xce\xc3\x0f\x2c\xce\x0d\x3b\x0b\x84\x76\x25\xec\x71\xd1\xaa"
	"\xa9\x92\x58\xab\x26\x19\xdf\x1b\x2e\x12\x42\x76\x25\xf1\x27\xac\x48\x72\x50\xd7\xbd\x29\x28\xd8\xd1\x2a\x09\x62\x5f\x52\x1a\xe3"
	"\x8c\x70\x5a\x09\x21\x8c\xdd\xfe\xdd\x73\x52\x6a\xdf\x69\x95\xb5\x7e\xb5\xd0\xdf\xda\x58\xad\x3b\x5c\xe4\x26\xac\xc0\x09\x78\x32"
	"\x91\xb0\x2d\xb3\x2e\x4e\xba\xae\x44\xaa\x2c\x8d\x23\x75\x9c\x56\xde\xe4\x6d\xf7\x26\xcc\xc3\xb6\xce\x37\x99\x44\x80\xef\x3d\x69"
	"\x43\x6a\x2d\x6c\xd8\x11\xba\x95\xd2\x9f\x4e\x19\x29\xd6\xc5\x0d\x7b\x5b\x1a\x25\x8d\x12\xbe\x96\xd1\xbe\xf5\x49\xdd\x3e\xef\x76"
	"\xe8\xfe\xf3\xe7\x5b\x97\xee\xd2\x2b\xfc\x7e\x08\x15\x30\xc7\x71\x8e\x08\x26\x61\x4d\x60\x97\x99\xc9\x34\x53\x3e\x7c\x43\x0e\x0e"
	"\x8d\xc6\x03\x3b\x10\x0e\xec\x34\x1a\xac\xc0\x0f\x0e\x0c\x0c\xa3\xc0\xb1\xbe\xed\x9e\xdd\xd0\xc6\x86\xfd\xe0\xc2\x09\xee\x56\x49"
	"\x9a\x76\x79\x01\x47\x60\xa4\x4f\x76\x14\xb8\xab\x70\x6f\x61\x87\xa1\x61\xcc\xb2\x0e\xe0\xa4\x89\x27\x00\xe6\xde\xc3\x7f\x27\x06"
	"\x4e\x6d\x0a\xf4\x01\x0a\x9a\x87\x21\x90\xc0\x20\xc3\x24\x78\x70\x20\x90\xd0\xb2\x89\xc1\x0b\x94\x6c\x9b\xa2\x1b\x31\x45\x26\x6c"
	"\x01\x8a\xff\xdf\xbd\x9b\xa1\x02\xb2\x02\x5f\xed\x46\x1c\x1b\x3e\x9a\x30\x07\x14\x2b\xba\x0f\x32\x29\xbb\x0e\x4e\xd0\xaa\x89\x21"
	"\xe9\x5b\x59\x61\x7d\x18\xff\x69\xb5\xf0\x6b\xff\x83\x0d\x1a\x1a\xdc\x70\x03\x19\xbf\x21\x1d\x62\xe3\x99\x87\x1b\xf0\xb3\x83\x5d"
	"\x6a\x83\x1a\x34\x44\x30\x80\x0f\x4a\x19\x25\x45\x48\xc0\x0f\x2f\x1c\xd5\xdd\x23\x95\xa6\x45\x2a\xcd\x25\x58\x55\x41\x09\x56\x79"
	"\xb3\xc3\x38\x14\x4a\xa5\x02\xdd\xbb\x99\xae\x73\xef\xe6\x5e\xc3\xdc\xe3\xa6\xd7\x0f\x51\x2c\x8c\x79\x6e\xea\x7e\x88\x13\xaa\x3b"
	"\xc4\x09\x52\xa2\x64\xd2\x63\x8a\xaa\xe9\xbd\xab\x09\x82\x84\xdb\xa6\x70\x70\xcc\xa4\xca\x62\x40\xe9\xfc\xad\xbf\x9d\x91\xd2\xcd"
	"\x3a\x6b\xd7\x34\x5c\x79\x94\xe7\x54\xba\xe7\x70\x4f\xab\xa6\xd9\x0d\x69\xd5\x34\xe9\xa4\x4e\x5d\xc6\x49\x4e\x1a\x34\x89\x1f\xbf"
	"\x3f\x02\xf4\x53\xfa\xec\x9f\xf1\x3d\x7e\xfc\xf8\xf1\x6d\x6d\x5b\xbb\xc2\x0a\x2b\x9c\x76\xda\x69\xe3\x7b\x7c\xef\x76\x1e\xca\x6e"
	"\x49\xa1\x7c\x28\x19\xbc\x00\x6d\xa5\x53\x32\x2c\x7d\x87\xaf\x7b\x81\x1d\x88\x04\xcc\x74\x4f\x36\xfd\x3a\x69\x84\xf4\xfd\xa2\xc0"
	"\x0c\x59\x17\x70\xc5\xd6\x14\xb2\x4a\x59\x1f\xb6\x54\x42\x4c\x59\x96\x63\x81\x7b\x1e\xf1\x08\x84\x50\x1a\x66\x38\x93\x6d\xb2\x31"
	"\x1a\xb6\xdf\x11\xca\xf2\x1d\x59\xf8\x3f\xe2\xee\xdd\xed\x44\x38\x4e\xb6\xc9\x36\xd9\x18\x98\xb4\x37\x8c\xb4\xa5\x96\x5a\x6a\xa9"
	"\xbf\xbf\xbf\xbf\xa5\x96\x5a\xda\x20\x1d\x2a\xa3\x9c\xb1\x67\x94\x91\x89\xcf\xd9\x91\x95\x5a\x4a\xbb\x25\x9c\x75\xfa\xb7\x5b\xea"
	"\xf3\x63\xf5\x48\x9d\xce\x4a\xa3\x5d\x64\x6c\x6b\x63\x85\x12\x4a\x58\xdf\xda\xa7\x14\x3e\xf8\x0e\x23\xf5\xea\xd4\xe3\x83\x35\xe1"
	"\x88\x60\x19\x57\x6f\xc7\xa6\x86\xee\x62\x93\x8d\x63\x43\x57\x26\x3d\xa6\x06\x6b\xe2\x81\x76\x49\xd1\x35\x51\x8f\x07\x23\xe4\x0b"
	"\x0b\xf7\x32\xeb\x5a\x32\xeb\xca\xac\x8b\x1d\x46\x88\x13\xfc\x81\x6b\xa2\x20\x0d\x4a\xf7\x6e\xdf\x6d\xc7\xcd\xe8\x1f\x1d\xf9\xf4"
	"\x23\xb5\x4f\xe5\x53\x3a\x65\x7c\x6f\xc8\xa1\x3b\x74\x26\x63\x07\x66\xed\xed\xdd\xed\x95\x25\xaa\x36\x0b\xdb\xd8\xe9\xb8\xe7\x3c"
	"\x0c\x29\x76\x2a\xdc\xbb\x60\x37\xc4\x4f\xf1\x02\x77\x14\xa4\x9a\xa2\x01\x85\xa2\xa8\xf3\x57\xc3\xa0\x74\x0c\xcd\xc8\x00\x08\x10"
	"\x80\x00\x43\x13\x60\x60\x50\x60\x26\x16\x09\x44\xe2\x26\x4a\x7b\x13\x80\x47\xc5\x02\x71\x24\x18\x0a\x85\x83\x31\x90\x03\x29\x8a"
	"\xc1\x28\x04\x21\x03\x08\x30\x00\x18\x43\x80\x31\x08\x4d\x0d\x86\x28\x00\xf8\xa4\xf2\xaf\x4b\xa2\xf2\x97\xca\x70\xce\x88\xb3\xe3"
	"\xa8\x6b\xfb\x43\xbe\xda\xc2\xa8\x7c\x73\xc6\x9c\x0d\xc7\x6b\xed\xa3\xdd\xbc\x66\xc0\x2d\x71\xe5\xc8\x9f\xcb\x8a\xc2\xa9\x70\x86"
	"\x85\xc6\x14\x02\xcc\xe4\x03\xdb\xcd\x6d\x19\x46\x97\xba\x11\x44\x0c\x11\x5c\x8b\x85\xa4\xe8\xe0\x8f\xe2\xd4\x3b\x84\x97\xe2\x11"
	"\x29\x73\x82\xf0\xf9\x1f\x95\xaf\xab\xc9\xb0\xa5\xbe\x2f\x20\x11\x8b\xc4\x23\x95\xa0\xb1\x2b\xc3\x77\xba\x31\xed\x1f\x83\xbe\x76"
	"\xdf\x33\xc0\x89\x03\x7e\x84\x9b\x5a\x89\xb9\x35\xb4\x3a\xf9\x78\xd0\x41\xc2\x89\xc1\xea\xeb\xd5\xc2\xa6\x95\xcc\xb0\x6f\x9b\xd8"
	"\xf2\x20\x01\x62\x38\x65\x69\xfb\x2c\x9f\x00\xbc\xcf\x45\x98\x80\xb2\x21\xc6\x38\x68\x99\x0b\xd2\x2b\xc6\xbd\x76\x73\xa7\xa4\x2e"
	"\x2f\x29\x79\x6a\x73\x59\xbc\x15\x33\xc7\x4d\xd0\x7a\xda\xed\x17\x75\x00\xc6\x9b\xbd\xae\x30\xaf\x32\xf0\x85\xf4\x92\xb4\xcf\xd5"
	"\x9b\x77\xa5\xbe\xf1\x2e\x26\x51\x76\x35\x90\x53\x3f\xf1\x84\x31\xd9\xc5\x8c\xf1\xa7\xc8\x8e\xf9\xa1\xe8\xae\xa9\x18\xb0\x9c\xae"
	"\x56\xa3\x20\xfd\x9d\xaa\x11\xfa\xeb\x19\x8e\x22\xfc\x70\xf3\xe5\x4b\x0e\xb8\x6a\xae\xba\xae\x10\xe2\x32\x77\xb6\xaf\x05\x65\x15"
	"\x54\x08\x2e\xf2\xca\x09\x6f\x59\x6a\xd1\xb9\x4e\xd8\xe8\xd8\x27\xf9\x2c\x9a\x8a\x5c\x9f\x81\x59\x87\xb7\xc4\xd2\xb2\x3b\x4a\xf3"
	"\xf1\xc6\xc9\xb6\xae\x71\x42\x95\x76\x11\x2b\x22\x6f\xa8\x4e\x86\x87\x0a\x64\x8b\xe7\x39\x95\x9e\x2e\xde\x39\x24\xe3\x8b\xb7\x63"
	"\x0c\x61\x7a\x9c\x5a\x1c\xe5\xf8\x83\x19\x2f\x8b\xae\x82\xc6\xab\x03\x06\x0f\x96\xbf\xd5\x42\xf1\xd4\x50\x24\x07\x3f\xe3\x3e\x39"
	"\xd8\x29\x54\x05\x2b\x7a\xc2\x69\xd2\xfe\x2f\xb7\x9f\x91\xfe\x9c\xa0\x83\x49\xc7\xfe\x0f\xe4\xcf\xef\xa7\xc7\x9d\x5d\xd7\x80\x94"
	"\x4b\xf5\x98\x10\x74\xd5\x21\xa0\xcf\x83\x7f\x55\x82\xfa\x33\xce\xcd\xd5\xd4\x1b\x05\x15\xf3\xf4\x72\x01\x51\x56\x9e\x23\x3a\x42"
	"\x01\x09\xa2\x09\xb9\x23\xe8\xea\x8e\xe7\x28\xa1\x8c\x2e\x06\x58\xad\x31\x58\x3c\xb3\x9c\x2c\x37\x45\x46\xdb\x22\x97\x93\x6b\xa7"
	"\xfe\x72\xe6\x42\xe2\x2c\xe3\x38\xa4\x7a\xf9\x74\xc1\xe3\x46\x3a\xef\xf8\x5c\xe8\x56\x3e\x9f\xd6\x0b\x15\x66\x3a\x0a\x55\x0d\x5b"
	"\x04\x5c\x14\x90\x3c\x5f\x82\xb0\xf8\xa7\x4f\x12\x3b\xcf\x26\x33\xa8\xcd\x1a\xfe\xb6\x05\x2b\xca\x52\xcd\x4e\xff\xc5\x20\x10\x49"
	"\xb8\x9e\x5a\xb0\x08\xf9\xc9\x37\xbc\xea\x54\x4e\x9b\x1c\x99\xba\x0c\x82\x15\x15\xd3\x47\x20\xd2\x1a\x40\x1f\x69\x8c\xe9\x9a\x7a"
	"\x1e\x56\xce\x5a\x8f\xd2\x5f\x1c\x2f\x7a\x53\xb4\xe8\x90\x4c\x06\xd5\x61\x8a\x16\x9d\xfd\x33\x19\x8a\x8f\x22\x68\x7b\xd2\x2e\x0e"
	"\x57\x5a\x49\x6f\xbb\xa7\x66\x89\xa3\xa1\x72\x27\xe9\xdb\x42\xef\x81\x23\x15\x76\x73\x37\xdd\x85\xbf\x02\x8f\xab\x3a\x35\xa7\x3b"
	"\x3d\xb5\x66\xfc\x9d\xca\xe5\x72\xe0\xa9\x71\xeb\x22\x1d\xe8\x45\x96\x5f\xa5\xbe\x5d\xff\xdf\xa0\x48\x8c\x0e\x64\xe4\xa9\x9b\x02"
	"\x02\x8a\x24\xfe\x7f\x76\x87\xd2\x12\x53\x41\x58\x7c\x7c\x3e\x25\x34\xc3\x05\x44\xb2\xc8\xed\xb9\xfc\x38\x5a\x2e\xc9\xea\x8f\x58"
	"\x91\x26\x86\xee\x73\x2f\x3e\x20\xe5\xd4\x10\xde\x3f\xe8\x11\x4c\xca\x9c\xcc\xa3\x05\x55\x82\x12\x60\x71\xfc\xac\xef\x70\x36\x00"
	"\xe6\x27\x81\x17\x97\x72\x57\xe7\x2d\xee\xac\xe9\xae\xe3\x8d\x8d\x4c\x25\x78\xcb\xe1\x36\x17\xf0\x17\xcc\xb1\x47\x75\x67\x01\x81"
	"\x67\x6b\x54\x87\x30\x56\x59\xa1\x23\xa6\x13\x0e\xe9\xe3\x95\x13\x85\x98\x3b\x63\xf5\x93\xb2\xaa\x69\x5f\x3e\xe8\xc8\xc2\xfa\xec"
	"\xe2\x43\x9f\x94\xa2\xd0\x12\x48\xd4\x24\xdb\x50\x70\x43\x4a\x7d\xc7\xca\x26\x0b\x19\xbe\xf3\xa6\x82\x7a\x33\xb2\xa4\xb8\x28\x6d"
	"\x4c\xb3\xfa\x3f\xc6\x09\x03\xf1\x43\x1c\x15\xbf\x1f\x1d\x25\x30\x31\x7b\xac\xe1\x31\x56\xc1\x5c\x31\x10\xfd\x24\x20\x4c\xeb\x85"
	"\x1b\xa6\x62\x32\x40\xc6\x36\xc0\xe1\xbe\xd4\xa4\x22\x90\x6c\xa2\x5f\x9e\x7d\x1a\x01\xb8\x29\x8c\x4d\x89\x39\xe8\x93\x27\x3a\x81"
	"\xcc\x98\x08\x8d\xa8\xb3\x4a\x3f\xc8\xdb\x58\xb3\xe5\xa6\xf0\x03\x38\xf2\x4d\x13\xcb\xe5\xa8\x25\x94\xe8\xfb\x51\x72\xa6\xea\x05"
	"\x77\x37\x3f\x7a\x77\x5d\xbe\xbb\x94\xd5\xaa\x43\xe6\x9e\x27\x50\xc3\x04\xcf\x50\xa0\xf9\x19\x29\x98\x2e\x8d\x39\x9c\x54\x21\xde"
	"\xfa\x3d\x99\xea\x2f\x08\x46\xa4\x8e\x01\xf0\x82\x8a\xad\xda\xfb\xe1\x30\x84\x85\x48\xb2\xa7\x65\xa6\x44\xe4\xb3\xe0\x43\x12\x3c"
	"\xd3\xc5\x7c\x3f\x11\xc7\x3b\x8c\x18\xfa\xcc\x01\xe1\x4c\xc0\x27\x89\xd9\x32\xbe\x1f\x32\x9d\x1d\xb9\x87\xa6\x4b\xc4\x81\x8d\xb6"
	"\x5e\x9b\x6d\xd8\x4d\x55\x78\x63\xf9\xf8\x97\x39\xc2\x09\x5a\x19\x37\x49\xe7\xef\x77\x84\x0c\x4a\x81\x00\x12\xed\x39\x01\x44\x91"
	"\x58\x4f\x2a\x70\x81\x1f\x82\x37\x43\xc2\x27\x57\xf8\xdc\x9c\x68\x28\xb6\x78\x82\xb7\x25\x96\x3e\x31\x27\xc1\x25\x90\x68\x4a\x0c"
	"\x7b\x01\xe6\x13\xca\xf9\xa2\x04\x50\xd8\x22\x45\xaa\x83\xe2\x4a\x2b\x79\xb6\xf2\x02\x9e\xdf\xd4\xe0\xa2\x71\xd6\x0e\x21\x49\x39"
	"\xc3\x56\x97\x10\xfd\x15\x70\xf7\x41\xda\x02\xe2\x3e\x4b\x90\xde\x3f\x8b\x7f\xbe\xc4\xaf\xee\x03\xf1\xf6\x71\xb4\x0b\x67\x9b\x28"
	"\xae\x66\x76\xe4\x82\x9a\x0c\xc7\x91\xe5\x34\x8f\x05\x4d\x48\xda\x96\x52\xb8\xba\xb4\x53\x4e\xa8\x56\x24\x48\x00\xe6\x30\x63\x8f"
	"\x14\x7c\xed\x15\x53\x86\xd5\x68\x88\x98\xe1\x0a\x13\x6e\x6a\xe7\xa0\xa1\xfa\xa1\x5e\x7f\x5f\x8c\xc7\x65\x0b\x2b\xcb\x4a\x6e\x24"
	"\x4b\xbf\xb0\xe9\x9b\x71\x59\x63\xd2\xed\x44\xa7\x8c\x61\x4f\x12\x2f\xbc\x34\x0c\x0e\x67\x1c\x12\x68\x96\xfb\xf8\x5d\x9c\x07\xfd"
	"\xf0\x38\xcb\x07\x5c\x55\xbe\x85\xe2\xfd\x9c\xf2\x2f\x9f\xd0\xfe\xf3\x78\x78\x41\x00\x84\xf9\x60\x58\x5a\x1d\x7d\x52\x40\x96\xe6"
	"\xdd\x6b\x4d\x0d\xb2\x2d\x3c\xb1\xa1\xd1\xf5\x62\xc6\xc5\x92\xe3\xea\x6d\x51\x0f\xf5\x86\xce\x99\x47\x05\x45\xf0\x4d\x0f\xcb\x04"
	"\xfb\x82\x09\xaf\xdf\x81\x61\x02\x8c\xa6\x5c\xf2\xfa\xb1\x25\xf2\x46\x00\x12\x8b\x15\xeb\xdc\x48\x20\xf6\x0b\x09\x56\x65\x26\xf8"
	"\xc5\xe8\x5e\x65\xb6\xdc\xfa\xdb\x63\x16\xe4\x1e\xe9\xff\x6b\xa5\x98\xc0\xb0\x5d\xb6\xf2\xdd\x3e\x86\x8c\x25\xd3\x16\x9e\xc1\x62"
	"\x4b\xb4\x2b\x20\x8e\x77\x75\x09\x48\x69\x43\xd8\x87\xac\x2f\xd8\x0c\xb7\x5c\x5c\x50\x6d\xe0\x5f\x43\xde\x9d\x7d\x11\x27\x21\x7d"
	"\x69\x98\x91\x36\x3f\x12\x6b\x49\x00\x69\x73\x61\x9e\x29\x82\x4a\xcf\x8b\xf6\x6c\xa4\x98\x8e\x32\x45\xfe\x5b\xcb\x02\xed\x06\x17"
	"\xad\xf4\x1f\x30\x13\x0e\x2d\xb8\x50\xe9\xfb\xd6\xb7\xec\x26\x41\x7b\x3c\x54\xb4\x0e\xb6\x27\x9a\xc7\xda\x40\xaf\x52\x5e\xd6\xac"
	"\x94\x73\xe6\x9d\x48\xe1\xc4\xb8\x65\x91\xc7\x94\x9b\xb5\x23\x9d\x09\xb0\x51\xf0\x36\x16\x01\x87\xea\x5b\x5a\x43\xdb\xc9\x3a\x4c"
	"\xa5\x2e\x22\xc7\xaa\xab\x5e\x50\x9d\x4b\x47\x6e\x8b\xc6\xfc\xc7\x80\xf6\x57\xee\x45\xbf\xab\xd4\xe0\x91\x7c\xdb\x2b\xe6\xd6\x17"
	"\x1c\x08\x62\xc9\x08\x83\x14\x39\x2a\xe8\xe4\x88\xb1\x80\xe1\x10\x50\x86\x75\x9c\xd5\x76\x13\x4f\xb0\x42\xdd\x54\x2d\x1d\x67\x2d"
	"\x35\x23\xbc\x3e\x1a\xe8\xce\xd4\x29\x72\x9f\xba\x5f\xac\x55\x39\xb9\x4d\x51\x68\xaa\x7d\x5e\x24\x7e\x44\xc6\x4d\x4c\xe0\x7b\x10"
	"\x4e\x2b\x30\x40\xc8\x1f\x0c\x6b\x35\x00\x92\xd1\xde\x12\xfd\x4d\x28\x5f\x77\x01\xde\xbe\xac\x0f\xbb\x40\x80\x37\x3f\x19\x21\x54"
	"\xb9\x58\xa3\xfb\x51\x6b\x8d\x06\x78\x56\x5c\x30\x77\x44\x5b\x22\xbb\x31\x23\x79\xf4\x8c\xa5\x44\x25\xde\xf4\xfc\x70\x3c\x9e\xb9"
	"\xdc\x53\xee\x62\xa7\xe3\xd4\x22\xf7\x34\x5b\xfe\x58\x89\x3f\xad\xb5\x39\xa2\x18\x4b\x0b\x6c\x78\x25\xf4\x7b\x28\xf6\xe8\x56\xbe"
	"\x01\x12\xbc\xb8\x1a\x53\x3d\x37\x5d\xfb\xb3\x11\x68\x15\xad\x7d\x3c\xf7\x0f\x3f\x11\xe7\xca\x06\x93\x38\x7e\x53\x08\xb8\x1b\x22"
	"\xa6\x23\xab\xcc\xcd\x0a\x67\x6b\x3f\x79\x9f\xbf\x08\xfe\xd7\x55\x92\xdd\x64\x05\x58\x92\x11\xb3\x45\x62\x09\x76\x81\x25\xc0\x43"
	"\x15\x7d\xf3\x09\x13\x04\xf5\x33\x20\xbf\x57\xb2\x94\x3c\x78\xbb\x0e\x09\x80\x63\xb6\xa8\xab\xc7\xcb\x2e\x58\x4c\x13\x55\x63\x48"
	"\x19\x98\xcd\xec\x18\x27\xcc\x1b\x1c\xa7\xf5\x15\xeb\xe2\x6f\xc5\x6a\x6f\x8e\xec\xc4\xec\x73\xf5\x46\x02\x9d\xd2\x3a\x5d\xb0\x13"
	"\xa7\xf8\xc9\x65\x55\xbf\x00\xd1\x06\x29\xc7\xef\xfd\x26\x5f\x44\x8b\x8b\x90\xb2\xf8\x08\x8d\xa2\xa4\x21\x8e\x0c\x80\xec\xbc\x5b"
	"\x09\x40\xc0\xd9\x0f\x08\x5c\x08\x13\x19\xad\x18\x8d\xb3\xc2\xb6\xea\xb2\xc3\xc5\x00\xf2\xc0\x70\xc8\x46\x3c\x84\x01\xac\x1c\x59"
	"\xb0\xb5\x9d\xa0\x09\x48\x47\x8b\x45\x93\xff\x52\x3f\x52\xd8\x9d\x0e\x05\x5b\xa5\x23\x5d\x6b\x36\x68\x6d\x42\x63\xdf\xac\xe2\xe7"
	"\x39\xc6\x06\x44\xdd\x4d\x72\xe5\x55\xce\x89\xa2\x0a\x80\x8c\x74\x4c\x66\xb7\x00\x62\xa5\x3a\x52\x00\x5c\x0f\x5b\x6a\x96\xed\xbf"
	"\x6a\x32\x7f\xe3\xf4\xe8\x0b\xb6\xd1\xce\x16\x7a\x82\xdb\x2a\x9c\x7f\xf2\x21\x69\x89\x84\x5d\xda\xfb\xd5\xb3\xff\xe7\x6c\xb0\x93"
	"\x60\xc0\xb5\xf1\x59\xa8\xf8\xe2\x54\x7d\xc4\x99\x28\x7c\x37\xf4\x7f\x72\x06\x0c\x8a\x6c\xc8\xd1\x50\x22\xe0\x5e\xc1\xb7\xd7\x9e"
	"\x44\x6c\xe9\x4d\xe6\x36\x6f\xde\xed\xf4\xcb\x36\x63\xed\x05\xc2\x34\x37\xd6\x6b\xc8\x83\xcf\xad\x0f\x39\x90\xc6\x5c\xc6\x4e\xe6"
	"\xb0\x68\x14\x42\x66\x85\x5c\x9a\x3c\x73\x8f\xae\x58\xbc\x75\xde\xf9\xea\x3b\x8a\xf0\xfa\xeb\xcd\x2b\xd3\x3a\xa7\x8d\x97\x05\x18"
	"\x1e\x46\x01\xd3\xcd\x60\x3f\xc6\x3a\x59\x27\xe7\xb9\x7a\x61\xc1\x51\x76\xa2\x4c\x9e\xc9\x7a\x32\x09\x88\x1f\xa0\xd3\xab\xbd\xaa"
	"\x64\x7a\x30\x2c\xa4\x8a\xae\x33\x43\xec\x33\x65\xcd\x12\x23\x41\x20\x60\xe6\x1c\xd9\xd9\x7b\xa1\x2e\xd0\xb1\x77\x96\xae\x04\x44"
	"\x91\x62\xa7\xab\x3e\x36\x8b\xdf\x4a\xae\xdd\x0a\xd8\xe2\xc4\xe9\x1c\xa9\x86\xf6\x74\x73\x23\xfe\x08\xed\x25\xb2\xac\x9a\x70\xf6"
	"\xae\x0d\xca\xa5\x71\xe4\x97\x61\x48\x63\x90\xf5\x32\xdb\xf9\x07\x4e\x09\x24\x74\x37\x21\xb3\xd5\xd8\x26\xc2\x16\x30\x8e\x3f\xe4"
	"\xca\x86\xa4\x7d\x9c\x5b\xc0\x14\xe3\x0c\x7b\x1a\x40\xf4\x37\x04\x15\x42\x87\x8e\x0a\xcb\xd7\xb7\xea\x5a\xc1\x22\x9f\xba\x11\x04"
	"\xad\x4b\x1c\x05\x42\xb6\xa3\x5b\x38\x1e\x26\x5c\x30\xab\xd6\x60\x96\x8f\x20\x16\x0c\x4b\xe4\x63\x04\x0a\x61\xf0\x01\x4a\x4d\x7c"
	"\x00\xa0\x8d\x0e\x0b\xdc\x3b\x9e\x7a\x7e\xf6\x7d\x78\x75\x60\xc1\x23\x05\x4a\x87\x8a\x23\x40\x85\xd1\xe3\x29\xc1\x09\x86\x8f\x53"
	"\x82\x83\xfe\x53\xc8\x32\x8a\xad\xa9\xd5\xd2\x72\xde\xab\x9d\xc4\x2c\x99\x58\x0a\x46\x8f\x0d\x1c\xad\xdc\xce\x8e\xd8\x48\x61\x85"
	"\x69\x69\xec\x90\x7e\xc2\xd2\x60\x86\x64\x57\x3c\xd8\x79\x78\xf3\x4d\xfd\xcd\x2f\x84\x64\x0d\xb2\xea\x59\xf9\x3d\x9f\xb3\x93\x56"
	"\x73\xbd\x9e\xc4\xa4\x71\x76\x82\x63\xd3\x00\xc9\x54\xa0\x85\x40\x2e\x22\xfa\x6b\x47\x1f\xf4\x68\xb1\x87\xb4\x96\xa7\x41\xb8\xeb"
	"\x7f\xf3\xe8\x04\x2f\xb5\x91\x5e\x3b\x4d\xeb\xe9\x0d\x09\x2d\xb5\x57\x9a\x88\x88\x7c\x9c\x4d\xc0\x1a\xd2\x38\x44\x4d\xf3\x95\xe4"
	"\xb5\x10\x5c\xc3\xf3\x1c\xff\xb8\x29\x5c\x0c\xb0\x2d\x6d\xa3\x05\xab\x90\xb5\x2a\xce\x8a\xfa\x5b\x4b\x10\x21\xc6\xb1\xc6\x35\x39"
	"\x29\x60\xd8\xb7\xb8\x51\xa0\x57\x7e\xa1\x6e\xbc\xa2\xc3\xb7\xe9\xad\x93\xdf\xdd\x45\x37\xa0\xff\x0c\xed\xe8\x24\xcd\x5f\xa4\xfe"
	"\x57\x68\x6c\xcb\x31\x43\x44\x94\x1b\x53\xf5\xa9\x19\x95\x22\x6e\x50\x13\xe6\xda\xb7\xab\xfe\xa5\xf0\x6f\xaf\xe2\xfb\xe3\xbf\x5d"
	"\x5e\x3d\x78\x92\x1c\xbf\x5b\xba\x7e\xb6\xa5\xf8\x6e\xa1\xc9\x04\x75\x53\xc1\x57\xae\x09\xce\x8d\x49\x44\xec\xcf\x98\xe7\x8d\x0e"
	"\x51\x94\x3d\x31\xfe\x24\x36\xc4\x08\xb7\xca\xe2\x79\x3c\xcf\x44\xf6\x4f\x9e\x71\x10\xa9\x90\x35\x8b\x57\x23\x44\x6a\xd0\x65\xba"
	"\x5e\xc3\xe3\x88\x94\x1b\x5e\x46\x73\x1c\xbc\x5a\x9d\xb8\x14\xe0\x91\x21\xed\x14\x6d\xb7\x9f\xa9\xcb\xb8\x30\xea\x78\xdf\x32\x82"
	"\xfd\xed\x3f\x6b\xd8\x50\x86\x5d\x32\x3d\xce\x81\xad\xdb\x3c\x13\xb5\x09\x26\x2c\xc3\x04\x5d\xcb\xaf\xfe\x83\xa5\xd9\xe6\x3c\xbe"
	"\xc7\xab\xeb\x64\x0a\x4a\x79\x8c\x9e\xba\xf0\x17\x2f\x9f\x1e\xda\xa8\x8a\xef\x70\x2c\x3e\xd8\x48\x32\x8b\xc6\xe9\x01\x27\x5d\xd7"
	"\x70\xa9\x1b\x85\x92\x95\xdf\x01\xf2\xfe\x7d\xc3\x4c\xb3\xc0\xe0\xf1\xfa\xda\xe2\x29\x70\xc9\x40\xc0\xc5\x76\xcb\x1d\x27\xcc\x75"
	"\x33\xb0\x03\xc1\xfd\x6d\x7c\xf2\x47\x08\xba\x5a\x14\xc3\xcc\xe9\xa8\xad\xab\xb5\x62\x62\x24\x94\xb4\xa6\x80\x0e\xf3\x11\x29\xe3"
	"\x42\x2e\xc5\x66\x94\xa6\x59\x8c\x3b\x34\x1d\x4d\xee\xfd\xfd\xba\x18\x17\x01\xcc\xb3\xa7\xc1\x3a\xe9\xbd\x80\xe4\x46\x48\x61\xee"
	"\x81\x34\x1f\x04\x91\x66\xb6\x41\x5e\x34\xa3\xf8\x57\xa4\xb3\x72\x7c\x05\x68\x46\x33\x55\xeb\x95\x89\x87\xa0\x38\xc3\x31\xd9\x98"
	"\x84\x2b\xa2\xb2\xba\xdd\x4e\xf5\x9f\x85\xd4\x57\xc6\x3d\x92\x70\x54\xca\xfc\xef\xfb\x27\x5e\x97\x79\xa4\xb5\x2b\xd0\x30\xc4\xda"
	"\xaf\x60\xf3\x5d\x6e\x47\xf3\x07\xf0\xfd\xed\x25\xfe\x15\x1b\xe4\x69\xb9\xdf\x3a\x4c\x94\x3c\xe0\x94\xa6\x74\x8a\x0b\xa6\x09\xb4"
	"\x62\xce\x18\x15\x0b\x34\xaf\xab\x77\xa9\xb1\xae\xd8\x27\xe5\x95\xb0\xe1\x52\x56\xf1\x57\x32\x90\xe8\x2c\xd2\xde\x76\x11\x42\x2f"
	"\xd8\xe2\x5c\x37\x58\x29\xcd\x95\x69\x7f\x27\x01\xf6\xb5\x43\x2a\x34\x93\x1b\x6e\xdc\x8e\x3f\xe6\x38\xaa\xb5\xfd\xa8\x9e\x82\x3d"
	"\xcf\x50\x32\xf2\x13\x75\x8c\x4c\x1d\x52\x5a\x5b\x0d\x74\x89\x96\x00\x0c\x96\x8f\xad\x4b\xbc\xa5\x1d\xbe\x74\x03\x26\xe2\x70\x05"
	"\xb7\x4e\x4a\x79\x8c\xf0\x8b\xa1\x76\x79\x14\x6c\x63\x49\x89\x7d\x66\x1b\x82\x1d\x01\x9d\xd9\xf4\x7a\x35\xcd\x07\xe9\xa1\x38\xfe"
	"\xbe\x74\x35\x32\x9b\x3c\xab\x8c\xd0\x0f\xc6\x66\x90\xba\xf2\x75\x24\x82\xf9\x00\x09\xe9\xc9\xf5\xa9\xdc\x81\x1b\x93\x54\x0c\x51"
	"\x38\xfd\xb6\x54\x69\xb8\x04\x2d\x2f\x27\x48\x11\xf0\x13\xe7\x03\x90\x5c\xda\xd5\x62\x5e\x34\x70\xd6\x64\x13\x0b\x7c\x7c\x98\xf1"
	"\xf7\xbe\x25\xe6\xae\x01\xc0\xae\x3f\x6c\xdd\x0e\xf4\xed\xdf\x86\xc1\x7e\x05\xb4\xd5\xeb\x2b\x42\xa0\x30\x91\x23\x8e\x33\x00\x8c"
	"\xcd\xfc\xb2\x72\xf4\x0b\x6f\x62\x68\x9a\xe0\x66\x2e\x20\x19\x3a\xea\x2e\x60\xf9\xca\x33\xe5\xaa\xb9\x5a\x07\x15\xc7\x1b\xeb\x73"
	"\x61\x1b\x4e\x9d\x8d\xac\x36\xec\xa2\x38\xb0\x9e\xa6\x14\xa5\x67\x8c\x03\x36\x5f\xb0\xeb\x31\x75\xa7\x8e\xcb\x7b\x9c\x82\xbb\x9b"
	"\xc8\xe3\x7f\x11\x0a\xe8\xe3\xb4\x00\xfd\x6c\x82\x6e\x79\x7e\x26\x1d\xbd\xf3\x0a\xfa\xc3\x6b\x0a\xc2\x88\x15\xc6\xf2\xe3\x36\xd4"
	"\x2f\x2e\x38\x8a\x47\x27\xe3\x38\x36\x3a\x4f\x27\x60\xdf\x85\x1d\x96\xcd\xe4\xa7\x6a\x67\x4b\x33\x2b\x52\xa4\x40\x2b\x1f\xe2\xff"
	"\x63\x91\xa2\xbf\xd0\x16\xb0\x43\xe6\x0f\xdb\x33\xdc\x79\x1a\x28\x93\xb2\xc4\x5e\x70\x09\x03\x6c\x67\x3b\xd4\x1f\x7a\xfd\xf6\xab"
	"\xc8\xcb\x58\xfa\x8b\x08\xf2\x16\x3c\xb3\xd4\x28\x94\x73\xba\x4f\xb7\xd0\x4b\xb4\xc0\x5b\x81\xef\xb3\xd9\x8b\x72\x02\x7f\x9e\x30"
	"\xa8\xac\x91\xa0\xda\x29\x3b\x7c\xff\x69\x19\x98\x39\x56\x34\xd3\xc8\xbd\x76\xa0\xbb\x96\xa2\xe2\xfa\xdf\xe3\x3c\x01\x80\xaa\x10"
	"\x56\x75\xe6\x30\x83\x43\xb2\xce\xb9\x00\xf4\xd9\x14\x98\x64\xf7\xd6\xcd\xac\x42\x51\xff\x60\xb6\x6a\xf6\xfe\x2d\x48\xba\x52\x08"
	"\x78\x67\x9d\x7d\x30\xc3\x62\x58\x14\x10\x52\xe7\x1a\x55\x85\xbe\xa7\x3c\x5a\x52\xa4\xd6\x47\x2f\xd0\xdd\xf2\xdf\x43\xaf\xa8\x24"
	"\xdf\x7f\xf3\xb8\xaf\x24\x16\xd7\xad\x2f\x5d\xc6\x14\x09\x0a\x87\x44\x5a\x52\xfe\xf8\x47\x40\xd4\x1a\xc9\xc5\x20\x39\x5f\xd8\xcf"
	"\x2f\x76\x08\xb6\xc5\x86\xd0\xe4\x07\x4d\x00\x00\x08\x32\x01\x00\x28\x2d\x1d\x08\x01"
;