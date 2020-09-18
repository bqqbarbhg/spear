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
	{ { 1 }, {  }, { 1 }, 0, 1132 },
	{ { 2 }, { 1 }, {  }, 1132, 1713 },
	{ { 3 }, {  }, { 1,2,3,4 }, 2845, 769 },
	{ { 4 }, { 2,3,1,4,5,6 }, {  }, 3614, 7940 },
	{ { 4 }, { 2,3,1,4,5,6 }, {  }, 11554, 7940 },
	{ { 4 }, { 7,3,1,4,5,6 }, {  }, 19494, 8194 },
	{ { 4 }, { 7,3,1,4,5,6 }, {  }, 27688, 8194 },
	{ { 5 }, {  }, { 5 }, 35882, 347 },
	{ { 6 }, { 2,1,8,9 }, {  }, 36229, 6386 },
	{ { 6 }, { 2,1,8,9 }, {  }, 42615, 6167 },
	{ { 6 }, { 7,1,8,9 }, {  }, 48782, 6640 },
	{ { 6 }, { 7,1,8,9 }, {  }, 55422, 6421 },
	{ { 7 }, {  }, { 1,2,3,4,6 }, 61843, 1346 },
	{ { 4 }, { 2,3,1,10,11,12 }, {  }, 63189, 7981 },
	{ { 4 }, { 2,3,1,10,11,12 }, {  }, 71170, 7981 },
	{ { 4 }, { 7,3,1,10,11,12 }, {  }, 79151, 8235 },
	{ { 4 }, { 7,3,1,10,11,12 }, {  }, 87386, 8235 },
	{ { 8,9 }, {  }, { 1,7,8,9,10,11 }, 95621, 1556 },
	{ { 4 }, { 2,3,1,4,5,6 }, {  }, 97177, 7940 },
	{ { 4 }, { 2,3,1,4,5,6 }, {  }, 105117, 7940 },
	{ { 4 }, { 7,3,1,4,5,6 }, {  }, 113057, 8194 },
	{ { 4 }, { 7,3,1,4,5,6 }, {  }, 121251, 8194 },
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
	"\x28\xb5\x2f\xfd\xa0\xa5\xf9\x01\x00\x2d\x80\x00\x3a\x75\xf4\x15\x2c\xd0\x6e\xc8\x0c\x90\x01\x46\x8c\x4c\x61\xaa\x94\x10\x61\xd9"
	"\x13\xb7\x92\xeb\x0e\x4a\x3d\xb5\x63\x0f\x0e\x50\xcc\x0a\xe5\x4a\x36\x96\x33\x3a\x9f\x69\x06\xb9\x9a\x99\x0c\x70\x02\x4e\x01\x54"
	"\x01\x58\x01\x5d\x07\x71\x97\xe0\x84\x78\xea\x7f\xe1\x00\xa7\xe7\x6d\x06\x91\x4e\xe8\x1b\x4f\x67\x0d\x2d\xf9\x17\x70\x4e\x9e\x7a"
	"\xaf\xcb\xc0\xc4\x0a\xe2\x99\x34\x11\x89\x02\x07\xb8\xdf\x4e\x5f\x0f\x67\x78\x8d\xc1\x3c\xe1\xae\xee\x09\xb4\x9b\x79\x9b\x01\xe6"
	"\xba\xeb\xca\xd6\x33\xcc\xb4\x71\xcd\xab\x0a\xff\x68\xe7\x9d\x19\x66\x8a\x40\x1b\xd6\x5a\x76\xb9\xd7\x3e\xae\x01\x30\x15\x74\xd6"
	"\x7f\xe4\x3b\x4d\xd3\x34\x93\x8a\xc6\xa8\xa7\x3e\x9c\xcf\x03\x4e\x07\x68\x67\x2c\x6f\xfc\xe2\x7c\x08\x4e\xf9\xf1\xa9\x77\x68\xfc"
	"\x13\x23\x38\x1b\xea\x1b\xd6\xc6\x79\xf5\x5d\x6c\x58\x31\xa7\x31\x38\xa9\x8f\x91\x36\x7f\x65\x8a\x29\xc6\x01\x0c\x33\xc5\x88\x83"
	"\x8a\xc6\x1a\xea\x41\x6c\x6d\x07\xb8\xda\xb7\x73\x3b\x77\xcf\x30\x19\x3e\x36\x3b\xdf\xec\xad\xcc\x3d\x86\x99\x2a\x98\x89\xeb\xb2"
	"\x75\x23\x38\x49\xd6\xed\xef\x9b\xeb\x14\x58\xc6\x0f\x1b\x0e\xaf\xae\xd4\xba\x5a\xa5\xa2\x71\xbf\x60\x26\x7c\xc9\x12\xb6\xc2\x5e"
	"\x5e\x30\x93\x75\x1f\x58\x79\x6b\xc0\x7d\x54\x61\x6f\xc1\x4c\x5a\xd6\x0b\xf8\x84\xa5\x37\xfe\x7d\xd4\x6f\xb6\x60\x32\x3c\x88\xfb"
	"\x86\x29\x78\xc3\x14\x9a\x84\xb7\x19\x60\x9d\x05\x47\x3e\xdb\xf8\x95\x77\xf7\x91\x46\xd5\xea\x9c\xbe\xd9\xc6\xb7\xd7\x3b\xce\x30"
	"\x53\xe6\xb8\x56\xb9\x0f\x55\xd8\xfb\x9d\x3f\x52\x6a\xf3\xdc\x6f\xc5\x75\xd3\x3b\x1e\x5e\xcd\x30\x53\xbf\x5c\x5f\xaf\x65\x5d\xb6"
	"\x70\x96\x2b\xbc\x69\x0c\xc3\xe1\xa7\x14\xfa\xcd\x33\xb7\x61\x4f\x40\x7d\xc3\x40\x3d\x05\xc4\x30\x7b\x29\x34\x7d\x1f\x28\xff\x48"
	"\x3a\xb7\x20\x26\x34\x68\x50\x89\x50\x40\x19\x81\xf4\x7a\xc6\x37\x97\x8d\xc0\xa7\xde\x32\x1e\x3a\xb7\x28\x0f\xbd\x2e\x54\x59\x53"
	"\x54\x12\x15\x07\x0b\x91\xa3\xc2\xed\x37\xd2\x77\xe6\xda\x04\x4d\xee\x86\xb3\x5b\xd9\xea\x70\x81\xbb\xd9\x04\xb7\xb0\x45\xdf\x8f"
	"\xd8\x09\x3f\xb6\x74\x0b\x3e\x6e\x5d\x0a\x6e\xe7\x9d\x77\x7e\x38\x8e\x6f\xd5\xcb\xff\x26\xf5\x7d\x2f\x34\x6d\xc1\xf4\xad\x4d\x15"
	"\x93\x04\x3e\xa1\xb1\xc5\x33\x6e\xdf\x6d\xa2\xca\xd9\xdd\xa1\xf7\x36\x5b\xbc\xae\x72\xdf\x42\x7d\x63\x6c\xdf\x5e\xb9\x1f\xd9\x4a"
	"\xfd\x33\x59\xd4\xd8\xe0\x4b\x7e\xea\xbd\xa7\x7e\x42\xbb\xee\x38\xbc\xc5\xa8\xd6\x86\x87\x04\x81\xd8\x49\x3b\xb7\xa2\x29\x82\x53"
	"\xf4\xde\x69\xbc\x07\x51\xf2\xa9\xc7\x5b\xc7\x71\x0d\xc4\xda\xcd\xf0\xe6\xfd\x79\x54\x1b\x55\xda\x60\xa1\x6a\x9d\x47\x83\xf4\x80"
	"\xd8\x06\xc7\xa3\x21\x82\x4c\x2a\x1f\xf4\x01\x41\x20\x50\x43\x03\x9d\x54\x8a\x68\x24\xe1\x13\x16\x6c\xe3\x62\xbc\x84\xf3\x89\xe0"
	"\x94\x24\xc6\xc7\x8f\xd1\xb4\x0f\xd8\x7c\x5c\x93\x92\x44\xe7\x82\x99\xc0\x97\x6f\x21\xd3\xd6\x05\x13\x1f\x14\xaa\x10\xa5\x94\x81"
	"\x07\x9b\xbf\xc0\x46\xcb\xae\xae\x16\x8d\x05\xdb\x24\xf8\xc4\x76\xf1\xce\x9a\x61\x2a\xe4\xef\xad\xeb\x78\x5b\x30\x52\x74\x91\x17"
	"\xc8\x86\x32\x74\x2e\x21\x38\xe3\x13\xad\x7f\xc2\x27\xd6\x2f\xf0\x74\x3a\x1a\x74\xd2\x47\xf0\x74\xd2\x03\xe0\x84\x68\xde\x79\xb0"
	"\x69\x2c\x2e\xc1\x5b\x59\x7c\xa8\xa7\xd0\x30\x5e\xbe\xd7\x35\x0d\x44\xbf\xd9\xdb\x68\xe3\x49\x3a\x3f\xc9\x7f\x22\xfe\xd1\x96\x4c"
	"\xa7\x64\x71\x89\x44\xd6\x75\x6e\xf3\x4a\x5e\x66\x84\xbc\x84\x33\x42\x86\x4e\x44\x84\x04\x67\xc4\xa2\xc2\x9a\x34\x29\xa3\xe9\x15"
	"\xd6\x17\xde\x9e\xfa\x08\xc9\xe0\x38\xbc\x29\xb0\x8e\x6f\x51\x86\x93\x5e\x2f\x25\xc9\xc0\xfa\x56\xdc\x4b\x87\x65\x45\x90\x65\x55"
	"\x87\x63\x5a\x08\x04\xa9\x92\x55\x2d\xc7\x84\x18\x96\xb5\x92\xb5\x20\x10\x47\xb5\xa4\x88\x3a\xa8\x43\x0a\x5f\x3a\x0f\xf8\xd2\x5a"
	"\x5e\x36\x3b\x67\x10\x07\x9d\x92\x23\xa2\xd1\x7c\x3c\xf5\x92\xf4\x98\x51\x04\x51\x08\x64\x52\x09\xf2\x98\xd5\xf2\x78\x38\xea\xa4"
	"\x0c\x0b\xf5\x21\xa5\xf3\xc8\xe6\x38\xae\x31\x7d\x73\xc6\x77\xa8\xf9\xa4\x81\x43\xc4\xdb\x0c\xe2\x43\xaf\x87\x07\x0a\xe5\xe1\x1f"
	"\x0a\xe5\x9d\x97\x3c\xfc\x43\xa9\x58\xc8\x28\xf9\x07\x62\x62\x7c\x74\xbe\x4e\xda\xdc\x6e\xe3\xde\x0f\x3c\x53\x41\xe7\xa3\x21\x4d"
	"\x7a\x01\x67\x6c\xfe\x59\x38\x72\x01\x88\x27\x02\x3c\x38\xa1\x9b\x15\x58\xdd\x3d\x58\xb0\x30\x20\xa9\xc7\x20\xb1\xc5\x46\x7c\x9a"
	"\xce\x31\x48\x74\x2e\x4f\x9e\x41\xfd\xed\x1a\x5b\x5f\xa7\xc0\x20\x68\x66\xb2\x00\x41\xd6\xd6\x3a\x59\xa0\xef\x3c\xf5\xf4\xf6\x6b"
	"\x7d\x84\xb9\x6b\xf9\x89\x83\x03\x85\x42\xfd\x3f\xff\x6c\x57\x77\x15\xd6\xb8\x8a\x43\x24\x42\x41\x81\xc6\x23\xf8\x03\x10\x09\x92"
	"\x1c\x16\x22\xd5\xa8\x4e\x48\xd2\x69\x33\xbe\xb1\xe5\x50\xef\x3c\xe3\x1f\x22\x12\x09\xf5\x12\x10\xc3\x9c\x7f\xf9\x4d\x16\xdb\xbd"
	"\x63\xbb\xdb\x6c\xdb\x9b\x66\xdb\x8e\xb6\x3e\xf1\x59\x2c\x18\xed\xfb\xbe\xcb\x85\x5a\x5b\x6b\x18\x3a\xd6\xd6\x0a\x4e\xf9\x16\xe8"
	"\x83\xf9\xe3\x1b\xf5\x50\x13\x81\x39\x2f\x2c\x58\x48\xad\x60\xf9\x88\xc3\x9b\x2b\xd8\xf9\x82\xc4\x27\x71\xb8\xca\xd6\xce\x11\x64"
	"\xbf\xf0\x0b\x77\x8e\x77\xf1\xdd\x9f\xaa\x64\xe9\x67\x78\x8a\x04\x9f\xa0\x35\xc2\x85\xea\x6a\xbb\xf3\xfa\xce\xd1\x14\x27\xcf\x68"
	"\x50\x32\x1e\xa0\xe8\xd8\xc6\x1a\xeb\x28\x99\x8a\x86\x49\xe9\xac\x52\x9d\x94\xce\xca\xf8\x07\x62\x8a\x8d\x8f\xe1\x6c\x98\x50\x33"
	"\xf9\x70\x12\x20\xc0\x60\xa6\x0b\xf5\x35\x62\xd3\xd4\xc7\xeb\x26\xf2\xa9\xef\x5c\x76\x48\xf6\x9b\xf5\x9b\x14\xc3\x6e\xad\x0f\xf6"
	"\xd1\x77\xf1\xba\x2e\xc2\xef\x0f\xc3\xfe\x05\xbe\x33\xa8\x03\xe0\xdc\x97\xb9\x19\xfd\xe3\x87\xf1\x39\xcb\x75\xfd\xf5\x56\xdf\xcd"
	"\xc2\x14\xb7\x57\x10\xb0\xc5\x2b\x5b\x25\xbb\xe5\x48\x78\xb7\x87\xbc\x2e\x47\xf8\x04\x45\xd7\xb0\xa5\x65\x5d\x55\xba\xe1\xeb\xea"
	"\x2c\x18\x05\x87\xf1\x8f\x04\x27\xc7\xb5\x86\x6b\x1c\xd7\xde\x42\xb6\x62\x5d\x8d\xc2\x34\x41\xcb\xfa\xb2\x5f\xd8\xff\xa9\xdc\x3f"
	"\x94\xab\x55\xff\x73\x01\xa7\x46\xf5\x33\x3e\xa4\xba\xd9\x55\x37\x84\xba\xd9\x38\xd7\x17\xd5\x0f\x4d\x6b\x12\xe1\xdd\xb6\x04\x11"
	"\xfb\x85\xd3\x63\xa3\xa7\x1c\x03\x53\xf3\x1b\xbf\xc2\xf8\x0f\xe5\x22\xdf\x91\xf2\xa3\xfe\x79\x22\xd4\x80\x99\x5e\xe0\x64\x3c\xe5"
	"\xe5\x53\x70\x2e\x70\xf7\x41\xbf\xd9\xa5\x49\xd0\x6f\x84\x6d\xa8\xe3\x87\xb3\x9c\xa4\x4c\x8d\x88\x08\x00\x10\x80\x02\x53\x12\x60"
	"\x50\x78\x68\x30\x13\x0b\xc5\xab\x30\x44\x3e\x13\x80\xc7\x45\x83\x91\x48\x1c\xcf\xc2\x20\x87\x51\x10\x32\x88\x10\x83\x00\x01\x00"
	"\x00\x00\x20\x00\x42\x03\x00\x30\x23\xc2\x02\xcd\xbe\x5f\x5b\xdf\x73\xac\x85\x27\x53\xa9\x44\x83\xa0\x7b\xda\x8a\xe9\x68\x80\xa3"
	"\x55\x65\x3d\x00\xf6\x1b\xa9\xe6\x80\x99\x5e\xc6\x7a\x24\x4f\xd4\x12\xe6\xb1\x62\x4f\x3b\x2d\x34\xef\x22\xcf\x01\xd7\xf5\x40\x10"
	"\x24\xbb\x79\x7b\x1e\x17\x78\x0c\x1b\xe7\x6b\x0d\x4a\x26\x53\x0c\x47\x23\xa7\x3d\x9b\x1a\xc0\xd4\xcb\x2d\xfe\xd4\x09\xf2\xd6\xff"
	"\x84\xe5\xde\x76\x4a\xc0\x03\x24\x13\xda\x38\x8e\x73\x1b\x8e\xba\x87\x52\x2c\xb6\xe1\x78\xb1\x8e\x99\x7e\x21\xa5\xa1\x43\x09\x53"
	"\x11\x4f\x5f\xff\xba\xa2\x46\xc7\xa3\x6b\x29\x01\x0f\x1d\xbf\xae\xb3\x91\xfa\x60\xc9\xc7\xc0\xe4\x80\x08\x42\xb4\xe2\xbd\x55\xeb"
	"\x92\x97\xea\xfa\xff\xb3\x49\x2c\xf1\x04\x79\x6f\x0b\x82\x50\x50\x1d\x14\x2e\xd7\x0f\x40\x8f\xb7\xe5\xd2\x93\x5d\x80\xdf\xb1\xbd"
	"\xf5\x3b\xaa\x42\x29\x92\x52\xc3\x22\xc8\x8b\xab\x5f\x57\xa8\x91\xfc\xa2\xf3\xf4\x3b\xdf\x15\x92\x17\x75\xe1\xbd\xf8\x59\x98\x97"
	"\x6e\xde\x38\x1a\x17\x57\x43\x77\xc4\x23\x12\x6c\x28\x1b\x4e\xfc\x45\x8d\x49\xef\x9f\x1d\x4d\xa8\xf7\xad\xbc\x34\x21\x5f\x3c\x68"
	"\x71\x19\x7f\xfd\x05\x29\x1f\xdb\xc0\x99\xfa\x76\x02\xd1\x72\x26\x34\x3e\x81\x02\x27\x4b\x77\xd2\x1c\xbd\x23\x5d\x85\xb9\x0b\x48"
	"\x15\x8f\xd8\x62\x9c\xe4\x62\x45\xe1\xcf\x21\xdb\xa4\x91\x54\xc0\xcb\xba\xd2\xf4\x3a\x83\x29\xb7\xd6\xbe\xa1\x87\x81\xe8\xa2\x01"
	"\x92\x66\x33\xda\x31\x87\x16\x31\x1f\xdd\xcd\x14\x1d\x58\xe8\xe2\x9b\x5d\x93\x33\xe8\x49\x21\xf4\x19\x1c\xa4\xc5\x4a\x41\xc6\xab"
	"\x02\x83\x46\x13\x40\xcd\x9a\xe8\x19\xce\xdd\xac\x0d\xe6\xcc\x4b\x14\xed\xf4\xb5\x9e\xe2\xdd\xfc\x5c\xab\x50\x20\x7d\xe0\x22\xe7"
	"\x2a\x69\x76\xa1\xa9\x59\x9d\x35\xa5\x24\x5b\x4e\x75\x4e\x94\x03\xa7\x19\xcb\xca\x17\x24\xc1\x45\xf2\xcc\x65\xe1\x01\xc2\x8c\x4b"
	"\x75\x9b\x4f\xd6\x01\xa8\x20\x35\x9a\x66\x18\x59\xc5\xf3\x7a\x82\x34\xa0\x42\x0d\xab\x20\x94\x5d\xc6\x86\x94\x2b\x32\x4d\x72\x6b"
	"\x0f\x34\x10\x89\x2d\xf6\xbe\x7a\x9e\x1b\xab\xc7\x14\x83\xec\x7a\xa4\x93\x76\x09\x5c\x6b\x8f\x8a\x8d\xfb\xe0\x7f\x6a\x29\x48\x31"
	"\xd9\xc7\x06\x8a\x8f\x6d\xa8\x94\x82\x36\x93\x06\xc6\x63\xac\x4f\xa1\x63\x78\x6d\x3d\x1f\x2a\xb7\x66\xe4\xe0\x97\x64\xfc\x4a\xa3"
	"\x72\xe8\x94\xb9\x86\x4a\xa4\x34\x62\xaa\xce\x57\x67\x49\x42\x09\xe0\x94\xa9\x06\x5d\xe6\xd9\x10\x75\xb0\xe7\x36\xd7\x00\x00\xe1"
	"\x48\xe5\x19\x84\x0f\x2a\x3b\x14\xbe\xcc\x14\x05\x4f\x34\x55\xd3\x73\xa4\xf9\x0a\x95\x77\x56\xe6\x5d\x20\x81\x46\x35\xc6\xf5\xc4"
	"\x27\x1d\x0e\xa8\x58\xf2\x3e\x37\xd7\x52\x5e\xaf\x0d\xbd\x40\x46\x0a\xdc\x85\x7a\xd9\x69\x87\x25\x73\xb9\xe3\xa5\x89\xb3\xfb\x1c"
	"\xef\xca\xfd\x1f\x36\x67\x90\x25\x06\x28\xe8\x32\xdb\x66\xa7\xcf\xf1\xe6\x21\x73\x34\x62\x65\xbd\x10\x5a\x7e\xaa\xb8\x93\x9e\xb1"
	"\xca\xa8\xab\xa5\x3a\x14\x07\xac\x2e\x05\x6b\x8d\x40\xb6\x13\xe0\x1e\x89\xdf\x32\xfd\x17\xb4\xcf\xa6\xab\xa2\x26\x74\x62\xb6\xef"
	"\x02\xf0\x61\x1b\x65\x76\x39\x24\xd4\xc6\x6e\x4c\x08\x24\xc1\x0b\x78\x2a\xcc\x28\x94\x0c\xc5\xf9\x18\x38\xae\x38\x0a\x51\xf1\x89"
	"\x97\x31\x6a\xd8\x62\xd6\xc5\x17\x76\x31\x43\x3f\xab\x48\x92\x47\xf4\x1b\x76\x92\xa4\xeb\x5c\x41\x2d\x08\x17\x3e\xaf\x99\x36\x41"
	"\x5f\xc4\x2a\xb6\xc3\x4d\x46\x4b\xc6\x80\x90\x3d\xee\x3e\x40\x00\xf2\xae\x84\x7c\x9a\x2d\x3f\xfd\xe3\x33\x27\x81\xd7\x00\x81\x0f"
	"\x56\x0f\xb9\x84\xf7\xef\x88\x3e\x54\xce\x37\xac\x1c\x92\xc9\x17\xc2\x30\x38\xaf\x25\xda\x0b\xf4\x59\xc6\x71\xbd\x66\xeb\x27\x58"
	"\x81\x0d\xac\xe8\x88\xd2\x06\xe0\x84\x0b\x36\x80\xdc\xb6\xa6\x67\x4b\xa0\x7e\x7a\xc3\x83\xaf\xd4\xdd\x20\x64\x06\x98\x3f\xa0\x5e"
	"\x2c\xc0\x70\xa4\x5f\xa0\x9e\x28\x60\xbb\x47\x80\xb6\x8a\x80\x2e\x2b\x80\x6e\xe9\x55\x1f\xf9\xf0\xbc\x67\x70\x6e\xb9\x18\xa0\x25"
	"\x6f\xcd\xc0\x8d\x29\x59\x31\x47\x89\x82\x00\xd7\x12\xdc\xa0\xd5\x66\x7c\x49\x07\x91\x08\x04\xc1\xcc\x54\xef\xef\x61\xf0\x6c\xc7"
	"\xe1\x37\x6f\x0e\x12\x82\xdc\x2f\x40\x0f\xe7\x56\x5b\x44\xc3\xe7\xa5\x0c\x0c\xaf\x01\x06\x30\x29\x00\xd5\xd7\xc2\x38\x96\x10\x01"
	"\xfd\xc7\x25\xb8\xb0\xe5\x02\x00\x75\xd3\x47\xc2\x6f\x97\x60\x80\xba\xb0\x5d\x06\xc4\xbd\xc2\xc4\xea\x5c\x6f\x13\xf8\x05\x22\x36"
	"\x25\x01\x87\x10\x73\xeb\xf1\x7d\x82\xc7\x20\xc7\xea\xd5\x8c\x5d\xd6\x3b\xa8\xd4\x65\x61\x95\x13\xa9\x1d\x99\xf4\x8f\x6a\x87\x77"
	"\x92\x58\x10\x05\xf4\xe2\x7a\x65\xf4\xf0\x7e\x85\xcf\xde\x34\x4b\xf5\x1b\x14\xc4\xf6\x75\x11\xce\x8c\xeb\x08\x9a\xbe\xec\x9d\xcb"
	"\x97\x25\x7b\xac\x3c\x8b\x9c\x40\x0e\xcb\xe4\xb5\x58\xa2\xbe\x74\xde\x83\x77\xba\x16\x82\xe8\x08\x04\xd0\x04\x26\xea\xf0\x0a\xe3"
	"\xb4\xe4\xb8\xc6\x37\x63\x4d\xdf\x0b\x07\xa2\xd6\xe8\xce\x98\x88\x51\x30\x9b\xe1\xe3\x5e\x7e\x19\x7b\x81\x52\x37\xc9\x6e\x66\x6f"
	"\x22\x59\x22\x94\x57\x68\xc5\x6c\xcb\x55\xdf\x38\x7d\x3c\x71\x77\xeb\xf3\x8d\xe7\x9c\xb5\x79\xc1\x68\x71\x40\x2c\x6a\x15\xbd\xe6"
	"\x4b\xdc\xcd\xa1\x7b\x82\x85\x31\xa7\x41\x63\x5d\xae\x25\x46\xd2\xe2\xf5\xa2\x44\x12\x48\x01\xe9\x97\xe4\xd5\xec\x4d\x68\x46\xf0"
	"\xe5\xab\x59\x09\xa0\xf7\x74\xba\x61\x00\x4a\x12\x31\x19\xe0\x24\x65\xa7\x2b\xcc\x46\xab\x0a\x62\x7e\x6c\xb1\xe8\x0b\x68\x0f\xc2"
	"\x50\x09\x2f\x3b\x2a\x0d\x87\x0d\xbf\x06\x5d\x85\xdd\x37\x48\x80\x5e\x83\x0e\x07\x16\x59\x81\xe0\x72\xc9\x79\xb6\x66\xe5\x70\x13"
	"\x2b\xb8\x70\x46\xd5\xda\x79\xb1\x4e\xf1\xb3\x67\xb0\xc3\xeb\x9c\x82\xea\xd0\x6e\x42\xb8\x6c\x7c\x44\x88\x4a\x90\x05\xe0\x9f\x55"
	"\xd5\xf3\xda\x77\x14\x86\xe8\xc9\xf9\xbc\xe8\xd2\xae\xe0\xb3\x75\x74\x11\x21\x20\xf7\x0b\xfa\x83\x5d\x58\x2e\xa1\x34\x2e\x1d\x5a"
	"\x33\x42\x24\xef\xb7\x7e\x38\x97\xfd\xe6\x92\xc3\x62\x6f\x8f\x1e\x67\xb4\x2e\x99\xce\xa9\x7f\x31\x43\xd1\x16\xe4\xf8\xe6\x1f\x6e"
	"\xbd\xee\x7a\xd5\x9d\xce\xcd\x68\x5f\xa8\x73\x2a\xac\x3e\xf9\xa1\x8e\xfb\x29\x80\x02\xa0\xa5\x27\x4a\xa5\x9d\x67\xe5\x31\x86\x0c"
	"\xd6\xd6\xe7\x81\x99\xe7\xc9\xa6\x37\xe8\xa5\x15\xa6\x92\x48\x39\x59\x76\xc8\x5a\x41\x2d\x12\x81\x9a\x7b\x22\x82\xb6\x15\x8b\x82"
	"\x27\xea\x89\xde\x18\xca\x80\x56\xaf\x22\x90\x51\xb7\x37\x17\xd4\xbf\x34\xdc\x94\x1c\x43\xd6\xb5\x78\xec\xc3\x54\xa6\xd9\x6f\xb4"
	"\xa9\x80\xd3\xa1\xdb\x42\x8a\x9c\xa4\xc6\xc6\x62\x0a\xc1\x42\x16\x12\x00\xe6\x5e\x6e\x60\x8f\x80\xc6\x3e\xfc\xf6\x15\x9d\x49\x6c"
	"\xb8\x01\x6f\xd6\x9a\x64\xf7\xf2\x23\x3f\x0d\x6f\x9e\xc1\x11\xe5\xde\xab\x5e\xe3\xd8\x21\x81\xe1\xd8\xa7\x85\x1f\x7c\x5b\x0a\x48"
	"\x86\xb3\xdc\x9b\xc7\xbc\x70\x3a\x50\xc2\x59\x63\x78\xec\xe2\x33\x70\xe7\x92\x71\x30\x9a\x77\x3e\xa2\xda\x0f\xf9\x66\xa8\xb0\xdd"
	"\x32\x9c\x1e\xff\xe2\x02\xe2\xe8\xc3\x4d\xa4\xe0\xd5\x93\xfd\x7b\x77\xc4\x54\xde\xd2\x1f\xcd\x05\x6c\xcf\x69\x9a\x43\x67\xda\x4e"
	"\xb8\x5d\x5f\x10\x9f\xb4\x7e\x62\xb2\x99\xcf\xb5\x31\x8c\x87\x26\x63\x03\x81\x2f\x44\x24\xd6\x88\xae\x70\xca\x79\x60\xc8\x88\x7c"
	"\x74\x3e\x54\xb9\x22\xd0\x8f\x27\xd4\xa8\x6d\x8b\xfd\x5f\x63\xc2\xd0\x0b\x77\xd3\x14\x47\x20\xcf\x31\x80\x6f\xbb\x76\x09\x38\x6f"
	"\x86\x18\xdc\xe0\xa3\x16\xd0\xba\xfa\xcd\xb1\x0d\x86\x70\x89\xc9\x96\x42\x2d\x96\x2c\x4f\xfb\x68\x10\x81\x7a\x24\xcb\x4b\x22\x37"
	"\x31\xf0\x56\xe7\x49\x40\x9c\xeb\xe8\x5f\x99\x5d\x02\xf8\x41\x13\x93\x6e\x13\xf4\x2f\xb5\x2e\x12\x06\x05\xeb\xb0\xe0\x61\xf5\x6a"
	"\xaa\xa2\x87\x76\x8a\xfd\x4a\x2a\x80\x95\xd0\x23\xcd\x20\x13\x09\x39\xd6\x34\x5b\x95\x45\x80\x22\xac\x2b\xab\x1a\x08\x8c\x15\x37"
	"\x8c\x69\x79\xe5\x8b\x3b\x12\x11\x44\x8e\x43\x50\xdd\x64\xe1\x5f\xf0\xc8\x18\xc6\x63\xed\x33\xf0\xda\x08\x30\x0b\xca\x78\x51\xa6"
	"\xe6\xdb\xe1\x8b\xa1\xd5\xf4\x9b\x0d\xd1\x59\xf8\x25\xc6\x2c\x5b\x27\x79\x55\x26\xaf\xbc\x80\x73\x04\x75\x47\x05\xc3\x88\x1b\x0e"
	"\x2d\xe0\x35\x39\x15\xcc\x19\xc6\x33\x42\xe2\x9a\x42\x07\xae\x5e\xc6\x30\x88\x48\xf3\x39\x3d\x90\xdf\x6c\x14\x7f\x92\x97\x82\xf8"
	"\x22\xa1\x68\xae\x7f\x4f\x52\x30\x42\x1a\xca\xf7\x9a\x2f\x7f\x77\xea\x3e\x16\x11\x5c\x41\x8a\xf3\xdb\xd1\x07\x8d\x45\x11\x50\xa6"
	"\x5a\x04\x46\xcb\x9f\x1e\xd9\x5b\xd8\x4d\xbd\x2c\xd8\x0f\xb8\x47\x02\xf2\x1a\x5b\xc6\x5f\xb1\x83\x71\xe8\x55\xae\x46\xc8\x30\xbe"
	"\xb7\x9b\xde\x7f\x39\x05\x93\x41\x05\xd1\xcb\x2c\x91\x13\xd3\xfa\xb9\x26\x53\x2d\xdf\x69\xc8\x3b\x4d\x6b\x58\xe5\xb1\x99\x2f\x36"
	"\xe1\xd4\xe1\x8e\xcd\x41\xfc\x7a\x27\xb7\x3f\x1d\xf4\xd0\x41\x1c\xcd\x23\xf7\x90\xab\xf2\x38\x25\x8e\x5e\x72\x40\x86\x6b\x7c\x27"
	"\x78\xc0\xb7\x02\xd3\x55\x31\x8c\x7c\xfe\x4e\x8a\x2c\xfe\x14\x4f\x4b\x03\x50\xa8\xb5\xc3\x73\x00\x11\xe0\x81\x2b\xfe\xc0\xc2\x16"
	"\x02\x9a\x35\xee\x1a\xf2\xed\xc2\x06\xc8\x5d\xa6\x47\x0c\x66\x5b\x15\xf8\xe6\x4c\xff\xfe\x11\x47\x83\x58\x8f\xf1\xbb\x61\xba\xeb"
	"\xe0\xa7\x74\x62\x5b\xbe\xf4\x8a\xa5\x5d\x47\x71\x79\xcd\xbe\x98\xf0\x7c\x26\xf0\xcf\x1c\xe3\x9b\x4c\xda\xfb\xdc\x7e\xe8\xf7\x68"
	"\xdf\xf8\x28\x8c\x39\xf5\x0b\x22\xe8\x69\x00\xde\x2a\x5a\xe1\xe5\x3e\x7d\xd3\x12\x1f\x93\x0d\x1e\x4d\xaf\xa2\x13\xdd\xbe\xf7\x13"
	"\x8c\xd2\xd3\x44\x13\x5b\x63\xba\x9e\xe0\x62\x21\x0f\x30\x84\x77\x1d\x9a\xcf\x55\x1b\x90\x43\x6c\x66\xa8\xe3\x0d\xdd\x3f\x09\x42"
	"\x5d\xfe\x0d\xf2\x5b\xaf\x21\xdd\xe2\x77\xa3\xe5\x77\xc8\xce\x34\x6b\xe7\xa4\xd5\xd1\x4a\x40\xd1\x8b\xc0\xa1\x7e\x9d\x78\xe0\x03"
	"\xf4\x7c\x8a\x64\xc1\x0c\xd0\x1b\x0d\x98\xcd\x2a\x1b\x1d\x5a\x9b\x8d\x31\x62\xd7\xa2\xfe\x45\xc4\x44\x24\x47\xa4\x7a\xbd\x3a\x14"
	"\xc2\x07\x4f\x72\x4d\xf3\xe1\xd8\x08\xca\xcc\x99\x64\x8f\x30\x9c\x6f\x9f\x5c\x61\xdc\x8d\x28\x97\x29\x21\x9f\x10\x26\xb0\xd1\x97"
	"\xa3\x9a\x25\xc8\x68\x1f\xad\x9a\x82\x62\x77\x6d\x1d\x7c\x3d\x71\x57\xf1\x15\x4b\x03\x6c\x45\x8f\xbb\xbf\x37\x79\x98\x5b\x94\x67"
	"\x08\x1e\xc0\xd4\x01\x31\x54\x73\x05\x35\xc6\x8e\x39\x30\xab\x1c\x16\x2a\xc3\xd1\x32\x4e\x27\x53\x52\x07\xd7\x91\xb4\x0e\x42\xb7"
	"\x8b\xca\xe8\x64\xfe\x50\x6e\xd1\x92\x96\x57\x71\x8e\x84\x0b\x58\x99\x27\xa8\x40\x64\x10\xf0\x23\x30\x69\xe4\x5d\x0b\xf3\x89\x8c"
	"\xf3\xed\x1e\xfb\x49\x4a\x96\x4b\x10\x19\xb8\x26\x0b\xe4\x2e\xfc\x0b\x7d\xdc\x02\x03\x6e\x3a\xdf\x0f\x9b\xa0\x81\xb6\xff\x7f\xdc"
	"\x2e\x5e\x7d\xd2\xa1\x58\xd4\x6f\x0a\x2e\x88\x88\x7b\x63\x16\x67\x64\x55\xe6\x92\x38\x71\x17\xfb\x18\xda\x06\x3e\x0b\xb3\xab\x2d"
	"\x5d\x88\xc4\x34\x11\xef\xe8\x62\x85\x93\x22\x88\x20\x48\x7b\x92\xfe\x22\xd6\x18\xc7\x5a\xbe\xf7\xb2\x1e\x06\x6c\x08\x56\xdf\x01"
	"\xb1\x1a\x2a\x53\x02\x9e\xa8\xb0\xc9\x28\x11\x14\x96\x91\x77\xde\x65\xdb\x31\x02\x9a\x02\xd1\xcd\x9e\x57\x2c\x64\xcc\xeb\x48\xd3"
	"\xc9\xb0\x48\xfc\xab\xbd\x80\x2e\x86\x95\x08\xf8\xc7\xcf\x63\xae\xe9\x3f\x87\x14\x7c\x64\x44\x07\x41\x7d\xdc\xd4\x76\x4e\xce\x71"
	"\xbb\x9e\xc4\x9b\xca\x07\xf0\xbe\xc9\x81\x25\x53\x8a\x70\xb3\xc5\x59\xb9\x4d\xd2\x02\xec\x88\x35\x44\x33\x2c\xe1\xcc\x17\xc0\x49"
	"\x04\xa3\x76\x8e\x5a\x7c\x43\x2a\x0c\xe4\x14\x5f\xbf\x80\xda\xc5\xa9\x83\xac\x68\x8b\xd8\x8b\x16\xb1\x8b\x4f\x4b\xf1\xe2\x63\x94"
	"\x98\xcf\xcf\x69\xb1\xfd\x7b\xfe\x52\xbc\xa3\xe0\x5f\xda\x44\xeb\x68\x9e\xed\x3b\xdf\x80\x2c\x63\xa4\x91\xff\x57\xaf\x0c\x7c\xe1"
	"\xc2\xb1\xc6\x4a\x14\x7f\x13\xea\xcd\xcd\xa2\x64\xc0\xd8\x9e\x16\x71\x01\x74\xd0\xd2\x41\x3f\x40\x98\x50\x9b\xa2\xb7\x9d\xbe\xa5"
	"\xfa\x33\x31\x77\x4f\x2d\x71\xd2\xa8\x2e\x82\xcd\xdb\x98\x49\xf5\x76\xce\x41\x26\x8d\x2d\x87\x72\x0d\xf4\xc7\x50\x26\x85\x4a\x27"
	"\x5d\xe5\x8e\x88\x30\xee\x10\x17\x13\xe0\x13\xfc\x62\x06\xcf\x64\x8b\x70\x57\x79\xfc\xdd\x68\x8a\x3d\x0e\x68\xca\x97\xc5\xbc\xf6"
	"\x07\xeb\xcc\xa6\xac\x22\x08\x0f\x7e\xe4\x8a\x54\xdc\xab\x3b\x31\xfd"
;