#include "GameShaders_ios.h"
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
	{ { 1 }, {  }, { 1 }, 0, 1712 },
	{ { 2 }, { 1 }, {  }, 1712, 3048 },
	{ { 3 }, {  }, { 1,2,3,4 }, 4760, 1167 },
	{ { 4 }, { 2,3,1,4,5,6 }, {  }, 5927, 10958 },
	{ { 4 }, { 2,3,1,4,5,6 }, {  }, 16885, 10958 },
	{ { 4 }, { 7,3,1,4,5,6 }, {  }, 27843, 11635 },
	{ { 4 }, { 7,3,1,4,5,6 }, {  }, 39478, 11635 },
	{ { 5 }, {  }, { 5 }, 51113, 609 },
	{ { 6 }, { 2,1,8,9 }, {  }, 51722, 9169 },
	{ { 6 }, { 2,1,8,9 }, {  }, 60891, 8754 },
	{ { 6 }, { 7,1,8,9 }, {  }, 69645, 9846 },
	{ { 6 }, { 7,1,8,9 }, {  }, 79491, 9431 },
	{ { 7 }, {  }, { 1,2,3,4,6 }, 88922, 2196 },
	{ { 4 }, { 2,3,1,10,11,12 }, {  }, 91118, 11010 },
	{ { 4 }, { 2,3,1,10,11,12 }, {  }, 102128, 11010 },
	{ { 4 }, { 7,3,1,10,11,12 }, {  }, 113138, 11687 },
	{ { 4 }, { 7,3,1,10,11,12 }, {  }, 124825, 11687 },
	{ { 8,9 }, {  }, { 1,7,8,9,10,11 }, 136512, 2089 },
	{ { 4 }, { 2,3,1,4,5,6 }, {  }, 138601, 10958 },
	{ { 4 }, { 2,3,1,4,5,6 }, {  }, 149559, 10958 },
	{ { 4 }, { 7,3,1,4,5,6 }, {  }, 160517, 11635 },
	{ { 4 }, { 7,3,1,4,5,6 }, {  }, 172152, 11635 },
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
	"\x28\xb5\x2f\xfd\xa0\xeb\xcd\x02\x00\xa4\xb5\x00\x4a\xa7\x4c\x1e\x30\xb0\x4c\x8a\xcc\x00\x90\xc1\x0e\x80\x0e\x0c\x41\x5d\x4f\xe8"
	"\x4c\xef\xd5\xa3\xb0\xb8\xf5\x9a\xc2\xd9\xf1\x16\x3c\x08\x2c\xbb\xc9\xde\xff\xf8\xd6\x65\x85\xb1\x9b\xff\x07\xfd\x47\x7a\x77\xd8"
	"\x04\xf7\x01\xc4\x01\xe2\x01\x06\x74\x03\x1f\xec\x85\xe6\x1e\x04\xc3\x39\xcc\xa3\x14\x44\xeb\x7f\xe5\xad\x37\xc2\xaf\x31\x52\xaf"
	"\xd4\xb9\xb7\xcd\x1a\xa5\xbd\x76\xde\x0a\xa5\xac\xf4\xa3\x8c\x0f\x76\xa3\xbb\xf7\xe8\xd7\xe8\x0b\xf7\x1e\x9b\xd6\xdf\x5c\x93\x55"
	"\x91\xb8\xf2\x64\x55\x30\x86\xe1\x29\x3b\x60\x21\x41\x14\xf3\xa5\x65\x5c\xce\x68\x84\xb6\x3e\x90\x40\xb4\xf3\xe8\x83\x08\xe7\xcb"
	"\x22\x63\x18\x1e\x10\x50\x20\x67\x58\x9b\xf0\x96\x19\x4d\x03\x61\x1d\xb6\x4c\x98\x61\x20\x08\x82\x3a\x53\xd2\x44\x61\x0a\x8a\xbe"
	"\xaa\xa9\x92\x40\x55\x4b\x0b\x57\x36\x4a\x41\xc6\xb4\xc9\x44\x8a\x7a\xca\x9c\xf4\x60\x30\x04\xc0\xb2\xd5\x65\x55\x56\x5d\x96\x03"
	"\x93\x2a\x0d\x38\x04\xe8\x28\x5f\x15\x45\x91\x5a\x22\x65\x58\x45\x51\xb0\x14\x35\x85\xc2\x93\xa4\x71\x85\x82\x2f\x6b\x00\x00\x18"
	"\x8c\x0b\x86\x02\x75\x69\x59\xbe\xe0\xf3\x76\x1a\xa6\x11\x98\x46\xbf\xe7\x65\x5a\xd5\xbd\x06\x87\x7b\x0d\x4f\x53\x12\x45\xba\x2a"
	"\xcb\x05\xa1\x9b\x8e\x7b\x11\x66\xd0\x8d\x6e\x18\xa6\x9f\x6e\x68\x12\x9c\x86\x7b\x1a\xf3\x59\x42\x74\x55\x95\xe4\xc2\x72\x71\xf9"
	"\xce\x33\x36\xe5\x0b\xee\xbd\xa7\x1b\x0d\xf6\xc3\xf4\xc4\xa2\x79\xff\x10\x01\x4f\x37\x0d\xf3\x48\x17\x11\xf4\xd3\x54\x54\x44\x12"
	"\x24\x51\x50\x34\x97\x23\x4a\x81\x2c\x51\x00\x9a\x28\x5a\x3b\x57\x9e\x4c\xb4\x08\x05\x45\xff\x43\x7d\x40\xe1\x0c\xa3\x2a\x49\x3b"
	"\x15\x15\x94\x04\x65\x0a\xce\x30\xa9\x19\xad\xda\x1a\xc8\xe4\x4c\x8b\x48\x57\x96\x61\x4c\x49\x15\x15\x0f\x68\xa2\xb4\xf4\x09\x57"
	"\xae\x8a\x52\xa8\xa8\x68\x20\x25\xa9\x26\xe6\xca\x98\x46\x41\xd1\x54\x25\x65\x12\x56\x59\xd0\xd6\x31\xd3\x2a\x08\x52\x12\xae\x5c"
	"\x30\x14\x30\x4d\x8a\x70\x04\xf6\x50\xd1\xa8\xc6\x32\x0b\x8a\x94\x1d\xae\xdc\xe1\xa9\xa5\xc3\x08\x5b\x10\x64\x61\x9d\x2f\x92\xc4"
	"\x67\x94\x14\x61\x06\x91\x74\x2d\xc0\x5a\xc1\x92\xa6\x04\xaa\x89\x68\xba\x2a\x4b\x4b\x4e\x66\xb8\x9a\x26\xdd\xdd\x8c\x08\x18\x5c"
	"\xce\xae\x0c\xc9\x57\xb6\xb0\x24\x36\x4a\x01\x45\x8b\x64\x86\xce\x24\xda\x95\x61\x0c\x86\x42\x67\x09\xb6\xb0\x5c\xc1\x54\x53\x28"
	"\x09\xd3\x18\x8c\x05\x8b\x97\x10\x55\x24\x8c\xb1\x10\x91\xa6\x28\xc3\xa0\x09\x67\xb9\x92\xa0\x73\xa4\x4d\x12\x4a\x1a\x20\x1e\x39"
	"\xad\x73\x52\x69\x61\x9d\x12\xd2\x07\x23\x1a\x61\x66\xd3\x75\xa3\x65\x24\x3a\x1f\x61\x18\x8f\x30\x0c\x25\x55\x20\x86\x25\x7a\x2b"
	"\xa5\x32\xc2\x58\xa9\xa5\xf0\x4d\x6a\xff\xed\x43\xf9\x6c\x3f\x3d\x5f\xbe\xbd\x1e\xa1\xa4\x71\xda\x27\x1d\x6e\x2b\x9c\xb0\xd6\x6b"
	"\xa3\x3f\xbc\xf7\xc1\x48\x57\x1a\x7d\xd2\xbf\xd0\xde\x6b\x9f\xd7\x08\x69\x8d\xb4\x5a\x47\xe4\xe9\x0e\x92\x55\x0c\x14\x86\x55\x13"
	"\xa5\x0f\xe0\x64\xc2\x52\xa5\xa2\xba\xa9\x7a\x2c\x68\x60\xd8\x28\x05\x15\x92\x52\xe0\x0c\x33\x64\x94\x54\x59\xf1\x81\xaf\x5c\xc1"
	"\x50\x94\x1b\x48\xb0\xf5\x0e\x59\x58\xc3\xa6\x6d\x9a\xc5\x61\x30\xac\xba\x24\xfd\x64\x94\x25\xdd\xca\x60\x3b\xd4\x93\x0e\x59\xd2"
	"\xa1\x3e\x88\xb8\xff\x71\x4b\x97\x9b\x06\x6d\xfd\x51\x8a\x3c\x85\xc2\x99\x42\x03\xd5\xc4\x94\xa3\xaa\x52\xb9\xaa\x09\x46\xe1\x6b"
	"\x42\x22\x55\x30\xd5\x3d\x4f\xcb\x7f\xc8\x79\x07\xe2\xe1\x94\xc4\xe4\x8b\xd4\x4f\x43\x6c\x0f\xf8\x60\x36\x69\x58\xbe\x28\x28\x3a"
	"\xa3\xf2\x04\xde\x38\x6b\x95\x92\x4a\x48\xef\x84\x10\x4a\xf7\x77\xfb\xd1\x9d\xd2\x5a\x6f\x7c\xaf\x76\x56\x6a\x6d\xb4\x17\x7e\xbc"
	"\x57\xda\xfb\x0f\x72\x4c\x56\x45\x92\x6b\x62\x42\x6f\x4f\xa3\xb7\xa7\xf3\xa5\x61\x95\xc5\x65\x0b\x57\x32\x55\x65\x2d\x1b\xaf\xce"
	"\x7b\xbd\xbc\x43\xb5\x73\x48\xa1\x79\x3a\x52\x87\xb8\xb7\xf5\xd3\x15\x7c\x61\xab\xb7\x78\x41\x25\x8c\xef\xb4\x56\x5a\xaf\xc3\x7a"
	"\x2f\x6c\x47\x37\x8d\x43\x97\xf0\xef\x9c\x1f\x2b\x9d\xd2\x52\x59\x74\xd2\x7a\xe7\xfb\xad\x72\x56\x79\xef\xbd\x72\xc2\xe7\x4c\x37"
	"\x7a\xca\x1b\xef\xd3\x0a\xff\xbe\x77\xe9\x0f\xff\x23\x8d\x34\xde\x9f\xff\xf3\x2d\x8c\x1e\x21\x7c\x12\x34\xe7\x70\x8c\x55\xc1\x93"
	"\xa4\x80\x02\x32\x0c\x4f\xb8\x3e\x88\x50\x2e\x89\x6a\xe2\xc2\xdd\xe8\xe6\xf2\x20\xde\x23\x7c\x0b\xa5\x75\x9f\x54\x46\xb7\x96\xde"
	"\xea\x77\xba\xb5\x32\x4e\x6a\xe9\xa5\xe9\xba\x8e\xe9\x07\x81\x7b\x10\xdd\xe8\xb8\xd7\x30\x53\xde\xf2\xd4\x4e\x13\xb5\xd3\x39\x4d"
	"\x01\x1d\xf8\xba\x30\x14\xbd\x25\x41\xe5\x53\xf9\x54\xbe\xa5\x83\xd6\x3a\xb4\xf6\x1d\xfa\xb5\x95\x92\x1c\x5e\x5b\x15\x1e\xb2\x1f"
	"\x84\x04\x0a\xd9\x55\x75\x8e\x8c\x90\x0b\x43\xa1\xd1\x8d\xb6\x3c\xed\x34\xb4\x3e\x95\xf7\xe3\x7d\x86\x93\xe0\x84\x7b\x90\x07\x9a"
	"\x8e\x92\x5e\x83\x79\xba\x01\x86\x19\xc8\xa7\x0f\x88\x8d\xf3\xf4\x03\xdf\x8d\xde\x34\x87\x81\xc8\x70\xa5\x5f\x32\x54\x9e\x32\xbe"
	"\x3b\xb3\x01\x86\x2b\x32\x4e\x54\x51\x8e\x4c\xf8\xdf\x1d\xe7\x2f\x85\xb0\xd2\x4b\xff\xd2\x86\x36\x0d\x3a\x1f\xa2\x43\x0e\x0e\x10"
	"\xc8\x43\x3b\x10\x0e\xed\x40\x20\x8d\xd0\x0f\x0e\x0a\x0d\x82\xb4\xf1\xba\x53\x77\x78\xa5\x43\x7f\x90\xe9\x04\xac\x51\x39\xcb\x70"
	"\x2f\xf4\x45\x3b\x08\xdd\x5d\xb8\xc7\xb4\xa3\xd0\xe8\x04\xb8\x3c\xf1\x04\xd0\xdc\x8b\x58\x42\xe0\x27\x8d\x41\x02\x09\x1a\x4f\x43"
	"\x20\x71\x75\xd3\x24\x3c\x38\x10\x48\x64\x98\x14\xb5\x63\x81\xcc\x82\x12\xe3\xbf\xbb\x42\x05\xce\x70\xd4\x92\x21\x5a\xaa\x08\x5b"
	"\x60\x19\xc3\x91\x3e\xc8\x28\xe9\x71\xa7\x85\x36\x42\x19\x63\xb5\x17\x46\xeb\xf1\xc1\x06\x06\x48\x37\xba\x61\x4a\xc8\x74\x90\x90"
	"\xd8\xb1\x9d\x0b\x4f\x1a\xd4\xc0\x10\x81\xc0\x45\x42\x3f\x4d\xa5\x43\x4d\xa3\xa9\xf4\xf9\xf0\xad\x9c\xd6\xa3\xfd\x47\x2b\x15\x85"
	"\xf0\x34\x7c\xde\x08\xed\x68\x07\x07\x86\x87\x07\xc8\x03\x47\xa1\x34\x9a\xc9\x2e\x08\x38\xca\x63\x0b\x6b\xc2\x0e\x90\x76\x1e\xb2"
	"\x2a\x82\x60\x1c\x58\xb0\x00\x62\x80\x34\xc7\xc0\x49\x3b\x26\x2f\xb9\xe6\x38\x87\xac\x8a\x20\x0d\x24\xd2\x64\x21\x91\x80\x1a\x98"
	"\x09\xa4\x1d\x08\x26\xab\x66\x37\x3a\xa2\x9d\xa7\x25\xd9\x92\x98\x22\x52\x23\x22\x22\x22\x22\x22\x22\x22\x42\x92\x2d\x8c\xf1\xeb"
	"\x10\xed\x40\x1c\x4c\x26\xc9\xd3\x24\xa2\x63\xc3\xd1\x39\x47\x34\x89\x5f\xc7\xb0\x4c\x59\x50\xb4\xea\x82\x4c\xba\x07\x27\xdd\x98"
	"\x0d\xc7\x84\x25\x18\x8d\x3a\x19\xa3\x26\x82\x11\xce\xa6\x1c\xbd\x84\x52\xfa\x8c\xd1\x29\x7d\x6f\xa9\xbd\xb1\xfe\x7d\x28\xe3\xf5"
	"\x19\xc9\x39\xad\xc3\x4b\x6d\x94\x50\xc6\x08\x65\x74\x3a\x9f\xbe\x18\xa3\xaf\xd7\xeb\xbb\xfc\x68\xff\xe7\x83\x1f\x45\x59\x69\x9c"
	"\x17\xde\x4b\x5f\x33\xde\x19\x2f\x95\x96\xee\xde\xed\x51\xd2\x7c\x96\xe7\x5c\x3a\x46\x52\x52\x8c\x0e\x65\x94\x64\x61\xa8\xa5\xd6"
	"\x71\x20\xbd\x88\xd5\x46\xbf\x11\x6e\x51\x92\x21\x12\xd5\x04\x74\x06\x28\x4d\x9a\x52\x06\x6d\x37\xde\x38\xeb\xa5\xb3\xce\x7a\xe9"
	"\xac\x97\x4a\xa7\xd2\xe9\x8d\x7f\xe3\x8d\x37\xde\x28\xaf\xcb\xeb\xd1\xa3\x47\x77\xe8\xd0\xe1\x8d\x7f\xe3\x3b\x75\xea\xd4\x29\x95"
	"\x54\x52\x49\xe5\x74\x2a\x9c\xd3\xa7\xfb\xac\x70\x3e\xda\x41\xe5\xe9\x99\x7e\x1a\xdf\x0b\x8c\x45\x46\xa1\xf2\x94\x27\x4a\x82\x7e"
	"\x6d\xa5\x8f\x69\x65\xfc\xcb\x73\x09\xbd\xc2\x5b\xa1\xb5\x97\x3e\x38\xfd\x41\x77\xb4\x03\x91\xd0\x4d\xf7\xa6\xd7\x68\x69\x95\xb0"
	"\xbe\xc8\x20\xf4\xe3\x2e\x74\x85\xe6\x14\xf7\xe9\x7d\x3a\x3e\x85\x7a\xaa\xaa\x88\xe3\x62\x69\xf4\xc3\xd9\xb4\x4d\x7b\x34\xfa\x46"
	"\xa7\x58\x2c\x32\xeb\xe9\x48\x27\x44\xf9\x48\x71\x77\x87\xd2\x72\xd3\x36\x6d\xd3\x18\xd8\xdc\x21\xc5\xa7\x9d\x76\xda\x69\x6d\xb4"
	"\xd1\x46\x1b\xa7\x9d\xd6\xa7\xbd\xe8\xa6\x4f\xef\xa6\x3f\x08\x99\x63\x3e\x14\x84\x13\xba\x3c\xca\xd1\x74\x4d\x83\x45\x26\x93\x8e"
	"\x93\x3e\xf8\x09\x5f\x51\xb6\xc0\x99\x64\x81\x1c\x55\x92\x1c\x5d\xc7\xa9\xbc\x54\x4e\x2a\xa9\x6c\xe4\xfb\x76\xa6\xad\xb7\x56\xf7"
	"\x09\xa9\xa5\x1f\xfd\x27\xbc\xf5\x69\x94\xf6\x65\xfd\x4a\x6d\x95\x97\x99\xd2\xef\x95\x16\x4e\x38\xa1\x8d\xf7\xc6\x5a\xe1\x83\xf1"
	"\xa1\xac\x6f\xbf\xbe\x7c\xa8\x26\x22\x94\x0b\xeb\xf7\xf9\x97\xc4\xd0\xe1\x6b\xd3\x5a\x3a\x64\x61\x12\x43\x35\x61\x91\x59\x39\xb2"
	"\xa4\x49\xd2\x92\x7b\x50\x42\xef\x60\xb8\x87\x55\x16\x83\x55\x16\x56\x59\xfd\x50\x42\x7d\xf0\xf1\x1d\x8a\x1e\xb0\xa4\x69\xd2\x96"
	"\x8e\xfe\xa5\x7f\xa9\xfc\x8e\xa3\xf2\xa9\x74\x6c\xac\x54\xbe\xac\x36\xd6\x4a\xa7\x8c\xef\xd0\xc3\x7f\xf8\x0d\xd6\x0e\xc4\xfc\xfd"
	"\xdd\xfd\x2f\x4c\x44\x75\xdc\x73\x9e\x86\x53\x3b\x17\xee\x59\xd0\x1d\x9a\xda\xd1\xdd\x04\x27\x9f\x6a\x37\x31\x57\x55\x4d\x93\xb5"
	"\xa6\x03\xba\x86\x2f\xa8\x23\x28\xcb\x1c\x65\x4c\xcd\x80\x08\x00\x01\x40\x00\xd3\x12\x60\x60\x48\x5c\x2e\x94\x09\x04\xf2\x2c\x4c"
	"\x77\x13\xc0\x87\xa5\x02\x61\x2c\x18\x8a\x85\x83\x31\x0e\x23\x21\x0a\xa2\x28\x04\x19\x03\x00\x31\x00\x10\x63\x0c\x31\x08\x11\x8d"
	"\x10\x11\x00\x7a\x48\xfe\xa8\x44\x52\xb5\x51\x8b\xbb\x18\x4b\x1c\xcb\x5c\x7c\x48\xce\x1a\xd9\x31\xb1\x1f\xf4\x35\x95\x29\xf6\x77"
	"\xd1\xdc\x1f\x2a\xaa\x87\xb6\x56\xb4\x50\x8b\x7f\x9c\x0f\x6e\x6e\xfd\x16\xa2\x60\x15\xaf\xd7\x25\x21\x2f\x32\xb1\xc0\x88\x68\xa6"
	"\x20\x1a\xe0\xf8\xdb\xa6\xc5\x65\x79\x21\x72\xb9\xbc\x67\x72\x25\x9c\x74\x0f\xf9\x89\x24\x9d\x19\xbb\xc4\x11\x2b\x44\x5f\xd4\x95"
	"\x6e\xc7\xdf\xc6\x2c\x36\x01\x3b\xc9\xa3\x4b\x11\x3b\xc9\xe0\x54\x53\xe1\x28\xbe\x5b\xcc\x1d\x41\xa9\x93\xd4\x1c\xf7\xcb\xaf\x9b"
	"\xe3\x1a\xb0\x2e\xf5\xc8\x10\xf9\xd2\x45\xbe\x3e\x47\x05\x53\x16\xd0\xd1\x7e\xa8\x56\xc5\x00\x1f\x22\xc4\x43\x0c\xbd\xd3\xfc\xec"
	"\x26\xf7\xda\x62\x99\x2a\x24\xf4\x8b\x5a\xdd\xe0\x8d\x8f\x87\x9c\x4d\x8f\x54\xdb\xf1\xbd\xa7\x23\xc4\x58\x79\xde\x39\xc0\x68\x56"
	"\x6b\xc2\x54\xe7\xc9\x19\x9c\x12\x31\xd0\x74\xc5\xd2\x82\xe9\x04\x3d\x25\xf8\x4c\xb4\x7b\xa9\xc1\x93\xbb\x7d\x4c\x5a\xc1\x47\x32"
	"\xd6\x3b\xcb\x41\x27\x2d\x7f\x58\x88\x40\x1a\xd3\xa3\x28\x4d\x99\x0b\x99\x8e\xd4\x48\xb4\xdc\xa2\x35\x8f\x4b\x41\x6a\x96\x71\x31"
	"\xe9\xdd\x87\xc2\x38\x29\xd5\xc2\x53\xde\x16\x10\xfc\x72\x93\x31\xd4\xe5\x99\xf4\xcb\xb5\x93\xef\x17\xc9\xe5\xa0\x85\x46\x00\x6b"
	"\x14\x82\x1e\x20\x16\xa8\x02\xd5\x01\xd2\x56\xd9\x09\xb2\x7c\x3c\x63\x11\x7b\x0a\xee\x59\x72\x96\xa1\x9c\x53\xff\x91\x68\x91\x23"
	"\x29\x9f\xb9\x91\xae\xc5\x7b\xa4\x4f\xab\x6a\x93\x44\x87\x26\xa5\x85\x47\x04\x28\x31\x6e\xa5\x90\x67\x08\x7c\x53\xdd\x27\x7c\x86"
	"\x33\x59\x5c\x33\x82\xd8\x5a\xf3\x51\xa8\x39\x90\xe9\xf4\x68\xbf\xc6\xa7\x56\xaa\xc8\xa2\x43\xad\x13\xb0\xf9\xd1\xb8\xfd\xb0\x14"
	"\xf9\x6f\x14\x78\x24\x00\x33\xab\x75\x6c\x14\x36\xaf\x0f\xc9\xb3\x5a\xe6\x4f\x6f\xa0\x8f\x90\x9a\x54\x68\x02\xda\xbc\x85\x44\x93"
	"\xb6\xf8\x15\x6b\x13\x0a\x91\x49\x16\x7f\xd2\x7b\xf2\x2a\x5a\x36\x5f\x5c\xaf\x38\xff\x4d\x57\x90\xdb\xef\x65\x86\x73\xa5\x93\x96"
	"\x6f\x35\x35\xad\x5e\x39\x82\xe6\x9a\x31\x75\x1f\x04\x6e\x07\x56\x9c\x28\x69\xba\x02\xbc\x9d\x59\x53\x27\x8a\x91\x62\x69\x53\x40"
	"\x96\x20\xf4\x27\xbc\xa3\x55\xd2\xc4\xd7\x08\x02\x65\x91\x75\x11\x80\xb2\xb3\x29\x32\x68\x69\x7f\x8d\x80\x4d\xcc\x81\x62\xc3\x78"
	"\x5c\x9b\xa3\x76\xde\x70\xea\xf1\x29\x73\x44\x95\x8a\x89\xf1\xde\x69\x66\x63\xe6\x37\x7b\x9b\xbf\x10\x12\x43\x44\xcc\x60\x56\xc5"
	"\x06\x6c\x2b\x38\x79\xc4\x3e\x12\x26\x11\x65\x4e\x22\xef\xfe\x1d\x88\x30\xe8\x54\xae\x95\x47\x0b\xc1\xd4\x20\x30\xb1\x19\x31\x3c"
	"\x20\x2b\xc8\x13\xa7\x62\xc5\x9d\x8f\xfa\xd2\x78\xa4\x85\x4b\x2a\x0d\x4d\x52\x39\x9a\x9c\xbe\x09\x9b\xcb\xcb\x2b\x1f\xb9\xa4\x10"
	"\x73\xb2\x27\x55\x00\x07\xcf\xe5\x78\xd1\x53\x77\x41\x12\x19\x59\x99\x84\x17\xe2\x1e\x05\x2b\x22\x6c\x95\x50\xae\xa3\x19\x89\x6c"
	"\xa7\x9e\xc0\x75\x62\x8e\x34\x94\x51\x61\x26\x03\x88\x66\xf2\xa7\xe6\xb0\x19\x6c\x59\xdf\x38\x72\xc3\x5d\x40\xe9\xbc\x5c\xce\x14"
	"\x5a\x00\x8d\x6e\xc2\x0f\xbb\xed\x4e\xef\x82\xa3\x20\x3d\x4e\x6e\x3b\xb3\x58\x83\xff\x2a\xec\xa1\x06\xe6\x98\xbd\xb4\x00\x1c\x27"
	"\x49\xa8\xa0\xef\x9e\xcf\x4d\x2a\xdf\x71\x0f\x16\xfb\x34\xbb\xbe\xd9\x47\x43\xb7\x4e\xf3\x6b\xcc\xff\x45\x70\xe0\xc6\x71\x04\x32"
	"\x31\x9f\x5a\x78\x05\x0d\x70\x0f\x60\x2b\x89\x0c\x71\xfb\xaf\x72\xb7\x8a\x3b\x39\x40\xb3\x8d\x1b\x72\xc0\xa7\x4e\xf4\xbb\x19\xf1"
	"\x63\x05\xf5\xeb\x37\xd9\x27\xcc\xa1\x04\xa1\x28\xd0\x4b\xf2\xf7\xb2\x82\x11\xb1\xb7\xad\xdd\x67\xb0\x72\x8b\xbc\x3c\x7d\x34\xcb"
	"\x38\xd4\x9e\xe7\xb1\xb3\x6a\x62\xc8\x5a\x2e\x66\xed\xce\xb3\x06\xd9\xe2\xbd\x6f\x4c\x7b\x5a\x84\x63\xfa\x9e\x74\xe8\xc0\xf7\x65"
	"\x76\x3e\x42\x59\x09\x41\x4c\xfa\x2d\xaf\x91\x37\xc5\x47\x0f\xaa\xd9\xcb\x65\xde\x2a\xff\xbc\x7c\x90\xdf\x54\xda\x25\xe9\x92\x3c"
	"\x70\x63\x04\xe3\xfc\x56\x9e\x08\xa1\xc9\xbf\xf1\xf1\x47\xce\xde\x20\xe2\x0c\x84\xc0\x01\xbe\xff\x13\xe9\xf2\x3b\xb3\x6c\x19\x8b"
	"\x8b\xc4\x12\x90\x31\x90\xe4\x9e\x50\xa6\xa0\xcf\xcc\x11\xf9\x4c\x3e\xd0\x6c\x7f\x8c\x62\x71\x02\xa4\xdf\x5c\x61\x95\x1b\x33\x17"
	"\xf9\x91\xe4\x65\xc2\x08\x11\xaa\x40\xab\xd9\x21\x1f\xfb\x22\xb3\x69\x41\x27\xbe\xc4\x67\x23\xf6\xc3\xa4\xe3\x04\x77\x4b\x42\x26"
	"\x51\x0c\x05\xf4\x67\x98\xf3\x59\x8a\x31\xa9\x45\x45\x76\x4f\x56\x45\xcb\x6e\xc6\xc5\x39\x58\x3e\x76\x54\xb9\xc3\x6f\xa8\x81\x69"
	"\xec\x84\xb5\x7b\x0a\xf5\xb8\x60\xcd\xd2\x59\x0e\xca\x55\x72\xbc\xc8\xa8\x4b\x06\x0e\x45\xa8\x2e\x32\x10\x38\xae\x47\x64\xd5\xf6"
	"\x27\x04\x3e\x91\x4d\x30\x72\x59\x73\x74\x5e\x15\x54\x29\x7d\xd4\xee\xba\x96\xad\x9f\xfa\x8c\xea\xba\x32\xa1\x15\xc9\xc2\x1a\xf0"
	"\xd9\x07\x80\x4b\x9f\x70\xc1\x45\xbb\xa4\xd1\x5e\x8a\x18\x25\xc3\x13\x1c\x8b\xe5\x9d\xcc\x57\x9f\x0b\x72\x8d\x09\xda\xe4\x4e\xf5"
	"\x05\x93\x0b\xeb\x61\xd2\x51\x6d\x8b\xd6\x58\x8b\x71\x92\x61\x5b\x1f\x38\x10\x08\xba\x97\x73\xbd\xa3\x25\x5d\xcf\x1c\x4e\xc8\xb3"
	"\x24\x96\xa0\xa9\x2d\x47\x30\x29\xbd\x4d\x31\x90\x14\x52\xeb\x4e\x5c\x8e\x68\x39\x21\x47\x6c\x4d\x47\xb1\x0e\xfd\x0a\xb2\x29\xc4"
	"\xb2\x4c\xe3\x26\x96\xa2\x63\x03\x05\x76\x8e\x25\xa9\x89\x97\x93\xfc\x38\x52\x8a\x36\x79\xf8\xb5\xcd\x33\xcf\xb1\x93\xda\xb4\xb4"
	"\xe3\x7b\xb9\x15\x74\x36\xf3\x2c\xa0\x73\xab\x80\x79\x0b\x09\x74\x08\xbd\x80\x2e\xf6\xfb\xb2\x93\x8b\x10\x33\x06\x34\x88\x74\x0a"
	"\xcf\x8d\x4b\x30\x62\xee\xfe\xe9\x80\xa2\xde\x22\xab\x6a\x58\x26\x4e\x1b\x42\x5d\x43\x6d\xea\xf1\xb4\x35\x7e\x15\xb2\x53\x68\x33"
	"\x26\x7f\x9e\x6c\xc3\xbc\x1f\xbc\xc1\xc2\xf1\xf4\x12\x36\x2b\xc8\xdf\x82\x87\xb2\x88\x8c\x90\x86\x25\x0b\x21\x41\x8b\xf0\xc5\x4d"
	"\x9f\xdc\x46\xc7\x75\x21\x4d\x42\x97\x60\x70\xdf\x4b\x51\x00\xcb\x8a\xb5\xb0\x28\xeb\x13\x2a\xdc\xa4\x47\x97\x96\xaa\xdf\x9e\x06"
	"\x68\x94\xf8\x3e\x74\x1d\xe3\x5f\x4f\x84\x5d\x0b\xc2\x1e\xcd\xbe\x20\x0d\x74\x15\x65\x00\x4f\xc3\xd9\x66\x43\x8f\xe7\x90\x52\x0d"
	"\x06\xae\x35\x1a\x49\xa0\x09\x6d\x2d\x51\xcc\xa3\x23\x93\xcd\x73\x8e\xbc\xae\xaa\xd9\x33\xc8\x16\x03\x85\x06\x64\x0d\x2e\x33\xe0"
	"\x30\x5b\xed\x15\x56\x8b\xb2\x38\x89\x0b\xe3\x83\x02\xad\x92\x58\x23\x65\x1b\xac\x3d\xa5\x59\xb9\x4a\x7f\xb1\x5e\xa8\x9c\x3e\xef"
	"\xdc\x8b\xf3\xe4\x2e\x46\x5c\x2a\xb7\xa4\x1d\x5f\x38\x9b\xad\x3f\x32\x69\x95\x45\x20\x02\x36\xad\xd5\x3a\xc1\x09\xab\xd4\x45\xc4"
	"\xb5\x1a\xaa\x27\x71\xf1\x5e\x94\x96\xdf\xd2\xfe\xe8\xd4\x44\xd6\x8e\xe1\xa8\x56\xa9\x78\x70\x6c\x78\x73\x1b\x2c\x7b\xa0\x0a\x99"
	"\x7f\x74\x98\xd9\x13\x86\x6b\xc5\xd2\xd9\x5f\xc6\x4c\xc0\x45\x86\x07\x58\x30\xfb\x0a\x79\x51\xbd\xb8\xb9\xf8\xc9\xa0\x71\xd6\xe1"
	"\x2c\xb0\xa3\xb4\x3b\x60\x2b\x53\x2b\xda\xcc\xe5\x70\x4d\x7c\x7b\x9b\x47\x13\xa6\x5b\xc2\xe6\x7b\xdf\x56\x92\x9c\x84\x85\x28\xbe"
	"\x66\x68\x34\xe2\x22\xb3\xe6\xc6\x09\xef\x0f\x00\xbd\x47\x20\xe1\x46\xa9\x3d\xf1\xc9\x07\xba\x7c\x80\xa1\xef\x94\x44\x2b\xe0\x08"
	"\x52\xb0\x07\x52\xd1\xf6\xeb\x41\x13\x3f\x3d\xbf\x66\x8e\x47\x98\x7b\x36\xba\xd8\x06\x03\xdb\x65\x57\x94\x18\xbb\xaf\x7c\x94\x3e"
	"\xcf\x1c\xe9\x89\xe5\x0c\xf6\xbd\x68\xfa\x63\x20\xf2\xe8\xd6\xa0\x02\x94\x56\x5c\x73\xac\x66\x39\x5d\xf9\x4b\x19\x18\x28\xe0\xbe"
	"\x6a\xf1\xbb\xf7\xc4\x84\xc8\x06\xb5\xbe\x76\xa6\xb0\x56\xd7\x22\x46\x82\x55\xee\xc7\x2a\xc3\xdd\x3f\xb5\x68\xc0\xde\x49\xe0\xcc"
	"\x10\x6d\xca\x65\x64\xf5\x48\x2c\x3b\x42\x08\x06\x7c\xc5\xd8\x37\x8e\x7b\x82\x84\xfe\x29\xe5\xe7\x9c\x57\x17\xe6\xa7\x35\xbc\x98"
	"\x83\x15\xeb\x68\x71\x90\xd2\xc9\x43\xdc\x31\x7d\x30\x06\xfa\x67\xd7\xeb\xd7\x95\x5b\x2b\x5d\x5e\x13\xd9\xad\xcf\x07\xad\xe1\x02"
	"\x74\xd3\x4a\xf6\x73\x24\xa7\x8e\x91\xa4\x5b\x91\x00\x39\x01\x52\x88\xf7\x46\xf8\x62\x58\x0b\xb1\x8a\x83\x67\xc5\x28\x0d\xbd\x65"
	"\xb8\xb3\x77\xd6\x8a\x4c\xf1\x7b\x88\xc8\xbb\x80\x4d\x28\xc7\x33\x0d\x98\x86\x15\xaa\x47\xd7\x0e\x68\x90\x17\x3b\x33\x5e\xc1\x2b"
	"\xa7\x2e\x50\x6d\x9d\x68\x1e\x8e\x51\xd5\x68\xd6\xbf\x8e\xc0\x16\x0e\x51\x17\x45\xad\x34\x49\x1d\x9b\x92\xac\x0d\x9f\x58\x0a\xcb"
	"\xe6\xb9\xd7\xd8\xbc\xc6\xbb\x91\x5b\xa6\xa2\xa7\x48\xab\x5c\xc3\x48\x71\x92\xd9\xae\xab\x8a\x0b\xfc\x15\x5b\xe5\x5c\x26\x1f\x91"
	"\x82\x00\xcd\xed\xd1\x45\xb9\xc3\xcc\x85\x66\xf5\x65\xb3\x39\x6d\x3d\xc5\x94\x4a\x28\x6b\xbd\x9f\x94\x42\xe1\xff\xfb\x82\x4e\xfd"
	"\x80\x82\xf3\xbf\x43\x9d\x54\x0f\xd0\xf0\x2a\x8e\x01\x53\xd9\x52\x09\xcc\x30\xc5\x7c\x88\x75\x62\x62\xc0\xcf\x50\x85\x1c\x34\xaa"
	"\x22\x78\xbb\xca\x8c\x7d\xc4\xad\x85\x28\x3a\x05\xeb\x44\x07\x44\x50\xe3\xa3\x68\x1c\xe6\x08\xb3\x12\x16\xd5\xfe\x70\xac\x93\x14"
	"\xc7\xde\xad\x98\x58\x5b\x16\x16\x91\x26\x2c\xe3\xbc\x55\x6f\x83\x5d\x37\x0e\x24\x08\xc3\xef\x84\xd9\x20\x14\x2f\x0a\xc0\x61\x32"
	"\xe1\x1e\x78\x15\x4b\x66\x1c\x11\xe4\xc5\xf0\xbf\xa2\x46\x48\xc0\x8e\x0f\xa8\x74\xd3\x2c\x00\x34\x15\xc2\x42\x62\x27\x7a\xcf\x45"
	"\x7e\x1f\xb2\xaf\x58\x40\x8b\x52\x00\x2d\x48\x22\x80\xad\xe7\xb4\x5a\xbd\x33\xcf\xe8\x3d\x1e\x9c\xd9\x7f\x00\xc8\x92\xca\x9a\x05"
	"\x97\xf6\xf3\xde\xd2\x2e\x7b\x18\x6c\x2f\x13\xbd\xec\x71\xec\x73\xeb\x61\x23\xc2\x80\x25\xa7\x86\x48\x2a\xf6\xe1\x93\xc6\x93\x9b"
	"\xb3\xdd\xb4\x73\xe7\x97\x47\xf8\x67\xad\x61\x1a\x48\xd8\xba\x31\xd7\x9b\xdd\xf6\xbc\x82\x9d\xbf\x9a\xcb\xeb\xc9\x26\x0d\x42\x13"
	"\x0f\x36\x9b\x90\x18\x9a\xc6\x14\xd2\xe7\x0e\x17\x37\xcd\xbe\xfe\x82\xed\x5d\x69\x73\x4e\xbd\xc6\x0d\xae\x84\x69\xe9\x70\x95\x06"
	"\x7a\x49\x69\x16\x4e\xf3\x48\x56\x52\x8e\x34\x3d\xb6\x1d\xc3\x6a\xe4\x6a\x76\xe0\xb0\x9b\xfa\x7f\xa9\x8a\x85\xb2\x18\x08\xf2\x78"
	"\xcf\x9b\x40\x5a\x5e\xac\x68\x51\x24\x98\xad\xaa\x57\x4f\x2d\xac\x17\xd8\x11\xc3\x84\xb6\x4f\xfd\x0f\x18\x1d\xad\x35\xd8\x50\xb0"
	"\xe5\x78\xe0\x90\x8c\x6d\x65\x50\xc9\xe4\xac\x13\xb5\xae\x5b\xeb\x8c\x68\xf0\x20\xbe\x31\xf2\xaa\x5f\xbc\xee\x87\xaf\xb4\x1d\xb3"
	"\x7a\x61\xb4\xf7\xaa\x5b\x66\x17\xeb\x3c\x40\x47\x98\x17\x63\xc0\x16\xec\xa3\xeb\x03\x07\xd4\x30\x8f\xea\xaa\xbe\x1e\x4c\x06\xb2"
	"\x08\xb9\xc2\x45\xd6\x76\xd6\x93\x14\xf5\x23\xc6\x86\xee\x6f\x26\x3b\xd8\xcd\x49\x25\xb5\x26\x6c\x38\xbb\x5c\xa9\xa3\xaa\xc1\xcf"
	"\x90\x86\xd8\x67\x0f\x8a\xc7\x87\x0d\x37\xe8\x6e\x07\x9f\x6e\xfe\x4e\x86\x84\x8b\x56\x25\x12\xc5\xb3\x4c\x44\x5c\x3a\x5a\x10\xd9"
	"\x9b\xae\x8a\xf5\xd4\x0b\x1c\x71\x2d\xe1\x17\x6a\x0e\x92\xa4\xfb\x0d\x4a\x2d\x1e\x19\xb4\x46\xd1\x10\xfb\x63\xe9\x62\x3e\x4c\xf9"
	"\xfd\xe0\x6e\xbe\xed\x4d\xff\x89\xc3\x46\xb1\xf7\xa6\xf2\xdd\x3b\x81\xa8\x33\x42\x13\x47\xe3\xd6\x28\x1f\x5b\xf1\x17\x93\xe8\xca"
	"\x90\xcd\x45\xb3\x10\x4a\xd7\xa8\x0a\xe6\xba\x9e\x6c\x8c\xcc\xfb\x9a\x47\x37\xda\xe1\x33\x30\xb1\x80\x6c\x65\x33\x43\xe3\xfc\x80"
	"\x73\xaf\x75\xb8\xcc\x47\x41\xc8\x8c\xee\x7e\xb9\x43\x6f\x9b\xd1\x58\xb7\x45\x7b\x5e\xd7\x3c\xed\x1d\x66\x78\x52\x35\xeb\x67\xe8"
	"\xd2\xa0\xdd\x1a\x2f\x54\x4e\x27\xd0\x95\xfc\x4c\x10\x60\x91\x19\x8a\xa5\x5d\x3a\x5d\x6e\xa7\xc7\x03\x7b\x74\x02\x10\x61\xd8\x22"
	"\x65\xe1\x63\x96\xa8\x11\x19\xc2\x94\xf9\x9d\x0f\x66\xd8\x44\x02\x56\x06\x6d\x8c\x4c\x91\x5d\x45\x09\x4a\xb1\x30\x09\xcb\xe6\xa4"
	"\xee\x7a\x8e\x40\xe9\x80\xfa\x50\x4f\x88\x0f\xc7\xb2\x84\x94\x05\xae\x6a\x20\xc7\x3d\xdc\x8b\x50\x3b\xa0\xaf\x2e\x03\x33\x47\x9f"
	"\xf4\x66\x34\xa9\x46\xc4\xda\x8b\x28\xf9\xcc\xb3\xdb\x66\x34\xc9\xc4\x17\x44\x04\x02\x3d\xf8\xcc\x59\x57\x21\x82\x3d\xa4\xcd\xc3"
	"\x21\x56\x8e\xc6\x42\xda\x59\x6d\x64\x7e\x20\xef\xc8\xc3\x95\x2e\x6e\x3b\x23\x1d\xe0\x77\x33\xce\xdd\x22\x0e\x20\x75\xed\x86\x59"
	"\x76\xcb\xaf\x62\xea\xca\xe1\x56\x76\xa2\x15\x40\xdc\x72\xcc\xd4\x05\x51\x85\xc1\x7c\x95\xb1\xb4\x32\x60\x8c\x13\xc4\x5b\x3f\xbf"
	"\x5c\x5d\x6f\xe2\xa4\x6c\xd9\xab\x61\x9e\xf3\x69\x1d\xc5\x51\x16\x4e\x4c\x08\xf3\x60\x6b\x2c\x0c\x93\xaa\x9d\x42\xf4\x69\x4a\x73"
	"\x48\x8c\x5b\x46\x21\xbc\x07\xea\x4b\x55\x08\xeb\x28\xb1\x4d\x9d\xa6\x8b\x6c\xf1\x22\x42\x94\xfd\x4c\x9f\x97\x78\x19\x00\xe6\xe8"
	"\x80\x8a\x61\x3a\xee\x4a\xe5\x0e\x62\x51\x8f\x57\xdf\x5f\xe7\xad\x1e\xee\x46\x13\xc0\x6f\x2f\x74\xab\x89\x4a\xfa\x71\x1b\xde\xdf"
	"\x2b\x76\x2d\x5e\xae\x7f\x2e\x87\x89\x21\xee\x77\xa0\xd0\xf6\x0e\xcc\xb0\x43\xb2\xf0\xf0\xa5\xdc\x88\xfe\x34\x6b\xed\x5d\xb8\x67"
	"\x3a\x00\xa0\xdc\x7f\x8d\x5d\x99\x2b\x7f\xb5\xa2\x51\x64\x36\x38\x27\x76\x08\x5b\x12\x58\xc9\xaa\x0c\x28\x60\x46\x4b\x2a\xf8\x88"
	"\x8b\xa9\xec\x2d\x35\x4c\x43\x52\xa7\xf9\x66\x32\xc1\xf3\x2a\xa6\xe1\xe5\xe1\x4c\x3b\x3e\xd3\x1d\x7a\x59\x95\x8b\xd7\xff\x87\xcc"
	"\x28\x89\x1b\x67\x56\x5c\xb7\xab\xd0\x40\x84\x37\x24\x07\x5b\x97\x4b\x51\x1c\x32\x2e\x79\x38\xab\xd0\x60\x29\x9c\x03\xb0\xcf\xa0"
	"\x28\x22\xf9\x11\x02\x81\xa9\x86\x9b\x95\x62\xc3\x4c\x0a\x84\xd7\x63\x5f\xb8\x5b\x06\xc3\xf5\xac\x65\x00\x11\xa6\xd9\x8a\x94\x99"
	"\x93\x90\xba\xdf\x58\xb0\x2e\x42\x2c\x97\x79\x6d\x9f\xdb\x43\x9c\x9f\x3b\x7b\x11\xc6\x06\x2d\xd7\xe7\x70\x6e\xe8\xbc\x58\x8b\x8d"
	"\xca\x94\x82\xda\xb5\x6c\x90\x96\x36\x7a\xb0\xe8\xe2\x80\xa5\xce\xa7\x7b\x25\x2b\xe2\xb6\xc4\xd5\x5e\x3f\xf8\x94\x22\x28\x22\x08"
	"\x40\x84\xd4\x3a\xaa\x88\x58\x30\x36\xf9\x34\xb2\x52\xfe\x67\x58\x09\x00\xac\x64\xfe\x8a\x38\xe1\x95\xb7\x2a\x2a\xaa\xba\x2e\xef"
	"\xa3\xcd\x9b\x42\x8a\x6c\x29\x5c\x32\x8a\x47\x34\x30\xac\x36\xfc\x35\x65\x94\x09\x92\x99\x55\x8b\x7c\x8e\x64\xd1\x54\x4e\x98\x7a"
	"\xc4\xba\x52\x22\x7b\xae\xbb\xa6\x6a\xf4\xb2\x6b\x8f\x2f\x5a\x32\x41\x11\x25\xe1\x64\xe1\xd6\x56\x3a\xb9\x65\x03\x44\x46\xd3\x7a"
	"\x48\x54\x59\x71\x9d\x7a\xa9\xc0\xe5\x52\x40\xc0\xb2\xa3\x75\x7d\xa2\xb9\xf9\xea\xea\x4d\xc7\xd9\xaf\xfc\xd1\xe5\x10\x0a\x45\x81"
	"\x82\x26\x84\x9a\x81\x88\x13\x71\x03\x25\x1b\x03\x14\xf1\x10\xfa\x13\x90\x06\x32\x32\x11\x99\xf5\x6d\x00\xd9\x4e\xb5\x72\xc3\x65"
	"\x32\x98\xa4\x24\xdb\x0f\xc4\x51\x8d\x5a\xb6\x55\x14\x89\xd6\x2c\x93\x40\xa9\x24\x04\xa9\x85\xb3\x97\x81\xd2\x1d\x7d\x2e\xbb\x39"
	"\x73\x07\x79\xfc\x3e\x1f\x95\xe9\x8b\xbc\xb8\xce\xbd\x5f\x34\x8d\x2d\xd6\x45\x2d\x59\x7a\x23\xfc\x7f\x7f\x75\xe5\x21\x2b\x24\x35"
	"\x30\xa9\x83\x3f\xbd\x98\x1b\x74\x1c\xe5\x02\xd1\xcb\xe4\x97\xa9\xa7\xf8\x40\x1f\x8f\x7a\x01\xc1\x90\xc1\xfd\x18\xe7\x3a\xb2\x4d"
	"\xeb\x0e\x5c\x8c\x04\xc6\xc4\x11\x0c\xae\xdb\x59\xe6\x57\xa6\xab\x6c\x2b\x4b\xd9\xab\x63\x2f\x5f\xf5\x81\xc8\x57\x36\x05\x90\x8e"
	"\x36\x63\xf0\xd2\x3a\xa6\xd2\x18\xde\x6b\xf5\x82\x28\xea\x6f\x7a\x08\x0c\x05\x88\xb9\xb5\x94\x94\xb4\xa5\x09\x63\xd5\x42\xec\xa6"
	"\x41\xe6\x96\xe9\x6b\x8b\x8d\xfc\xb3\x66\x67\xa0\x16\x68\xbb\xf9\x84\xbf\x8b\xc7\x71\x4c\x18\xbc\xf7\xe6\xe2\xbd\x1a\x89\xc6\x41"
	"\x51\xad\xd7\x96\x20\x5f\xbd\xef\xf6\xa9\xa3\x5d\x10\xc2\x8d\xf2\x94\xa6\x77\x40\x5f\xfa\x71\xe8\xda\x2d\x03\xe2\xbf\x24\x08\xd5"
	"\x03\x68\x77\x25\xe6\x53\xfc\xb1\x5b\x73\x54\xa2\x7d\x2b\x21\x2e\x5b\x7e\xce\xb6\x70\x41\x18\xdb\x28\xc5\x05\xdb\x03\x5a\xc8\x46"
	"\x31\x01\x63\x44\xe3\xbe\x0b\x1b\x4b\xf7\x2c\xef\x20\xb5\x32\x90\x5c\xf1\xbf\xfe\x2a\xc6\x27\x20\xaf\xd5\xc4\xb2\x59\x16\xa2\xfa"
	"\xfc\x23\x04\x60\x59\x26\x50\xe7\xe1\x16\x75\xa4\x0d\xb2\x5c\xaf\x4a\xe1\x6d\x5b\x02\xfa\xc6\x3d\xb7\xa8\x91\x5e\xf5\x87\xfd\x3b"
	"\xd3\xa8\x32\x8e\x56\x7b\xb0\xad\x2e\x65\x9a\x11\x33\xb3\x8d\xdc\x9b\x81\xa0\x77\xf3\x78\xff\x07\xb5\x70\x4e\xec\x32\x11\x10\x18"
	"\xef\x43\xd9\x66\x9e\xbb\x61\xb8\x8e\x2e\x87\xea\x2a\x78\x02\x50\x5f\x3e\xd4\x51\x09\x54\xf9\x45\x7b\xd9\x8a\xaf\x2b\x3f\xb3\xc1"
	"\xd1\xa6\x33\x4e\xe4\x6e\xa5\x56\xb1\x5f\x57\xbc\x8a\xca\x2e\x48\x7f\x53\xfd\x6c\xb6\xd5\x4c\xc2\x8b\xaa\x44\x3c\x96\x96\x5d\x3d"
	"\x67\x7a\x8b\x5a\xe4\xc9\x00\x1f\x0a\x89\xa4\x52\xba\x20\xea\x3a\xb6\x4d\xd2\x09\xbc\x50\x14\x19\x9d\xa8\x8a\x03\xac\x2b\x6c\x85"
	"\xd1\xab\xee\xfc\x23\xe7\x20\x34\x6f\xd8\xbc\x18\x74\xb8\xf8\x48\x7a\x8d\x5f\x29\x36\xc6\xb6\x54\x3d\x31\x49\xac\x04\xd1\x1d\xad"
	"\x85\x91\x64\xec\xbe\x63\x1a\x24\x28\x59\xbc\xaf\x87\x60\xb5\xdb\xa8\x4f\x28\x6e\xcd\x75\x54\x08\x70\x88\x81\x72\x80\xca\x2b\xea"
	"\x2a\x55\xa0\x0f\x37\xea\x1c\x56\x33\xf4\x54\xc0\xc9\xa3\x06\x75\x0a\xa7\xc1\xed\x8d\x91\xf5\x07\x11\x62\xf9\x65\x80\x83\xa3\x39"
	"\x88\xb4\x26\x18\x51\x82\x62\x56\x03\x9b\x0d\x5c\xa9\x1f\x10\xe9\xf9\x60\x61\x90\x91\xee\xdb\x5c\x5a\xa8\xad\x3e\x1f\xe2\x54\x98"
	"\x11\x31\xe0\x80\x89\xaf\x65\x5c\x3c\x3b\x51\x2d\x43\x1f\x97\x1a\x2c\xd0\x15\xf5\x61\xff\xff\xc2\x72\xaa\x47\xc0\x58\xea\xf7\x3d"
	"\xfa\xac\x05\x9f\xaa\x52\xf1\x6b\x2d\x40\x6f\x73\x07\x92\xa4\x28\x41\x65\xe8\x14\x8a\xfb\xfe\x25\x6a\xa0\xc9\x44\xf0\x32\x2a\x2c"
	"\xee\xbc\xe0\xeb\x88\xbe\xc5\xe1\xd0\x62\xb0\x65\x31\xed\x92\xdc\x03\x5e\xac\x96\xb5\x5f\xf5\x73\xad\x8e\x36\x15\x59\x28\x77\xfc"
	"\x9d\x09\x00\x33\xc6\x11\xf6\xff\x29\x54\x96\x51\xa8\xec\x2b\x17\x05\x2b\x17\xf5\xde\x4e\xe3\x58\x2c\x97\x0b\xfc\x77\x6f\xba\xee"
	"\xdf\xdd\xd2\x8d\xae\x80\xcd\x49\xcf\x23\x64\xa9\x2e\x4b\x13\x39\xc9\x3d\x42\x7d\x60\xa7\x05\xcb\x12\x29\xa2\xa4\xf8\x4e\x49\x0d"
	"\x24\xae\x53\x44\x5a\x62\x99\xaa\x10\xb0\x74\x16\x01\x69\xe0\xe1\x05\x49\x21\x06\x66\x26\x12\xd5\x01\x58\xb1\x22\x03\xa4\x87\xc8"
	"\x40\xa3\x7b\x31\xd3\xb1\x0e\x0c\x4d\x97\xa3\x80\x81\x73\xec\x90\x50\xcc\x38\x02\xcf\x5a\x9b\xbc\xbf\x1c\xc2\xe2\xbe\x74\xdd\x7b"
	"\x29\xec\xd3\x6b\x3e\x2d\xce\x4c\x87\x9d\xe1\xb6\x8c\xb9\xdb\x20\xb9\xa8\x28\x8d\xf2\xf1\xee\x33\x04\x77\x70\xd2\xf0\x0e\x7a\x23"
	"\x72\xd2\xf8\x0e\x4c\x0d\xef\x20\xbd\x83\x80\xa4\xe7\x3a\x4a\xd6\x97\xa2\x5c\xd1\x4b\x22\xa2\x3d\x12\x6b\xa8\xbc\x90\x98\xc6\x5b"
	"\xa2\xad\xc1\x24\x61\x05\x09\x8b\xb8\x9d\x2d\x9c\x04\x01\x3f\x6a\x1b\xcb\x40\xbd\x20\x6e\xad\xd6\x99\x7c\x08\xd0\xfd\xe0\x3f\x83"
	"\x95\xad\x77\xce\x68\x9b\x19\x0c\xab\xd1\x76\x17\x1e\x04\x84\x32\x00\x92\x45\xe2\x0b\xc5\x00\x5c\x3c\x5c\x41\x5d\xb9\x90\xc7\x10"
	"\xef\x24\x4a\xce\x43\x26\xd0\x08\x5d\x10\x68\xbd\x39\xc9\x09\x8c\x04\x8c\xb0\xe2\xf2\x44\x77\x2e\x0f\xb7\xdc\x93\xbe\x9b\xe3\x29"
	"\x02\x78\xe7\xa2\xd6\x61\xf7\x74\x1a\xeb\x43\xb8\x8b\xb1\x41\xff\xa6\x9f\xc9\xf6\xe5\x91"
;