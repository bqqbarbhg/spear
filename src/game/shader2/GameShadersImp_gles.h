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
	{ { 5 }, {  }, { 5 }, 35882, 337 },
	{ { 6 }, { 2,1,8,9 }, {  }, 36219, 6392 },
	{ { 6 }, { 2,1,8,9 }, {  }, 42611, 6173 },
	{ { 6 }, { 7,1,8,9 }, {  }, 48784, 6646 },
	{ { 6 }, { 7,1,8,9 }, {  }, 55430, 6427 },
	{ { 7 }, {  }, { 1,2,3,4,6 }, 61857, 1346 },
	{ { 4 }, { 2,3,1,10,11,12 }, {  }, 63203, 7981 },
	{ { 4 }, { 2,3,1,10,11,12 }, {  }, 71184, 7981 },
	{ { 4 }, { 7,3,1,10,11,12 }, {  }, 79165, 8235 },
	{ { 4 }, { 7,3,1,10,11,12 }, {  }, 87400, 8235 },
	{ { 8,9 }, {  }, { 1,7,8,9,10,11 }, 95635, 1556 },
	{ { 4 }, { 2,3,1,4,5,6 }, {  }, 97191, 7940 },
	{ { 4 }, { 2,3,1,4,5,6 }, {  }, 105131, 7940 },
	{ { 4 }, { 7,3,1,4,5,6 }, {  }, 113071, 8194 },
	{ { 4 }, { 7,3,1,4,5,6 }, {  }, 121265, 8194 },
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
	"\x28\xb5\x2f\xfd\xa0\xb3\xf9\x01\x00\x2d\x80\x00\x1a\x75\xec\x15\x2c\xd0\x6e\xc8\x0c\x90\x01\x46\x8c\x4c\x61\xaa\x94\x10\x61\xd9"
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
	"\xbf\xf0\x0b\x77\x8e\x77\x11\xbf\xfb\x53\x95\x2c\xfd\x0c\x4f\x91\xe0\x13\xb4\x46\xb8\x50\x5d\x6d\x77\x5e\xdf\x39\x9a\xe2\xe4\x19"
	"\x0d\x4a\xc6\x03\x14\x1d\xdb\x58\x63\x2d\x3f\x4a\xa6\xa2\x61\x52\x3a\xab\x54\x27\xa5\xb3\x32\xfe\x81\x98\x62\xe3\x63\x38\x1b\x26"
	"\xd4\x4c\x3e\x9c\x04\x08\x30\x98\xe9\x42\x7d\x8d\xd8\x34\xf5\xf1\x3a\xaf\x9b\xc8\xa7\xbe\x73\xd9\x21\xd9\x6f\xd6\x6f\x52\x0c\xbb"
	"\xb5\x3e\xd8\x47\xdf\xc5\xeb\xba\x08\xbf\x3f\x0c\xfb\x17\xf8\xce\xa0\x0e\x80\x73\x5f\xe6\x66\xf4\x8f\x1f\xc6\xe7\x2c\xd7\xf5\xd7"
	"\x5b\x7d\x37\x0b\x53\xdc\x5e\x41\xc0\x16\xaf\x6c\x95\xec\x96\x23\xe1\xdd\x1e\xf2\xba\x1c\xe1\x13\x14\x5d\xc3\x96\x96\x75\x55\xe9"
	"\x86\xaf\xab\xb3\x60\x0e\x03\x9c\x1c\xe7\x1a\xc7\xb5\xb7\x90\xad\x15\xeb\x6a\x14\xb4\xac\x2f\xfb\x85\xfd\x9f\xca\xfd\x43\xb9\x5a"
	"\xf5\x3f\x17\x70\x6a\x54\x3f\xe3\x43\xaa\x9b\x5d\x75\x43\xa8\x9b\x8d\x73\x7d\x51\xfd\xd0\xb4\x26\x11\xde\x6d\x4b\x10\xb1\x5f\x38"
	"\x3d\x36\x7a\xca\x31\x30\x35\xbf\xf1\x2b\x8c\xff\x50\x2e\xf2\x1d\x29\x3f\xea\x9f\x27\x42\x0d\x98\xe9\x05\x4e\xc6\x53\x5e\x3e\x05"
	"\xe7\x02\x77\x1f\xf4\x9b\x5d\x9a\x04\xfd\x06\x84\x6e\xa8\xf3\x77\xb3\x9c\xa4\x4c\x8d\x88\x08\x00\x10\x80\x02\x43\x12\x60\x50\x78"
	"\x68\x30\x93\x0b\xc5\xab\x30\x44\x3e\x13\xc0\xc7\x45\x83\x91\x48\x1c\xce\xc2\x20\x87\x51\x10\x32\x88\x00\x83\x00\x01\x00\x00\x00"
	"\x20\x00\x40\x03\x00\x30\x23\xc2\x02\xcd\x3e\x63\x5b\x1b\xe0\x2c\xc5\xe6\x08\x92\x47\x3b\x59\x97\xda\xea\x16\xfc\x5d\x5e\x8a\x80"
	"\x6b\x46\x8c\x17\x98\xe6\x40\x80\x9d\x8d\x3b\xdc\x4d\xe8\x32\xcc\x9d\x63\x03\xe2\x74\x04\xef\xa8\xcf\x95\xe5\xf5\xe0\x93\x27\x9b"
	"\x48\xf6\xdc\xf4\xca\xb2\x1f\xd0\x52\x64\xce\xcc\xf7\x3d\xc7\xa7\xe3\xe6\xfb\x57\x49\xae\x27\x60\x59\x86\x8f\x50\x43\x84\x92\xb1"
	"\x7b\x67\x13\x2c\xfe\xbb\x3f\x24\x8c\xdb\x3d\x6e\x57\xe7\x7a\x56\x63\x21\x0f\x23\x6a\xdd\x48\x5a\xf6\x94\xf1\x8e\x50\x7a\x45\xb8"
	"\x7d\x9d\x63\x45\x4d\x64\x48\xd7\x4d\x52\x4d\x86\x81\x5d\x3f\x44\x72\xc0\x12\xa7\x81\xfd\x33\x11\x84\xe8\x96\x5b\xac\xd6\x55\xcc"
	"\xaa\xab\x04\x65\x35\x3a\x6d\x16\xd4\xfe\x16\x04\xd1\xaa\x3a\xa8\x2d\xae\x1f\x40\xee\xb7\xfb\xd2\x93\x5d\x80\xdf\xb1\xbd\xf5\x3b"
	"\xaa\x42\x45\x92\x52\xc3\xa2\xcd\x8b\xab\x5f\x57\x88\x92\xfc\xa2\xf3\xf4\x1b\xdf\x31\x92\x17\x65\xe1\xbe\xf8\x59\x98\x97\x2e\x5e"
	"\x3c\x1a\x57\x57\x43\x77\xc4\x17\x09\x96\x9b\x45\x27\x7e\xd1\xc6\xa4\xf5\xcf\xc6\x26\xd4\xfb\x53\x3e\x9b\xd8\x2a\x1e\xd5\xb8\x0c"
	"\x5e\x9f\x4e\xca\x6f\x36\xa8\xa6\x8e\x9d\x40\xd7\xaa\x09\x7d\x4f\xa0\xce\xc9\xd2\x7d\x35\x47\xeb\x4f\xd7\xe0\xc6\x85\x18\x45\x08"
	"\xb5\x50\x27\x89\x3e\xa2\x90\x8a\x10\x71\xd2\x1b\x2b\x40\x5f\x06\x37\x2f\xd4\xa0\xe0\xad\xa6\x6f\x68\xec\x23\xba\xd3\x8c\xf3\x8e"
	"\xc0\x0e\x44\xf3\x7d\x90\xfe\x07\xb4\x94\x19\x3a\x0a\x08\x5e\x4d\x18\xe4\x24\x14\x9a\xdf\x15\xe9\x7f\x4b\x9e\x71\x3a\xf9\xa0\x2f"
	"\x02\xc5\xd9\x19\x80\x46\x2d\x9d\xe1\xdc\x73\x60\x2b\x52\x96\x96\xa8\xbb\xbd\x98\x81\xb5\x2d\x54\x4e\x1f\xe8\x48\x5c\x41\xcd\x68"
	"\x4d\x6b\xa6\x38\x53\xc9\xdf\x6e\xb2\x9d\x84\x91\x41\xd5\x2c\x02\x05\x40\x42\x26\x23\x87\xe6\xbb\x50\x81\x62\xf2\x52\xdd\xf2\xd3"
	"\xe9\x18\xb4\x90\x8d\xa6\x99\x47\x82\xde\x79\x7d\x41\xb2\xab\x9a\x86\xd5\x11\x52\x2e\xa3\x43\x9a\x8b\x5c\x93\x92\x4e\x2b\x80\xa0"
	"\x9e\x17\x9d\xbd\x9a\x98\x1b\xdb\xc7\x6a\xd6\x4f\xdb\x51\x49\x05\x12\xd0\x5b\xb6\xa0\xb5\x77\xd8\x9b\x35\x3b\x74\xb0\x80\x9b\x02"
	"\xda\x98\x37\xc8\xab\xce\xd9\xd4\x98\x0d\x16\xe3\x53\xe4\x1b\xba\x3b\x4f\x41\x75\xad\x11\x39\x7b\x67\x31\xad\xd6\x1d\x9a\x34\x36"
	"\x33\x4e\x60\xd7\x68\x21\x9d\xfd\xeb\xe5\x6c\x3a\xbb\x79\x49\x4e\x53\x5d\x0c\x28\x33\xba\x86\xb8\x7b\xc9\x5b\x53\x03\x40\x27\x0e"
	"\x85\xce\x91\x7c\x70\xd1\x11\xe1\xc6\x8c\x2b\x90\x7a\xa9\x32\xcf\xd9\x00\xd7\x1e\xfc\xa8\x31\x2f\x1f\xa9\x36\x8c\x30\x3a\xa8\xa4"
	"\xd9\x70\xc4\x8b\x25\x9c\xe7\x56\x96\x24\xeb\xd5\xeb\xdf\x30\x92\xf4\x2e\xc4\x4b\x5f\x16\x91\x0c\xb6\x9d\x04\x0e\xef\xd5\x47\x77"
	"\x2a\x9b\xfa\x0a\x74\x51\x8c\xc8\x8c\xb6\xd6\x4a\x91\x4d\x3e\xf7\x9a\x07\xbf\x68\x84\x20\x4b\xd4\xb3\x6c\xb1\xca\x62\x22\x83\x95"
	"\x44\x75\x85\x70\xee\x4a\x13\x5e\xb9\x22\xd0\x50\x42\xf3\xe1\xf6\x93\x9a\xbf\x77\xf1\xbc\xa9\xa5\x8e\x43\xe7\xf9\xba\x5c\xd4\x01"
	"\x6c\x75\x3b\x94\x03\x9c\xda\xd0\xc0\x7c\x4c\x12\x20\x81\xaf\x9f\x8b\xc2\x93\xdd\x99\x8f\xd9\xe3\xbb\x8f\xf0\x33\x1f\x57\xbb\x55"
	"\x98\xb5\x66\x0d\xed\x72\x8e\x99\x87\x7b\x10\xd9\xc0\xa8\xab\xb1\x22\x49\xc2\xc9\x95\xd3\xe2\x70\xe1\xe8\xf6\xd2\x26\xe6\x45\x04"
	"\x27\x36\x54\x66\x94\x51\xcc\x37\xb1\xa2\x66\xc3\x4a\x90\x7b\xab\xa7\xd1\xdc\x8e\xd4\xbf\x9c\xb9\x84\x01\x04\x80\x68\x40\x74\xa4"
	"\x21\x3c\x63\x47\xe3\xf4\xe4\x38\x12\xe5\xa8\x26\x2f\xc2\x03\x53\x2a\x2d\xb8\x5e\x20\x9d\xfd\xc6\xf0\x3a\xac\x5f\xa8\x0b\x44\xa0"
	"\x76\x21\x94\xe6\xff\xb2\x5b\x9c\x81\xe7\x02\x74\xd5\x60\x04\x4a\x7d\x37\x5c\x7a\xd5\x9c\x06\x51\x64\xa0\xd7\x01\x25\x63\xf8\x0b"
	"\x47\xab\x05\x1a\x74\x02\xa8\x67\x04\x88\x41\x04\xb4\x89\x02\xb8\x4c\x5d\xf2\x95\x9f\x15\x2b\x06\x50\xfa\xeb\x00\x5a\xf0\x97\x0b"
	"\x5c\x48\x92\x94\x1b\xdd\x57\x7d\x37\x97\xd8\x48\x35\x12\xc0\x2d\x94\x78\xd6\x42\xa0\xb5\xd5\xdf\xf2\xbd\xff\x4c\x04\x4f\x3d\x0e"
	"\xff\x44\x7b\x90\x38\x72\x7c\x01\xa4\x32\xec\x71\x5d\xad\xc2\xcf\x73\x31\x30\xd8\x0d\x30\x4c\xa4\x00\x22\x9f\x16\x46\xb1\x80\x08"
	"\xc0\x1f\x97\xfb\xc2\x96\x0b\x80\xdf\x4d\x1f\x09\xbf\x5d\x82\x01\xea\xc2\x76\x19\x10\xf7\x0a\x13\xae\x73\x05\x4d\xe0\x17\x88\x17"
	"\x9c\x04\x14\x42\xbc\xad\xc7\xa4\x25\x0e\x06\x40\x44\xaf\x46\xd9\x25\xdb\xb1\x93\x97\x55\x58\x9c\xa4\xef\xc8\x24\x3e\x4e\x21\xbc"
	"\xc3\xc4\x82\x4f\x40\x3f\xb3\x56\xd6\xb4\xf7\xa6\x7e\x76\x33\xb3\x64\xbd\x01\xa4\xbb\xaf\xb4\x70\x2e\xd7\xd9\xd6\x34\x38\xee\x5c"
	"\x1e\x2d\x79\xb2\xf2\x1c\x72\xe4\x94\x59\x26\xbf\x05\x12\xcd\x85\xf3\x1e\x7c\xea\x5a\x08\xe2\x02\x20\x50\x05\x98\x4c\x8f\xff\x18"
	"\xa7\xef\xc7\x8f\xda\x8c\x1b\xfc\x7f\xb8\xc9\x15\x43\x77\x22\x2e\x46\xb0\xd6\x18\x00\x7e\xf9\x6b\xec\x05\x4f\xdd\xd4\x36\xd3\x97"
	"\xc6\xa4\x52\x5e\x5e\x41\x89\xf2\x08\xbf\xfa\x06\xd5\x91\x44\xfa\x94\xbb\x6f\x3c\x32\xab\x82\x1d\x3c\xc5\x01\x00\x23\x2f\x74\x33"
	"\x8f\x7d\x37\xcb\xee\x8d\x2c\x34\x4f\x03\xf1\xfa\x2f\x2c\xfd\x26\x2d\x33\xb5\xd2\x14\x23\x05\xd0\x3d\x47\xaf\x8e\x6f\x82\x76\x82"
	"\x6a\x5f\x2d\x24\x00\x6d\x27\x75\x85\x01\xaa\x29\x4e\x32\x00\x9f\x34\x9c\x98\x61\x96\x1e\x57\x1d\xe4\x07\xaf\x85\x21\x40\x1b\x10"
	"\x06\x09\x7c\xd9\x8a\x94\xfb\x6c\x18\x0f\xea\x2b\xbe\x6f\x71\x02\xc0\x69\x3a\x24\x5b\x1a\x00\x82\x2c\x91\x1c\x8a\x35\xe5\xc6\x5d"
	"\xa8\xe0\xf7\x19\x75\x93\xf3\x73\x6e\x8a\x27\xce\xc0\x16\xaf\x47\x14\xb4\x7a\xbb\x21\x45\xcb\x26\x0c\x21\xf2\xb0\x2c\x30\x7f\x56"
	"\x55\x6f\xc7\xbe\xa7\x1c\x22\x08\xe7\x84\xa2\xef\x5d\x91\x41\xeb\x7a\x23\x74\x87\xec\x07\xf4\x0c\x76\xe1\x81\x84\x12\xb9\xd4\xbd"
	"\x66\x85\x53\xde\xeb\xd2\x73\x22\xed\x57\x4a\x4e\x2e\x7b\x93\x64\x68\xc6\xba\x6e\x73\xde\xd3\x62\x2a\xb3\xb6\x48\xc7\x67\x70\xb8"
	"\x02\xb5\xe1\xab\xee\x0b\x47\x1a\xd4\x2b\xcf\xa9\x80\xa6\x65\xbb\x3c\xb9\xdd\x1c\x0a\xb2\xd6\xfa\x2a\x95\xf6\xb6\x0a\x6b\x8c\x18"
	"\xac\xad\x15\x02\xf3\x0e\x27\x9b\xde\xa0\x8c\x56\x98\xc2\x22\xa5\x32\xd9\xe1\x6b\xeb\xb3\x38\x00\x81\xef\x69\x12\x12\x57\x9c\x13"
	"\xde\xcc\x32\x7a\x83\x25\x03\xda\x9c\x8a\x40\xc6\xc2\xde\x28\xa8\x7f\x69\xf8\x69\x76\x0e\xaf\xeb\xe0\xb1\x0e\x53\x44\x43\x65\xa3"
	"\xb6\x03\xde\x9e\xbe\x03\x29\x26\x5d\x7d\x9c\x34\x7b\x83\x74\x2c\x98\x00\xbc\xcd\xd8\x00\x3d\x02\x65\xfb\xc0\xda\xaf\x5a\x33\xb1"
	"\x81\xc0\x49\xac\x5f\xd6\x2e\x93\xd4\x45\x37\x28\xe1\xf6\x91\xdf\xbb\xfe\x3a\x46\x3d\x8f\xd4\xe2\x88\xaa\x05\x7d\xcf\x64\x2d\xf4"
	"\xee\xaf\xfc\x3d\x67\xb6\xb0\xa9\xb2\x94\x33\x71\xf7\x60\xc5\x2f\xe4\xee\x7d\x62\xe1\x3a\xd8\x14\x44\x15\x10\xf2\x15\xd1\x65\xbb"
	"\x96\x37\x63\xfe\x15\x17\xc4\xdf\x87\x7d\x52\xd8\xa6\xd7\xfe\x6d\xf3\x54\x2a\xe3\xca\x1f\x04\x05\x99\xcf\x69\x5a\x46\xe7\xcb\x4f"
	"\xb8\x1d\xe9\x8c\x4f\xd1\xfd\x64\xbb\x19\x73\xb5\x21\x11\x0f\x24\x62\xe3\xb5\x2f\xb2\x62\xac\x41\x96\xb2\x98\x52\x25\xfc\x25\xf9"
	"\xc0\x1a\xe5\x6d\x45\xf8\xc6\x27\x76\x5e\xdb\x00\x7b\xe9\x62\xc2\x70\xba\x72\xd3\x5b\x46\xe0\x8a\x18\x6c\x6b\x5f\xb0\x04\xb9\xef"
	"\x7f\x0c\xca\xe0\x6f\x16\x78\xa7\xfa\x8a\xb1\x5d\x1b\xa1\xc3\xec\x80\x42\x4a\x6c\x2e\x4f\x6d\x34\xcc\x83\x9a\x27\x0b\xca\x64\x2b"
	"\x51\x3d\xa7\xa1\x49\x80\x99\x6b\xa3\xaf\x4c\x2f\x01\x29\xa0\x75\x90\xce\x1d\xc4\x8d\x5a\x6f\x08\x23\x0b\x2b\x45\xf0\xd0\xfc\x6a"
	"\xe5\xa2\xb7\x5d\x8a\x7d\x52\xda\x8f\x81\xac\xf3\xcf\x80\x89\xd0\x0e\xac\x19\xae\x80\x45\xa8\x3f\xac\x10\x2b\x08\x64\xad\x15\xfd"
	"\x86\x73\x79\xe8\x88\x86\x8b\x38\xb7\x04\xc1\x49\x5d\x9b\x20\x2d\x98\x62\x8c\xe4\x58\x10\x1b\xb4\x6d\xc4\x80\x75\x62\xf4\x28\x13"
	"\xf4\xd4\xed\xff\xd5\xea\xf8\x9b\xac\x28\x7d\x7e\xc1\x6b\x02\x5b\xee\x79\xbd\x09\x6b\xde\x52\x1c\xdd\xdf\x0c\x0f\x3d\xe2\x6b\x87"
	"\x96\xc5\x9a\x30\x16\x60\x0e\xce\x19\xe1\x67\x4d\x01\x81\xab\x43\xa6\x01\x44\x48\xfe\x03\x0d\x54\x45\x10\x2c\x2d\x9a\x4b\x4c\xc5"
	"\x3a\x41\x2e\xc1\x7e\x83\x50\x70\x8b\x34\x10\xd2\x9a\x07\x7b\x57\xcb\x3e\xb2\x11\x9a\x23\x89\xf8\xed\xcc\x07\xcd\xa0\x2c\xb9\x4c"
	"\x15\x04\x81\xe5\x27\x1e\x17\x2d\xd0\xa7\xab\x8c\xe1\xc7\x8e\xa3\xf1\xfc\x5e\x5b\xd9\xdf\x85\x82\x81\xf5\xd2\x5d\x4d\x82\x1b\x77"
	"\xcb\xcd\xf6\x7f\x06\x00\xd3\xa3\x82\xc2\xcb\xec\xd2\x89\x59\x6b\x48\xbb\xb8\xd6\xff\xe4\xe7\x48\x4c\x25\x48\x68\x31\x3c\x19\x63"
	"\xe5\x14\x62\xb3\xe3\x4c\x62\x4d\x22\x17\x8a\x0e\x1c\xd0\x22\x9e\x98\xc7\xf6\xe0\x52\xc5\x3c\xc5\xaf\x37\x49\x36\x9b\x60\x7d\x15"
	"\xbc\xd6\xbb\x12\xe9\x5b\x91\x4a\xde\xfe\x56\x82\x38\xce\x54\xd1\xd2\xa6\x4f\x38\xb7\xc3\x54\xca\xa4\xe8\x81\x2d\xfe\xe0\xd3\x16"
	"\x22\xce\x1a\xbb\x1b\xf2\x7f\xe1\x93\xc8\xbd\xa6\xc7\x40\x75\x5b\x26\xf8\x76\x16\xed\xfe\xa9\x47\x9b\x58\xfb\xf8\x9f\x2d\x5d\x8f"
	"\xe0\xa7\x78\x62\xdb\x12\x7a\xc5\x16\xae\xc3\x75\x29\xc1\xbe\x9c\xf0\x0a\x24\x30\xc2\x1c\x77\x82\x5f\x3a\xe6\xdc\x77\x28\xa6\x68"
	"\x83\xfc\xa8\xd4\x9c\xe3\x0b\x22\xe9\xc9\x02\xde\x0a\x99\xf0\xa5\xd5\xfe\xb1\x09\x9a\x98\x0d\x4c\x27\x3f\xcc\x89\x5e\x92\xf7\x03"
	"\x8c\xd3\x53\x41\x8b\x0d\x04\x5d\x8f\xb8\x5d\xe8\x12\x31\x04\xc3\x1c\x12\x84\x55\x1b\x90\x43\x6c\x66\xa8\xe0\x57\xdf\x43\x65\xa1"
	"\x8e\xff\x8d\xfa\xad\xfd\x73\x7e\xf6\x87\xb4\x95\x3b\xb4\x60\x9a\x02\xe4\xe4\xcb\xd0\x4a\xe6\xe9\x65\xe2\xe1\x79\x31\x3c\x54\x19"
	"\xf8\x7e\x55\xb2\x62\x86\xe9\x11\x0e\xf8\x64\x55\x8d\x0e\xd0\x66\xcd\x18\x61\x4b\x91\xbb\x88\x82\x08\x12\x21\x5a\x3d\xc8\x0b\x0a"
	"\x9a\xc1\x33\xaf\xa6\x26\xe0\x18\xf3\x65\xa7\xcb\x93\x67\x3a\x74\x60\x02\x46\x61\x5c\x0c\x29\x99\x69\x21\x45\xd1\x06\xb0\x55\xcd"
	"\x55\x2a\x34\xc8\x50\x1f\xad\x94\x82\x9c\x5d\xe9\x0e\x0e\xbc\x5f\x57\x0e\x8a\x75\x12\xb6\x3e\xc6\x52\xb5\x36\x00\x4c\xe3\x95\xea"
	"\x00\xcf\x18\xea\xfc\x18\xda\x71\x82\x0a\x6e\x87\x12\x36\x6b\x1c\x16\x08\xc2\x31\x28\x4e\xed\x52\x0f\x07\x37\x97\x54\xfc\xd2\xdc"
	"\x65\xf3\x75\xd5\xfc\x31\xa7\xc8\xdb\xcb\x57\x70\xce\xc1\xc5\x96\x96\x53\x54\xf0\x17\x08\xd2\x11\x04\x32\xb2\x5d\x8b\x8e\x0b\xc8"
	"\x41\x8d\x1d\xf3\x19\x56\xfd\xa5\xf1\x0c\x0a\x4b\x16\x53\x17\xfe\x9a\xbe\xf9\x02\x06\x88\x3b\x5f\xb6\x89\x9f\x31\xdb\xff\xec\xdd"
	"\x16\xaf\x1e\xe9\x80\x5a\x24\x37\x0a\x17\x43\xc4\x14\x62\xf0\x67\xd4\x26\xb3\x91\x38\xb6\x8b\xbd\xcf\x9a\xf6\x3d\xcb\xd0\xd5\x52"
	"\x2e\x3c\x62\x8a\x86\x17\x42\xb1\xbc\x49\xed\x44\x4c\x8a\x3d\x57\x7f\xd1\x6b\x8c\x62\x2d\xff\x7b\x31\xf3\x13\x77\x06\xc3\xc3\x10"
	"\x99\x30\x99\x21\x81\xa7\xb2\x00\x64\x7b\x0b\x7b\xcb\x08\x3a\xaf\x48\xdb\x14\x00\x4d\xae\xe8\x66\xce\x63\x18\x3a\x5e\x74\xa4\x62"
	"\x32\xac\x24\x0e\xd3\x5e\x80\x17\xa7\x26\x84\xfd\x83\xe0\x21\x92\x74\xcc\x21\x18\x19\x99\xa2\x83\x60\x3f\x6e\x38\xa9\x4e\x46\x5d"
	"\xc5\x27\xa1\xa6\xcc\x02\x90\x6c\x22\xe7\x72\x8c\x22\x94\x75\x71\x56\xb6\x26\x7f\x00\x76\xd8\x1a\xf2\x07\x8b\x67\xf2\xc5\x70\x12"
	"\xc1\x50\x34\x73\x0a\xbe\x61\x12\x06\x0e\x8a\x6f\xd1\x40\xca\x82\xd6\xc1\x61\xda\x29\xdc\xcb\x96\x38\xc6\xff\xa2\xb8\xe4\xe3\x69"
	"\x1c\xd9\xc7\xb1\x38\xc1\xfb\xbf\x74\xd4\xa3\x10\x67\x40\x62\xe1\x28\x0f\xed\x08\xda\xf4\xac\x31\xaf\x41\x11\x9c\xaf\x0b\x8e\xf9"
	"\xbc\x5b\x7b\x33\x4a\x68\xd3\x78\x23\xc1\xea\x74\x8c\xf7\xde\x16\x25\x80\xf7\x7a\x7a\xca\x9f\x5a\x27\xc4\xa9\xa4\xea\xca\x9f\x07"
	"\x7e\x23\xde\x1e\x9c\x45\x4b\xda\x35\x0a\x66\x63\x3f\x26\xeb\xa2\xcf\xd5\x67\x0a\x7d\x37\x53\xaa\x79\xfb\xef\x14\xe1\x42\x56\x92"
	"\x1a\x2f\xb4\x1c\x3e\x3b\xd8\x7e\x02\x21\x41\x12\x18\xbf\x9e\x42\x11\x1e\x5f\xde\x85\x1c\x41\x6b\x8f\x97\x9a\x32\x74\x31\xd5\xfe"
	"\x60\xa1\xb5\x29\xf0\x28\x5f\x8b\xe1\x5c\x91\xac\xf7\x4a\x69\x46\x3f"
;