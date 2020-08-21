#include "GameShaders_hlsl.h"
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
		"TestMesh",
		{ {
			{  },
			5, 1,
		}, {
			{ {0,2}, {1,2} },
			6, 4,
		} }
	},
	{
		"TestSkin",
		{ {
			{  },
			10, 1,
		}, {
			{ {0,2}, {1,2} },
			11, 4,
		} }
	},
};

const SpPermutationInfo spPermutations[] = {
	{ { 1 }, {  }, { 1,2,3,4 }, 0, 1030 },
	{ { 2 }, { 1,2,3,4 }, {  }, 1030, 4851 },
	{ { 2 }, { 1,2,3,4 }, {  }, 5881, 4851 },
	{ { 2 }, { 5,2,3,4 }, {  }, 10732, 5157 },
	{ { 2 }, { 5,2,3,4 }, {  }, 15889, 5157 },
	{ { 3 }, {  }, { 1,2,3,4,5 }, 21046, 1480 },
	{ { 2 }, { 1,6,7,8 }, {  }, 22526, 4897 },
	{ { 2 }, { 1,6,7,8 }, {  }, 27423, 4897 },
	{ { 2 }, { 5,6,7,8 }, {  }, 32320, 5203 },
	{ { 2 }, { 5,6,7,8 }, {  }, 37523, 5203 },
	{ { 4,5 }, {  }, { 1,6,7,8,9,10 }, 42726, 1825 },
	{ { 2 }, { 1,2,3,4 }, {  }, 44551, 4851 },
	{ { 2 }, { 1,2,3,4 }, {  }, 49402, 4851 },
	{ { 2 }, { 5,2,3,4 }, {  }, 54253, 5157 },
	{ { 2 }, { 5,2,3,4 }, {  }, 59410, 5157 },
};

const SpUniformBlockInfo spUniformBlocks[] = {
	{ }, // Null uniform block
	{ "DynamicTransform", 128 },
	{ "Pixel", 1056 },
	{ "Transform", 64 },
	{ "SkinTransform", 64 },
	{ "Bones", 3072 },
};
const SpSamplerInfo spSamplers[] = {
	{ }, // Null sampler
	{ "shadowGrid3D", (uint32_t)SG_IMAGETYPE_3D },
	{ "albedoTexture", (uint32_t)SG_IMAGETYPE_2D },
	{ "normalTexture", (uint32_t)SG_IMAGETYPE_2D },
	{ "maskTexture", (uint32_t)SG_IMAGETYPE_2D },
	{ "shadowGridArray", (uint32_t)SG_IMAGETYPE_ARRAY },
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
	{ "a_tint", 4 },
	{ "a_uv", 1 },
	{ "a_normal", 2 },
	{ "a_tangent", 3 },
	{ "a_indices", 4 },
	{ "a_weights", 5 },
};

const char spShaderData[] = 
	"\x28\xb5\x2f\xfd\x60\x37\xfb\x35\x4e\x00\x7a\x50\x20\x0f\x2c\xc0\x8c\xac\x0e\x10\xa8\x66\x92\x08\xda\xc6\x15\x8c\x9a\xad\xfd\x9a"
	"\x2e\x99\x55\xc6\x4e\xb7\x04\x5c\x77\x46\x4f\x47\x60\x11\x23\x20\x86\x75\x1b\x00\x00\x00\xa8\x5e\x34\x10\x01\xe4\x00\xe5\x00\xea"
	"\x00\xd7\x83\x5b\x52\x6d\x6a\x46\xf0\xc1\x9b\x19\xa1\x4f\x60\x41\x52\xd1\xbc\x6d\x8e\x5e\x60\xc6\x16\x6b\x30\x7c\x6b\xd3\x4e\x9e"
	"\x17\x00\x8b\x09\x37\x6f\x9b\x7a\x36\xcf\x76\x09\xd2\x1a\x67\xcd\xa3\x7f\x2e\xf7\x85\xe8\x44\xbd\xbc\x9e\xa6\x5d\x4e\x8a\xc5\x86"
	"\x97\xee\x44\xc7\x4d\x4f\x7a\x70\xa2\x2e\x4d\x10\x64\xb7\xec\xf2\xda\xab\xff\x8b\x6b\x6f\x18\xd3\xf4\xad\x98\xa6\xa1\x5b\x67\xa6"
	"\x55\xdb\x80\x2f\x9f\x6b\x63\x33\x66\x80\x2f\x96\x6e\x70\xf6\x5b\xdc\x33\x86\x30\x33\xec\x83\xf3\x83\x19\x28\xd3\xad\x6b\x87\xe6"
	"\x0f\x78\x96\x31\x47\xdf\x99\x2f\x9a\x9a\xcd\xdb\x49\x9e\x29\x87\xb3\x65\x34\x18\xc6\x52\xe1\xe8\xce\xd4\x83\x69\xda\xf2\x3c\x56"
	"\x35\xe3\xf1\x74\x6e\xed\xf2\xc4\x2c\x16\xd7\x84\xc5\x55\x79\x36\x6c\x3e\xc6\x7d\x69\xe7\xce\x8d\xcd\x5b\x94\x9d\x59\xc6\xf7\x82"
	"\x92\x05\x0a\x14\x34\x34\x38\x70\xf9\x80\x5f\x5c\x77\xd5\x58\xc3\xe6\xab\x3d\xb1\xe5\x28\x16\x12\xae\x33\x35\xef\xcd\x80\x55\x2c"
	"\xdf\x9a\x2d\x6f\x1d\x91\xa2\x2e\x08\x78\xc6\xe5\xe1\xdc\x73\x73\x8b\xea\x99\xf9\xbe\x15\xf3\x50\xa9\xd0\x3d\x37\x3b\x3d\xf4\x0a"
	"\x0e\x0e\x18\x8c\x04\xe5\x1d\x5e\x5d\xba\xc4\xe1\xd4\x8d\xe6\x55\x9e\x76\xd3\x50\x4d\x63\x0e\x92\x05\x06\x5e\x0d\x50\x78\x81\x13"
	"\x88\x38\x2a\x87\x80\x18\x71\x50\x16\x35\x55\x11\x9c\x62\x86\x2f\xdd\x30\xe4\x19\x6f\x7c\x32\xb9\xbc\x84\x42\x91\xa7\x70\x9d\x0e"
	"\xe3\x8f\x0e\xed\x1c\x41\x3f\xba\x3c\x86\xeb\x74\xe2\x8c\xd0\xec\x9c\x60\x2e\x51\x36\xde\x3c\xbc\x01\x02\x04\x98\x04\xf3\xc7\x45"
	"\x16\x07\x3d\xfa\x83\x9c\x37\xfe\xa0\x33\x0e\x39\x31\x5a\xdc\xa3\x73\x97\x8f\x5c\xe3\x70\xa2\x20\x2d\x9b\x56\x73\x79\x6e\xf1\x26"
	"\x1b\x6f\x30\x97\x20\x07\x92\xd0\x54\x1b\xa1\xb1\x60\x0e\x10\x96\x80\x07\x73\x4f\xc9\xbe\x15\x6b\xb7\xaa\x99\x69\x36\x35\x14\xdc"
	"\x4c\x0d\xc3\x34\x7d\x39\xe8\x1d\xd1\xa9\x94\x6c\xbb\xb7\x94\xab\x95\x57\x55\x12\x7a\x66\x31\x52\x14\xde\xd0\x8c\x9d\x51\x6b\x1a"
	"\xa3\x7b\x6e\x4b\xa6\x56\xcb\xa6\x65\x11\x1a\x47\xb5\x99\xd1\x8b\xf7\x2d\x09\x42\x45\x8b\x82\x8f\x58\x51\x91\x01\x79\x38\x20\x91"
	"\xf8\x10\x00\x79\x58\x91\x09\x46\x10\x08\xa9\x22\x91\xea\xf1\x80\x13\x74\x58\xd0\xa2\xe0\x4b\xa6\x93\xf5\x9b\xe8\x2e\x3d\x6e\xf2"
	"\x78\x5e\xc1\x3f\x6b\x8a\x12\x07\x7d\xb0\x16\x39\x1c\x70\xa2\xda\xd2\x18\x99\x22\xa5\x58\x3c\x08\xac\xa8\x0a\xc2\x85\x73\x1c\x97"
	"\xe9\x64\x0b\xbe\x38\x20\x9c\x60\xf4\xf7\xba\xe9\x01\xd2\x99\x29\x19\xd7\xbb\x04\xa3\x68\x32\xc1\x60\x28\x97\x1e\xde\xe4\x12\x06"
	"\xa2\x7b\xe6\x6d\x4f\xcc\x19\xa7\x4c\xae\xf0\x06\x43\x9e\xf4\x56\x7b\x35\xfe\x18\xa7\x26\x08\xcd\xed\xd2\xbd\xe7\x09\x7c\x31\x41"
	"\x31\x41\x14\x1c\x13\x64\xbc\x9b\xe0\xa6\x07\x5d\x82\x2c\xf5\x50\xca\xa4\x60\x44\x9b\x8c\x71\x03\x47\x2f\x32\xc0\xb9\x49\xbe\x9a"
	"\xd5\x40\xb4\x5b\x02\xaa\x61\xe1\xe7\xd4\x48\x3e\x82\x25\x1f\x4e\x10\xb2\x9b\xa6\xf9\x29\xbc\x92\xa0\x8d\x23\x58\xb6\xae\x95\x37"
	"\xe4\xe7\xb6\x74\xf3\xd1\x5b\xdd\xcc\x05\xb5\x53\xb3\x02\xbb\xb8\x20\xaa\x2a\x06\xb1\x73\xf7\xee\xdc\x1d\xb3\x7c\x2f\xcc\xf2\x7d"
	"\x91\x92\x9e\xa4\x50\x2c\xa0\x4a\xc3\xda\xe7\xf3\x99\x98\x80\x31\x52\x54\xd7\x31\x62\xa4\x28\x6e\x8a\xce\x4d\x5c\xfd\xe0\x7c\x6b"
	"\x71\x55\x5c\xbd\x05\x11\x23\xd4\x83\xcc\x2d\x4a\xd3\x8b\x0d\xdc\x2a\x61\xa1\x1f\x6c\x6a\xaa\xa9\x9b\x5b\x6f\xdd\x22\xf5\xe9\x3e"
	"\xdd\xd5\x45\xfb\xd6\x09\x3d\xed\x66\x78\xf2\xd5\xe1\xd9\xb7\x7e\xba\x7d\x89\xd2\x97\xa0\xa6\x28\x7b\x69\x82\x9b\x34\x79\x68\xdf"
	"\x78\x67\xa1\x85\x6e\xcd\xec\xa3\xb3\x50\xc6\xd8\x72\xa0\x85\x74\x56\xb3\x74\x33\x67\x06\x49\xfc\x74\x4f\x5e\x97\x30\x52\xc5\xba"
	"\x64\x2c\x5c\xf0\xf2\xd0\x33\xe6\x44\x91\x7c\x8c\x3e\xf2\x92\xf4\x2a\x48\xcb\xd4\x72\xb8\x49\x36\x8e\x71\x50\x33\xdd\xc4\x98\x1e"
	"\x5e\x69\xf2\x15\x79\x09\x3d\x13\x58\xf2\x0c\x7b\x66\x4f\x92\x77\x00\xbd\xf1\xe8\x12\x04\x3d\xec\x99\x3d\x5a\xd0\xbe\xf5\x52\xc5"
	"\xb3\x28\xea\x01\x89\x44\xa2\xc4\x59\xd7\xe3\xae\xe7\x39\xc3\x23\xa3\x7b\xe2\xa5\xc6\x01\x82\x8f\xa8\x82\xeb\x56\x2a\x56\x33\x1a"
	"\x10\x80\x8e\x00\x72\x09\x22\x8e\xe7\x61\x9a\xc5\x18\xc2\x00\x12\xe0\x91\x30\x0a\x82\x24\x87\x71\x4a\x21\x63\x10\x18\x22\x00\x00"
	"\x12\x00\x00\x04\x00\x08\x00\x80\x00\x01\x16\x4c\x0f\x81\xca\xea\x00\xe6\x52\xa4\x63\x3b\x11\x30\xe4\x98\x53\x1e\xfb\x3c\xdb\xf5"
	"\x72\xf4\x62\x27\xd1\x27\xa1\x4a\x89\xeb\xa7\xff\x3f\x1a\xdd\x68\xf9\x40\xf4\x3f\xb0\xec\x3f\xb5\xe3\x2a\x38\xa2\xa2\x29\x0d\xe1"
	"\x31\x0d\x80\x14\x08\xf0\x99\xdc\xff\x7c\xb7\x66\xfa\xaf\x89\x85\xf3\xdf\xf0\xc9\xf9\x67\x18\xfb\xad\xa1\x77\x00\xbd\xd0\xaf\xa6"
	"\x95\x0c\x16\x00\x95\xb2\x03\xb8\xab\xea\x6b\xe4\x35\xd9\xdb\xf9\xdf\x0f\x7f\x9e\xa8\x0e\xb0\x81\x00\x9e\x58\x45\x1b\x3f\x0a\xcd"
	"\xf3\x4e\xdd\xb5\xc9\x33\xc7\x77\xf6\x6b\x42\x25\xca\x80\x9d\xd1\x38\x90\xbd\x39\x39\xa4\x68\x1d\x77\x5a\x18\xc1\x85\xfa\x4b\x9b"
	"\x97\x3a\xc0\x55\x4d\x4e\x5e\xab\x8a\x7c\x1d\x8e\x1e\xc4\x52\x84\xe1\xc1\xde\x35\x9a\x6e\xf5\xcd\x54\x2b\x77\xb2\xa3\x28\x54\xb3"
	"\xd4\xf2\x91\x2d\xf6\xe7\xb7\x22\x74\x8b\x25\x77\xe7\xaf\xa7\x55\x08\xa0\x16\xe6\x97\x11\xed\xdc\x09\xc8\xad\x64\xf0\x35\x89\x93"
	"\x8c\xa3\x85\xe2\x88\xf6\xca\x35\x11\x29\x1b\x0c\xd5\xf8\xac\x49\x56\x0d\xdb\xe8\xd9\x28\xe7\x0e\x62\x13\xe2\xaa\xdb\xb7\x72\xa7"
	"\x0a\xef\xec\xb5\xa1\x06\xce\x8c\x92\x60\xba\x9b\x0e\xc5\x08\xd4\x12\xdc\x87\xfb\xb4\x53\x0c\x8e\xfc\x1c\xff\x1c\xd8\xcf\xd2\x49"
	"\xe4\x44\x7f\x2e\x30\xb1\x96\xff\xf9\xec\x65\x24\x01\x69\x81\x24\x87\x3c\xf8\xa4\x7a\x74\x47\x61\x84\xa3\xd6\x0f\x60\x4e\x7c\x0c"
	"\xb2\x92\x15\x3c\xd7\xe9\x5c\xcb\x28\x5d\x79\x3a\xb5\x80\x90\xba\xb2\xd2\x49\xdf\x43\x52\xcd\xaf\x96\xd3\xcd\xa6\x28\x4d\x3c\x9c"
	"\x9a\xc3\xb1\xa8\xc7\x3b\x50\x45\x62\x39\x92\x1a\x74\x6a\xd2\xc9\x1b\x27\xe9\xe1\xf9\x0d\xfd\x84\x51\x55\x52\x6b\x41\x84\x76\xb9"
	"\xec\xf4\x39\x52\x6e\x13\xca\xd9\xee\x8f\x42\x8e\x96\xfa\x6c\x9c\xdf\x40\x91\xe3\xd9\x1b\xad\x8f\x6f\x84\xcb\xc0\x1c\x41\xe9\x54"
	"\xae\x72\x60\xf1\xe5\x17\xac\x16\x86\x3a\x12\x04\x33\xb7\x95\x37\x58\x49\xeb\x17\xaf\xaf\xb8\x93\xf8\xb1\x21\xdb\xb6\xf1\x2a\xef"
	"\x5a\x8e\x98\x75\x20\x51\x7d\x98\xe4\x52\xba\xf8\xbc\x15\x9e\xcd\xcd\xbc\x1e\xff\x89\x52\x48\xc2\xdb\x81\x16\x04\xcc\x38\x94\xa7"
	"\xfe\xf2\xe8\xae\xa5\xee\x0b\x95\x35\x81\xac\x41\xbe\xbd\xb8\xc3\x9c\x09\x99\x7c\x3f\x3c\x70\x97\xc2\x4e\x4b\x1c\x4f\x08\x4b\xac"
	"\x8d\xf9\xe2\x4f\x3f\x0a\x0a\xfa\xe9\x70\xd5\x65\xe8\x18\x20\x8b\xc3\x69\x5b\x97\x6d\x1a\x35\x78\x2f\x44\xa3\x69\x7a\x57\x46\x41"
	"\x90\xa9\xf6\x4c\x79\xab\x8e\x4f\xa8\xe4\x0c\xd4\x55\x00\xb8\x8a\x16\xf8\x6a\xde\x76\x09\xcd\x9f\xa9\x75\xea\xda\x4d\xfc\xa6\xa6"
	"\xb7\x2b\xd9\x0d\x6e\x5e\x9e\xc7\x38\x89\x1d\x32\x3a\x71\xea\xc2\xda\x41\x7c\x43\x09\x41\xa8\xcf\x6d\xb8\x6a\xbd\x9d\xa6\x8c\x14"
	"\x48\xba\x37\xa4\x09\x80\xfe\xa0\x3d\x15\x72\xbc\xe4\xc1\xbf\x78\xc8\x0a\x66\x85\x35\x50\xb5\x6b\x92\x18\x38\x9a\x7a\xbf\x3b\x0d"
	"\x79\xad\x7c\xd3\x9c\x86\x48\x4b\xa3\x64\xaa\x60\x69\xac\x9a\x37\xb9\xc6\x06\x2c\x15\x4a\x10\xbc\xc1\x81\x71\x78\x99\xbf\x84\xa8"
	"\x35\x12\xc0\xbf\x27\x4b\x66\x61\xa0\xd5\xc2\x65\xf9\x1a\x08\x2f\xbb\x01\xe0\x07\x30\xf0\x73\x5d\xf3\x69\x3e\x91\x3c\x16\x09\x5e"
	"\x19\x82\x71\x48\x30\x00\xd9\xa7\xc4\xb4\x43\x68\xb8\x50\x64\x0e\xf1\x5c\xf6\xb3\x96\xfe\x85\xca\x8b\x60\x1f\xc5\xaa\x8e\x48\x0c"
	"\x78\x0f\xc2\x34\x41\x5e\x1e\xc4\x57\x18\x3c\x44\xb2\x8d\xb5\x99\x5a\xb9\x69\x98\xd2\x84\xcc\xae\x4f\x2d\x74\x52\x2a\x11\xde\xe5"
	"\xb0\xfc\x04\x14\x1e\x68\x0a\x49\x98\x7b\x81\xec\xbc\xa0\x22\x1b\xda\x05\x4b\x5d\x8e\x77\xf3\xbd\xfc\x2f\x19\xdc\x09\x25\x37\x30"
	"\xa8\x19\x23\xb2\x08\x3f\xac\x48\x12\x1c\x22\x31\x7f\x7b\xdf\x7d\x28\x47\xb2\xa3\x91\x42\xff\x10\x9c\x1c\xcb\xee\x30\x22\xac\x97"
	"\x4b\x06\x62\x06\xb2\x17\x7e\xab\x2a\xee\xf0\xda\xc0\xd4\x0c\xca\x31\xc2\x13\x1a\xc8\x15\xe9\x3c\xd5\xb3\xb3\xa0\xe3\x17\xb7\xc0"
	"\x80\x2a\x9b\xbe\xab\x77\xc9\xa8\x62\x7e\x95\x1e\xf3\x31\xa4\xd8\xd9\x27\x4f\x77\x7e\x11\xec\x17\x56\x03\xfb\x41\x2f\xf4\xab\xc7"
	"\xc1\x01\xc8\xb1\x97\x3a\x2e\x17\x11\x20\x83\x53\x6a\xd4\xa8\x5d\xcb\x32\xb4\xea\x1c\x02\xbc\xef\xc0\xd9\x3d\x19\x13\xef\x55\x98"
	"\x4f\x97\xd8\x84\x84\x6b\x2b\xa7\xb2\xbe\x01\x4d\x75\x2b\xdb\x26\x3d\xb4\x35\xcb\xd5\x61\x8c\x60\x7a\x68\x93\xbe\xc3\x95\x4d\x82"
	"\xf3\xc0\x70\x66\x2b\x83\x61\xc8\x9a\xd1\xfc\xf0\xce\x43\x96\x3e\x14\xb1\xae\x11\x17\xd9\x73\x13\xd7\x0b\xda\xf2\x2b\x1f\x20\xc7"
	"\x06\x64\x61\x55\x7a\xab\x65\xe4\x78\x1d\x85\xec\x9d\x00\x90\x4e\xf1\x7c\xa1\x0a\x1a\x26\xcc\xb8\xe5\xf8\xd5\x7c\x12\x7e\xf1\xd1"
	"\x6a\x8c\x8e\x0c\x62\x2f\x19\x43\x81\x5a\x0e\x8b\x7b\xfd\xb9\x98\xe7\x35\x42\x03\xca\x1b\x59\xb0\xc0\xde\xc3\xe8\x24\x4a\x8b\xc2"
	"\x81\x0e\x53\x23\x16\x55\xc6\x38\x4c\xe9\x32\x49\x18\x3c\xe1\x5e\x57\x05\xb4\xce\xbc\xd3\x5f\xef\xef\xa1\x11\x14\x41\x6e\xfa\xdf"
	"\x21\x30\x51\x2c\x61\x0b\x4e\x35\x28\xa0\x76\x30\xde\xfa\x6e\xce\x60\x34\x26\x95\x9d\x8b\x96\xa0\x59\x0a\xeb\x44\xc8\x70\xa8\x3e"
	"\xf4\xf2\x60\x8d\x46\xe5\xb9\x4d\x77\xc4\xe7\x29\x5f\x40\x95\x75\x49\xc6\x31\xaf\xed\x8c\xae\xf4\x56\x70\xc4\x14\x14\x1d\x8b\x91"
	"\x39\xd4\x36\x51\x8d\x42\x27\x8d\x4b\x6d\xe9\x74\xc4\x31\x89\x92\x58\xe5\xe2\x68\x19\x0b\x36\x65\xbe\x1d\xc6\x91\xe8\x9a\x3d\xfb"
	"\x9d\x40\x4e\xe1\x63\x37\x1e\xef\x5c\x70\x14\x6c\xca\x21\x66\x40\xe1\x09\xb2\xa7\xae\xbf\x47\x35\x0a\xeb\x60\x76\x3f\x60\x64\x64"
	"\x3d\x05\x41\xf0\x7a\xbf\xe7\xab\xa0\x87\x10\xa3\x45\x3b\xa6\x6f\x89\xf7\x63\x4b\xe6\x57\x8e\xe2\xe4\xa6\x72\x53\xdd\xe0\x15\xa3"
	"\xbb\x5e\x32\xc0\xe0\xf0\x51\x82\xd8\xe5\x5b\x92\x0f\x48\x8d\x53\x61\x0b\x13\xcc\x0f\x23\xe0\x32\x0d\xf1\x7b\x78\x62\x4e\x27\xdd"
	"\x6e\x0d\xdb\x2e\xdc\x25\x62\x21\x43\x66\x51\x2c\xc8\x13\x8c\xa9\x22\x58\xb3\x9f\xed\x84\x38\x52\x4b\x90\x7f\x61\xec\x75\x44\x49"
	"\x1c\xa8\x4e\x9f\xee\xda\x02\x99\x9a\x02\x79\x2f\x11\x79\xf6\x62\x2b\xba\x6f\x1c\xf7\xee\x67\x6e\x11\x03\x0d\xc4\xa8\x33\xbb\x75"
	"\x85\x64\x52\xa5\x57\x34\xe1\x6b\x06\x44\xc0\x20\xb5\x0b\xc6\x95\x20\x48\xdd\xab\xcb\xd4\x86\xb8\xa0\x2d\xec\xc4\xe4\x3b\xb9\x3d"
	"\xc4\x06\xf4\xc1\x2e\x76\x0d\x19\x6e\xb9\xf5\x7f\xc9\x89\xfb\x92\xe4\xab\x46\x92\xd7\x45\xc9\xc2\x45\xa2\xd8\x32\x5e\xc2\xcb\x88"
	"\x95\x1d\x6b\x85\x81\x04\xc6\x51\xb7\x1d\xac\x6d\x46\xbe\xb2\x2d\x6e\xf5\xa9\x18\x71\xc6\xa6\x63\x4f\xb2\x0c\x1b\xfb\x3f\x18\xa3"
	"\x71\xec\x24\xaa\xbd\xfc\xc1\xae\x1a\x44\x84\x24\xda\xd6\xfe\x35\x34\xf3\xa2\x48\x90\x45\x26\x5f\x01\x0a\xb9\x5f\x72\x38\xc9\x72"
	"\x24\x2c\xae\x95\x9a\xac\xc4\x7d\xe3\x0c\x31\x2a\x33\x1e\x8d\x7c"
;