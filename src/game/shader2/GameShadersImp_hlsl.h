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
	{ { 1 }, {  }, { 1,2,3,4 }, 0, 1030 },
	{ { 2 }, { 1,2,3,4,5,6 }, {  }, 1030, 8938 },
	{ { 2 }, { 1,2,3,4,5,6 }, {  }, 9968, 8938 },
	{ { 2 }, { 7,2,3,4,5,6 }, {  }, 18906, 9173 },
	{ { 2 }, { 7,2,3,4,5,6 }, {  }, 28079, 9173 },
	{ { 3 }, {  }, { 5 }, 37252, 477 },
	{ { 4 }, { 1,3,8,9 }, {  }, 37729, 6972 },
	{ { 4 }, { 1,3,8,9 }, {  }, 44701, 6755 },
	{ { 4 }, { 7,3,8,9 }, {  }, 51456, 7207 },
	{ { 4 }, { 7,3,8,9 }, {  }, 58663, 6990 },
	{ { 5 }, {  }, { 1,2,3,4,6 }, 65653, 1480 },
	{ { 2 }, { 1,2,3,10,11,12 }, {  }, 67133, 8985 },
	{ { 2 }, { 1,2,3,10,11,12 }, {  }, 76118, 8985 },
	{ { 2 }, { 7,2,3,10,11,12 }, {  }, 85103, 9220 },
	{ { 2 }, { 7,2,3,10,11,12 }, {  }, 94323, 9220 },
	{ { 6,7 }, {  }, { 1,7,8,9,10,11 }, 103543, 1825 },
	{ { 2 }, { 1,2,3,4,5,6 }, {  }, 105368, 8938 },
	{ { 2 }, { 1,2,3,4,5,6 }, {  }, 114306, 8938 },
	{ { 2 }, { 7,2,3,4,5,6 }, {  }, 123244, 9173 },
	{ { 2 }, { 7,2,3,4,5,6 }, {  }, 132417, 9173 },
};

const SpUniformBlockInfo spUniformBlocks[] = {
	{ }, // Null uniform block
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
	{ "shadowGrid3D", (uint32_t)SG_IMAGETYPE_3D },
	{ "envmap", (uint32_t)SG_IMAGETYPE_CUBE },
	{ "diffuseEnvmapAtlas", (uint32_t)SG_IMAGETYPE_3D },
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
	"\x28\xb5\x2f\xfd\xa0\x16\x29\x02\x00\x54\x7a\x00\x0a\x70\xf4\x14\x2c\xb0\x8e\xac\x0e\xa8\x54\xb0\x2d\x89\x2c\x25\x3d\x35\x07\xf9"
	"\x0f\x94\xea\x83\x8c\xc5\x0a\x5c\x02\x79\x83\x5b\x3d\x09\xae\xe4\xc9\x6e\x6c\x0b\xec\xfe\xa0\xff\xc0\x3d\xaa\xb9\x02\x45\x01\x3f"
	"\x01\x44\x01\x9f\xa2\xe6\xa1\x90\xc9\x6c\x45\xa6\x72\x81\x90\x50\x39\x22\xd0\x4d\x25\xfb\x66\x78\xc3\xe8\x82\x8a\xa2\xb9\x6a\x34"
	"\x23\x23\x46\xb7\xcb\x65\xd3\x11\x08\xbd\x2b\x36\xd9\x44\xe3\xee\x6d\xf5\x32\x0a\x64\x59\x14\x34\xd5\xaa\x8a\xa2\x30\x56\x31\x96"
	"\xce\x39\x35\xb5\xdb\x00\x82\xaa\x39\x8d\x26\xc3\xa4\xdc\xa8\xd6\xdd\x5a\x31\xdc\x83\xde\xf4\xa5\x6e\xba\x2b\xb9\xda\x4d\xa3\x15"
	"\x46\x04\x8a\x5b\x42\x61\x59\x4e\xc0\xc1\x18\xad\xd0\x77\x20\x20\xa1\x6a\xe6\xb6\x57\x29\x62\xc6\x16\x67\x2e\xf8\xd5\xae\x9b\x4c"
	"\x29\x00\x30\x2e\xdc\xcc\xed\xda\x71\x1d\xf7\xe9\xd1\x1a\xe7\xcc\xa3\x73\x7e\xed\xfb\xd4\x4e\xf6\x2b\x02\x6d\xdb\x2f\xb8\x60\xec"
	"\xf8\xe9\x76\x3e\x0f\x42\x13\x14\xa1\xfc\x04\x9f\xc7\x72\xda\xaf\x6f\xa7\xfe\x33\x22\x9b\xc3\x97\x4d\xdf\xec\xb2\xe9\xe7\x66\x54"
	"\xdb\x32\xfb\x80\x29\x38\x11\xcc\x66\xec\x00\x53\xd8\x8a\x61\xed\xaf\xbc\x67\x3c\x81\x6a\x17\x0e\xce\x50\xc4\x3c\xb9\x6e\x9d\x6d"
	"\x6a\xfe\x80\x6b\x19\x6b\xf4\xa5\x99\x22\x93\x69\xf4\x7e\x92\xe7\xaa\xd1\x70\x98\xcc\xc5\x52\xa1\x78\x75\xe7\xda\x5d\x36\x5d\x75"
	"\x9f\x54\x86\xe9\xba\xcd\xcd\xbe\x4c\x2d\x0c\x46\xe4\x02\x23\x42\x75\xdc\x85\x7e\xce\x7d\xeb\xe6\xd2\x7d\xa1\xaf\xa4\xa5\x5a\xc6"
	"\x37\x12\x25\x13\x0c\x18\x3c\x3c\x40\x40\xf1\x01\xbf\x38\xdb\x99\xc6\xdb\x85\x9e\xda\x14\x5b\xcd\x82\x61\xe1\x1a\x93\xe5\xbd\x0d"
	"\x5c\x28\x95\xef\x0c\x97\xb7\x0e\x8f\xd0\xc1\x10\x61\x74\xf0\xa0\x84\xf7\x44\x1e\x94\x58\x38\x3c\xd8\x71\x91\xee\x22\x98\x1f\xf6"
	"\x8e\xf4\xcf\x83\xd1\x3b\x1e\xa4\x7c\x4e\x0e\x28\x80\xa6\x18\x11\x61\xb1\x40\x11\x9f\x2c\x96\x4b\x9f\x44\x7c\xb2\x44\x8b\x05\x34"
	"\xf9\x24\xf1\xd4\x4d\x33\xb7\xa9\xfe\x15\x69\x1c\x92\x2e\x61\x7c\x28\x77\xeb\xde\xf4\x0e\x67\x5c\x58\xc0\x07\x83\x05\xa3\xf4\xaf"
	"\xe3\x41\xe8\xf9\xf4\xd8\xda\xd5\x09\x5c\x48\x11\x96\x31\x7e\xf0\x2a\xc5\x15\xe0\x1e\xbc\x4e\xd1\xec\xc0\xb4\xdc\x02\xf9\x40\xe0"
	"\xd7\x64\xd1\xf5\x0a\x95\xfc\x08\xe3\x63\x39\x5d\x33\xc4\x70\x6a\x7a\x36\xae\x50\xd9\x3a\xa3\xbc\x71\x9d\x6e\x5b\x31\x8e\xe6\x32"
	"\x8c\x4e\x64\x9b\x2c\x33\xb0\x8c\x88\x04\x8a\x9a\x53\xca\x88\x84\x78\xe9\xd7\x23\xdd\x14\x7b\x55\x35\xc5\x7c\xdf\xec\x93\x04\x85"
	"\x52\x37\xe5\x2c\xed\xea\x19\x20\x20\x2c\x96\x29\x5e\x87\x38\xf5\xe9\x53\xe6\x54\xa6\x96\xcb\xd4\x4d\x5f\x20\x24\x13\x07\x4e\x0d"
	"\x60\x78\x81\x3b\x40\x2c\xca\x72\x4c\x88\x25\x2b\x09\xa9\x28\xc2\xa9\xe1\x5b\x31\xe9\xba\xf4\xc6\x23\x22\x7e\x5d\x32\x99\x5c\x9f"
	"\x78\x9b\x8d\xf4\x47\x7f\x76\xae\xa0\x1f\xfd\xba\xc5\xdb\x6c\x26\xad\xd8\x2c\x8d\xb0\x7c\x9a\x6c\x8c\x49\xfc\x41\x82\x04\x17\x89"
	"\xe5\x90\x17\x55\xde\xf3\xe8\x50\xd4\xbc\x71\xe8\xb9\xf4\xa8\x99\xb3\xf2\x1e\xdd\xfb\xf5\xe9\x35\x1e\xa1\x7c\x36\x8d\xda\xcc\xaf"
	"\x7b\x0c\x28\x62\x63\xcc\x12\xa7\x5c\xa7\xa0\xe9\x82\xea\xba\x81\x4d\x35\x68\xba\x0e\xe1\x79\xe3\xd1\xa7\xe7\x91\xd8\x54\xfb\xac"
	"\x9e\x7d\xb3\x5b\x17\x50\x25\x65\xc3\x03\x02\x99\x30\x7a\x55\x04\xf2\x22\x50\x77\x89\xe1\x5d\xa7\xf1\xe6\xd5\x8d\x6a\xf5\xe2\xfd"
	"\x6a\x7a\xc8\x59\xc9\xd8\x30\x29\x39\xa5\x47\x83\xf5\x80\xcc\x06\xc7\xa3\xa1\x82\xc0\x18\x3d\xe2\x83\x82\x40\xa8\x86\x86\x08\xa3"
	"\x55\xc5\x6a\x82\x29\x30\x9b\xac\xa1\xce\xf7\x09\xf2\x60\xd7\x9d\x8a\x87\xaa\xc9\x7b\x38\x78\x8b\x2c\x2b\xc2\xaa\x6d\x9d\x10\x38"
	"\xe5\x82\x01\x3d\x22\x25\xa9\x18\x09\xdc\xf3\x3c\xcc\x26\x57\x30\x45\xa6\xf5\x66\xf6\x91\x29\x46\xd1\xa5\x47\x9f\x26\x3e\x5d\xb7"
	"\x98\x2c\x42\x0b\xc3\x93\x8a\xc6\x6d\x7d\x71\xa6\x03\x27\xa2\xce\x07\x27\xa2\x4f\x9a\x98\x88\x3c\x38\xb1\x38\xd5\x78\x76\xa6\x5a"
	"\x4d\xcd\xbf\x3e\x71\x2a\x6a\x68\x5d\x6f\xc5\x86\xc3\x59\x8a\x3e\x18\x35\x8f\xac\xa8\x51\x2d\xcd\x68\xc8\x81\x61\x15\x85\x61\x98"
	"\x74\x38\xe0\x84\x50\x10\x69\x61\x12\x73\x40\xaa\x81\xcd\x69\x4d\x0c\x02\x71\x48\xcc\xaa\xa2\x23\x62\x3f\xe4\xb0\xbe\x47\xd2\xe4"
	"\x3d\x9a\x98\x47\xcf\xcd\xb4\x43\x1c\x11\x5a\x8e\x2a\x56\xf0\xf1\xeb\x96\xf5\x80\x55\x45\x45\x08\x04\x46\x0b\xf2\x80\x12\xf3\x78"
	"\x38\x24\x8c\x0d\xcc\xf5\xcf\xb2\xb1\xe4\x6a\x8d\xed\xc6\xd2\x4c\xc3\xf7\xb3\x98\x92\x00\x42\xd2\x5d\x04\x56\xd5\x89\x3a\xd4\xfc"
	"\xa2\x4f\xea\x7c\xd8\xa7\x6f\x82\xe4\x07\x04\x73\x73\xf7\xde\xdc\x3d\xb5\x7c\x6f\xd4\xf2\x85\xa6\x9c\xa0\x69\xb1\x60\x6c\x1c\x0e"
	"\xa7\xd3\x89\x73\x4a\xf9\x7d\xd2\x9c\x52\x7a\x30\xba\x84\x78\x2f\xe3\xe0\x4c\x79\xd9\x2b\x09\x13\x22\x23\x60\x5e\x49\x5d\x2f\x3e"
	"\x70\x33\x0a\x95\xca\xb9\x50\x5d\x33\x19\x76\xb3\x9b\x71\x53\x72\x3e\xce\xb7\x6f\xd6\x79\xa0\xcc\xea\xf6\xcd\x22\x10\xe5\x0d\x46"
	"\x84\xe1\x8a\x1c\x2a\xf6\xe6\x56\x5b\x6d\x74\x28\x6a\x22\xc6\x85\x06\x18\x23\x94\x96\x6c\x5c\x97\x88\x40\x0c\x87\x3e\xd8\x00\xa9"
	"\x87\xf2\x0f\x0a\x10\xd0\x88\x0f\x23\xd2\xb9\x38\x0c\xd3\xd4\x71\x1d\x47\x89\x7e\xdd\x61\x71\xa0\xe4\xc0\xb8\xd9\x50\x29\x25\x14"
	"\x3f\x4d\x9b\x57\x9c\x4e\xc7\x71\x0d\xf4\xef\xbf\x6f\x37\x2e\x07\x0f\xde\x0e\xbc\x8d\x0d\x5d\x63\xa0\xe6\x0f\x75\x3f\x2d\xcd\x58"
	"\xad\xa0\x3b\xc3\xa0\x62\x38\x33\xa5\x35\x73\x26\xf9\x68\x3e\xd5\x71\xaf\x64\xcd\x5c\xba\x75\x26\xc1\x14\x15\x7b\xab\xd8\x96\xd9"
	"\x8c\x8c\x1b\x29\xa5\x86\x07\x35\xde\xf0\xa6\xf1\xf6\x49\x4d\x79\xc1\xe8\x06\x61\xcb\xac\xc6\x12\x27\xe7\xdb\x9f\xa4\xfe\x14\xb5"
	"\x94\xf6\x53\xc7\x83\xdb\xf5\x68\xdf\xf8\x67\xa3\xb5\x0d\x9f\x7d\x32\xc6\x56\x13\xed\x63\xb3\x5b\xe5\xa3\x39\x1b\x90\x4c\xce\x07"
	"\x5d\x91\x69\x56\xbf\x24\x02\x91\xe7\x19\x6b\x3e\xe8\x93\x65\xa2\x74\xa2\x4b\xd1\xf5\x19\x71\xae\x4f\x13\x44\x3d\x5b\xae\x56\xe3"
	"\xc1\xab\x71\x0b\x84\x0c\xf3\x41\x49\xe2\xd4\x76\x05\x84\x31\xa8\xd3\x97\xb3\x20\xc4\x8c\x66\x26\x00\xec\x08\xa3\x12\x40\x50\x60"
	"\x6c\x30\x15\x0b\xc5\x9a\xb2\x36\x7b\x13\x80\x08\xc5\x42\x89\x3c\x24\x8d\x05\x4a\x0e\xa4\x30\x65\x94\x41\xc0\x00\x62\x80\x00\x40"
	"\x00\x00\x40\x02\x32\x41\xd3\x60\x01\x84\x65\xe5\xe5\x52\xc2\xc4\xf2\xf8\x48\x9b\x14\x5c\xa9\xae\x4b\xc2\x9b\x60\xc6\x98\x32\x53"
	"\x48\x3b\xa1\x37\x85\x7f\x42\xdf\x24\xfa\x8d\x77\xdb\xfa\x45\x80\x10\xff\xc0\xcb\x0b\xe0\xe7\xf3\xd4\x12\xfc\x24\x23\x14\xe4\x98"
	"\xd4\x71\xca\x80\xb4\x04\x38\xea\x7b\x41\x23\x9f\x98\xae\xd9\x61\xe7\xa7\xfd\x09\x1c\x09\x0c\x70\xfa\x9a\xfb\x9b\x89\x21\x3c\xd6"
	"\xb7\x93\x91\x03\x43\x4e\xe5\x80\x8d\x10\xf6\x70\xfb\xb6\xfe\xe5\xf9\x7f\x27\x09\x27\x54\xe5\x71\x37\xe0\x78\x8e\x2e\xf5\x95\x22"
	"\xe5\xf3\x1a\x80\x2a\x13\xf1\x74\x82\x9d\xe4\xf9\x88\x0f\xa6\x24\x1d\x06\x42\x55\x08\xc8\x12\xe1\x0e\x15\xd4\x42\xa6\x42\xf8\xe9"
	"\xb5\x03\xc5\xe5\xaa\xde\x52\x9e\x8b\x01\x51\xed\x86\x94\x1c\x06\x48\x50\x82\xf5\x25\xd0\xb4\xb3\xf5\x41\xf9\xcc\x70\xc8\x93\x35"
	"\xb1\x02\xca\x72\x28\x0e\x74\x37\x7f\x40\x70\x5d\x14\x02\x39\x46\xa7\x3d\x5f\xdc\xdd\x69\x6f\xec\xc2\xfc\x64\x90\x68\x44\xdf\xde"
	"\xa2\x59\x0c\xa3\x0b\x75\x8e\xb1\xda\x75\x61\xfd\xaa\xee\xbc\x5d\x7c\xad\x23\x52\x0a\xb1\x94\x59\xef\x45\xa0\xd3\x49\x8d\x5f\x79"
	"\xe7\xc5\x51\x8c\x4e\x1f\x66\x98\x42\x2f\xe8\x70\xe0\x92\x01\x35\xbd\x2e\xd0\xfa\x33\x85\xb3\x1d\x97\xdf\xd8\x01\x59\x1a\xd5\xf2"
	"\xc2\xe5\xe7\x0e\x2a\x78\xc1\xbe\xd9\x05\x5e\x05\x26\xc6\x20\x71\x65\xd1\xc4\xc8\xcf\x9c\x71\x62\xaa\xd8\x87\x27\x26\xce\x48\x77"
	"\x06\x8a\x69\xb5\x8e\x4f\x7a\x6c\xdf\x8c\x5a\x81\x64\x0b\x66\x04\x30\x9d\x22\x06\x9c\x1d\x2f\xfa\xaf\x50\x25\xc9\xc0\x5e\x7a\xc6"
	"\x5e\x7b\xc0\x85\x46\x2a\x70\x39\x99\x07\x5c\xcf\xb0\xc0\x45\x8b\x3c\xe0\xd2\xff\x2c\x70\xfd\x53\x1d\x70\xb1\xfb\x63\xeb\x6b\x1c"
	"\x70\x01\xfd\xfc\xd8\xf5\x0e\xfa\xa3\xd3\x0d\x01\x74\xb8\xa5\x02\xd0\x73\x0d\x80\xcd\x7e\x60\x43\xa1\x20\x14\x25\x40\x77\xf9\xb0"
	"\xa4\x0a\x8f\x1a\x78\x3c\x00\x42\x63\x80\x77\x9d\xce\x1a\x55\x38\xbb\x46\xa5\xd1\x42\xe7\x3d\x80\x49\x9b\x6e\x90\x38\xe5\xd3\x5d"
	"\xd6\xda\xd0\x9f\x8b\x99\x32\x05\xd8\x34\x0e\x4b\x6a\x87\x3f\x8b\x56\xbd\x63\xe7\xc6\x68\x86\xe3\x6d\xaa\x8d\xbf\x60\x69\x87\xd4"
	"\x2b\xa5\x0f\x34\x2c\xd6\x42\x32\x67\x86\x42\xb8\x53\xb9\xd4\xe6\xc8\xd8\x8d\xbf\x0c\xbc\x66\xed\x6a\x19\x50\x39\x71\xe9\x5f\xcf"
	"\xac\x1c\x81\xbc\x0f\xe4\x6b\xc2\x7c\x1c\x52\x2d\x25\xa7\x49\x0c\x1b\x11\x40\x8c\x66\xf2\xef\x98\x26\x32\xc2\x19\x90\xf0\x86\x41"
	"\x17\xf3\xfc\xe7\xaa\xd6\xc7\x0f\x06\xa8\x34\x12\x78\xee\xd2\x11\x87\xff\x44\xad\x94\x87\x1f\x5c\x36\x04\x6c\x9a\xf7\x55\x66\xf7"
	"\x7d\xde\x7e\x92\x52\xa0\x74\x30\x7e\x77\x63\xa7\xac\x26\x4f\xbb\x50\x57\x42\xa3\x55\x94\xc6\xcd\xb3\x0d\x40\xf5\x5a\x19\x7d\x01"
	"\x1c\x31\x71\xc1\xce\x7e\x01\x66\x4e\xc2\xab\xe1\x28\x8f\x12\x10\x1b\x36\x92\x7f\x6f\x6c\xed\x8c\xf2\x97\x22\x39\x42\xbb\x96\x95"
	"\xad\xcb\xe7\x9f\x50\xa2\x37\xb9\x4f\x7d\xe0\xaa\x5b\x50\x4f\x9a\x8b\xbc\x87\xc4\xbe\xc7\xf8\xef\x65\x6a\xca\xdf\xc8\x64\xa2\x3d"
	"\xf4\x43\xd6\xae\x05\xc1\xca\x57\x0f\x28\xd3\xb8\x34\x92\x79\x25\x89\x7a\xb1\xd6\x60\x20\x8d\x15\xd4\x24\x19\xa0\x57\xca\x9f\x60"
	"\x94\xf2\x45\xd9\x84\x39\x47\x46\xb3\x94\x8a\xa4\x8d\xb7\x7c\xb0\xff\xb0\xfd\x78\xee\xfb\x94\xf8\x21\xff\xa1\xd4\x02\x87\xb2\x8c"
	"\xb4\xf7\xb7\x34\x70\xd6\xd9\x24\x10\xb7\x99\xbd\xd3\x15\x86\xb3\x7f\xb2\x19\xae\x87\xf2\xe8\xb5\x9a\x68\x12\xee\x58\xb8\xb2\x6c"
	"\x74\x20\xb6\xf0\x70\x5b\x4a\x6f\x2d\xf8\x01\x52\x20\xc3\x3a\xd6\x98\x93\x8b\x5c\xf5\x83\x6c\x80\x52\x4f\x80\x0d\xe4\x4b\x82\x6b"
	"\x51\x8d\x14\x58\x50\x1d\x4a\x00\x14\x8b\xc8\x89\xf1\x4e\xb9\xd5\x89\x4f\x2e\x04\x13\x43\x7e\x6d\x08\xaf\x03\x98\x8d\xd3\x95\xd2"
	"\xd9\x56\x9b\x17\xa1\x44\xf5\x55\xef\x68\x6d\x91\xc1\xc2\xfe\x29\x25\x25\x3e\xee\x5a\x06\x44\xe4\x18\x0c\xc9\x56\xd1\x81\x29\xe7"
	"\x93\x02\x58\x3c\xa9\xdb\x64\xe4\x7f\x3e\x45\x0a\x92\x52\x9a\x18\xe7\x94\xf2\x85\x3b\x5e\x97\x22\x93\x1c\x48\x38\x65\xdb\x4c\x01"
	"\xd7\x7c\x67\xfd\xd3\xe4\x69\x1f\xb2\x5f\x67\x01\x75\x6e\xad\x2d\x61\x22\xaf\x4c\x21\x56\xef\x0f\x67\x1d\x3d\xc2\x5d\x4e\xf7\x1a"
	"\x59\x83\x3d\x8b\x08\x75\x3f\xb9\xf7\xfe\x10\xa5\xcd\x9a\x56\xe0\xb1\x10\x77\x87\x1b\xee\x70\x8e\x5b\x28\xc4\x6a\xda\x86\x1a\xd7"
	"\xdb\x3f\x95\xb0\x75\xe6\x1a\xc4\xa4\x28\x0b\x59\x93\x66\x33\x24\x07\x9e\x08\x13\x11\x5f\x26\x3c\x8c\x44\x9e\x60\x46\x77\xf6\xd8"
	"\x63\xdb\xe5\x45\xc2\x86\xbd\xd0\x05\x18\x94\x0b\xfc\x0f\x02\xf7\xfa\xeb\x7d\x39\x5e\x21\xa2\x19\x59\x2a\x75\x17\x90\x5e\x2c\x8d"
	"\x95\x73\xd3\x35\x15\x34\xe6\xfa\x78\x67\x2c\x0d\xf5\x26\xf3\x45\x76\xb5\xd0\x12\xc7\x0b\xe1\x2a\x32\x1a\xac\xb6\x86\xc7\x8c\xa1"
	"\x9b\x2a\xc9\x8a\x30\x6a\xbb\xd5\x25\x16\x49\x1d\x5a\x0b\x8d\xfa\x14\x60\x38\xc3\x05\x79\x84\x86\x0b\x59\x79\x72\xa2\xed\xf0\x45"
	"\x1c\xb4\x6a\xeb\x85\x3c\x5b\x26\x2e\x2f\x14\xc7\x66\x24\xbd\xbf\x6f\x17\xaa\xc2\xac\xc4\xf0\x23\x1f\xca\x94\x0b\xbd\x0c\xb2\x54"
	"\x4e\x8b\x1a\x1c\x2b\xd9\x20\x16\x23\x4d\xd2\xc0\xf5\x18\x5d\x4d\x67\x59\x21\x1d\x11\x1f\x3b\x2d\x60\xf8\x84\xd8\xb8\xaf\x7a\x5d"
	"\x8f\xef\x46\x03\x98\x0d\x20\x12\xc5\x7a\x8a\x0f\xfc\x99\x10\xf6\xa4\x70\xec\x4b\x09\xd7\xf3\x50\xa2\x39\x68\x29\x01\xa6\x55\xb4"
	"\x90\xcb\x28\xb0\xe1\xb9\xe9\xfd\xd2\x51\x6b\xe7\x87\x36\xd3\x0e\x99\xca\x60\x75\xb5\x35\xf9\x79\xde\xb9\x22\x2f\x21\x6a\xb6\x1c"
	"\x7d\x78\xd9\x00\x65\x0e\xe8\xcb\x84\xb9\x12\xa3\x2d\x41\x8d\xc0\x37\x98\xb9\x55\x4b\x03\x8a\x83\xea\x14\xd3\x1c\x52\xf1\xf7\xe8"
	"\x86\x56\xe5\xa0\xa9\xe2\xa0\x6a\x33\x8e\x19\x73\x65\xb3\x47\xc6\x0e\xb5\xd6\x51\x44\x71\x42\x5f\x85\x51\xd2\xee\x10\x72\xad\xa2"
	"\x1b\xe4\x36\x68\x9d\xea\x51\xb5\x8c\x41\xfd\xd9\xed\x1f\xfc\x79\x5d\xb2\x09\x15\x6c\x13\xe4\xae\x71\xfe\x7c\x80\x46\x1e\xb2\x81"
	"\x6b\x75\x5d\x0b\xbf\x57\xbd\x58\x06\xc0\xd8\x5b\x77\x5b\xe9\x48\x36\x10\x7c\x18\xf8\x8e\x0d\x76\xef\xdb\xf3\xc7\x50\x4f\x11\x02"
	"\xf4\x31\xce\xbf\x58\x44\xb0\x36\x22\x0b\x17\x17\xd1\x19\x25\x54\x80\x45\x7b\x84\xfd\xc1\xe8\x94\x77\xf8\x69\xf4\x0f\x42\x66\x03"
	"\x37\x96\x20\x6a\x7b\x1c\x0a\x08\xac\xcf\x66\xf9\xe4\x98\x93\x4a\x09\x9b\x03\x8e\xc9\x3f\x01\xc6\x01\x1c\xc4\x12\x34\xbf\x80\xc3"
	"\xba\x20\xa9\x19\xc0\x5c\x83\x98\x66\xc9\x4d\x69\xc9\x51\x20\xf4\x29\x48\x6d\x3c\xaf\x09\x31\x4c\x38\x0f\x88\x49\x5f\xc1\xf1\x74"
	"\xab\x1b\xc7\x40\x9f\x5c\x4e\x2e\xae\xbb\xc6\x14\xc5\x2c\xf7\x9c\x0e\x1f\x1f\xf6\x8f\x6e\xe6\x5c\x20\x58\x66\xe7\x87\x00\xaa\x24"
	"\x64\x8f\x15\xb2\x57\x72\xa8\x12\xea\x5d\x02\x30\x5e\x8b\x02\xf0\xf9\x8d\x78\x90\x7b\xba\x3e\xd5\xa9\x78\xfd\x11\xb5\x90\xc2\xd7"
	"\x69\xb2\xb4\x5c\x4b\x99\x9a\x17\x02\x6b\x6c\x3a\x1a\x5a\xc4\x50\x6e\xa3\x41\x3c\x66\x33\xfc\x73\x9a\x96\x5a\x81\x5c\x25\x05\x34"
	"\x90\xfc\x8c\x39\xcb\xa8\xaa\x35\xb4\xd0\x04\x29\x7d\x1e\xef\xcf\x8d\xcd\x8e\x80\xe8\x67\x4e\x42\x0b\xf0\x0c\x4d\x51\x58\xf2\xa8"
	"\x81\x0d\xf5\x01\x12\xc9\x17\xba\x8f\xb5\xd3\x73\x96\xd0\x75\x80\x62\x74\x2a\xae\x5a\xce\x0e\x6d\x1f\x23\x5d\x9c\x97\xc9\x31\xda"
	"\xab\xa8\x70\xe2\xb8\x79\xf1\x7c\x30\x5f\x75\xfd\xf6\x5b\xc9\xcf\x46\xf5\x80\x17\x42\xc2\x37\x11\xc2\x7d\x41\x24\x14\x61\x60\x1a"
	"\xe0\x21\xec\x58\x41\xe0\x4c\x00\x04\x43\xe6\xb5\x7e\xb6\xb8\xe2\x99\x21\x20\x34\x40\x7d\x1e\xff\x66\x32\xf8\xf7\x4e\x62\x78\x88"
	"\x55\xa9\xfa\x0b\x63\x61\x87\x62\xe9\xf3\xfe\xee\xc4\xcc\xa4\xa3\xdb\x04\x2f\x1a\x82\x82\x0f\x91\x1b\xd1\x0f\x09\x2f\xf9\x7d\xa4"
	"\x03\x08\xcb\xae\xb3\xaf\xc3\xe3\x94\x3f\xf7\xff\x12\x27\x15\xfe\x95\x08\x94\x1f\xb1\x3d\xfc\x67\x3d\x8a\x63\x0b\x85\x8a\x40\xfd"
	"\x67\x4b\x8f\x04\x75\xca\x1c\xe2\xff\xf0\x5d\xea\xec\x76\x33\xe3\x8b\x8d\xda\x37\xc0\x76\x73\x0e\xe2\x5d\xf3\xa0\x69\x21\xb1\x1c"
	"\x63\x7f\x90\x02\xae\xea\x6a\xda\xf5\x2c\x57\x3f\xff\x41\x8c\xfe\x9c\x12\x13\x4e\xbe\xe7\xfd\xa3\x46\xb9\xf1\x60\xec\x88\x8e\xb2"
	"\x9c\x7b\xc8\x60\x46\x01\xb3\xd4\x30\x3e\xdb\xf8\xde\x24\xe4\x19\xa8\x41\x08\x27\xa7\x64\xb2\x4b\x03\x76\x09\x9b\x67\xb2\x38\x67"
	"\x8d\x76\x53\xfc\xde\x28\x7a\xab\x60\x44\xe0\x51\x91\x3e\x4a\x29\xe2\x09\xfd\x68\xaa\x8d\x4c\x2d\x8f\xe7\xb6\xd2\x09\x90\xa4\x3e"
	"\x8e\x29\xd8\x44\x50\x41\xad\x00\xf1\x00\x01\xbd\xb9\x98\x86\xaf\x5f\xe0\xa2\xb0\x25\x77\x3c\xd8\xf9\x21\x50\x69\x83\x38\xb0\xcf"
	"\xbc\x64\xbf\x43\x80\x37\xb8\xb7\xa1\xfa\x0a\x4c\x06\x4f\xf2\xfe\x5f\x43\xaf\x15\xd8\x78\x41\x1e\x9a\xa5\xc6\x33\xe1\x24\x67\xab"
	"\x56\xa8\x0a\x94\x09\x07\xc9\xa0\x58\xf8\x95\x45\xe8\xe1\x5b\xd3\xff\x50\xa7\xe1\xcd\xbf\x85\xf9\xc1\xd5\x03\x27\xdf\xb5\x89\x2c"
	"\xc0\xe6\x27\xce\x63\x7e\xc0\x24\x62\xd6\x7e\x1a\x40\x9e\x91\x10\x15\x15\xc5\xe0\x0b\xf0\x42\xa4\x34\xe3\x73\xa1\x58\x28\x3b\x40"
	"\xed\xc0\x2b\xf1\xb9\x0a\x42\x31\x35\x04\xea\x70\x41\x80\x48\x18\x4f\xef\x6b\x05\x41\x0a\xca\x22\xbc\xd6\xee\xec\x8a\x7f\xb5\x7c"
	"\xce\xb0\x75\x67\x07\xbc\x73\xff\x32\xa6\x0e\xf5\x0f\xab\x30\x99\xbe\x48\x68\xb2\x9c\x78\xda\x77\x14\x50\x77\xde\xba\xcc\xc2\xe0"
	"\xe5\xc1\x81\x0c\x3e\x8e\x58\x47\x4d\xa9\x91\xd0\xb9\x3e\x62\xac\x02\x4a\x7f\x8c\x48\xf6\x66\xd6\x70\x4a\xfe\xae\x2d\x4a\xd5\x6f"
	"\x9a\xff\xb9\xcc\xae\x36\x65\x77\x84\xe1\x8a\xb1\xa1\xeb\x78\x5f\xe8\x19\x94\x21\x53\x8f\x33\xfe\x25\x30\x0b\x97\x9d\x70\x66\x98"
	"\xa2\x5f\xba\xc4\xaf\xc1\x54\x29\xdb\xdc\x1c\xf9\x3c\xe1\x1a\xa0\x0b\x5d\x0e\x71\x0c\xd9\xf6\x48\x85\x74\xe2\x70\x90\x17\xcc\x3d"
	"\x31\xdd\xf8\x04\x31\x51\xab\x82\x6a\x7b\xc0\xba\xa4\xda\xf0\x93\x01\x7f\x13\x55\x7b\x85\xc7\x21\xcb\x58\xd5\x54\x94\x2d\x3a\x27"
	"\xbc\x75\x0d\xb9\xb3\x8b\x38\x6a\x8b\x55\xf0\x58\xe4\xb3\xe3\x53\x6e\x02\x45\x16\x03\xfb\x2e\x7b\xbc\xb5\x55\xea\x81\x02\x0a\x5c"
	"\x10\xcd\x2d\x72\x3d\x9e\x73\xa8\x50\x32\xff\x42\x39\x4a\x66\x83\xc9\x42\xe4\x6d\x52\x94\x00\xa3\xef\x24\xbc\xac\x88\x75\x3a\x93"
	"\x2f\x4b\x76\x92\x10\xa5\xff\xb3\xa2\xdf\x00\xe6\xaf\x31\xa8\xb3\xc3\x09\x21\x57\xe1\x8d\x4d\xb8\x35\xfa\xcf\x5d\x95\x81\x5c\x78"
	"\x46\xd9\xdd\xcd\x9f\x43\x15\x1c\x60\xca\x41\x5d\x1d\x2b\x56\x7b\x37\x79\xa6\x3b\xa5\x77\xba\x26\x81\xb1\x39\x14\x54\x26\x35\x4a"
	"\xa0\xe2\xf4\x1a\x56\xaa\x26\x17\xcd\x10\x05\x76\x53\x98\x6e\xcd\x37\xdb\x73\x8b\x17\x99\xab\xe9\xdf\x4b\x99\xd1\x6d\xdf\xfc\xf3"
	"\xfb\x17\x2e\x7d\x86\x04\xf2\x44\x43\x46\x23\x99\xfd\x5b\xaf\x4d\xfc\x77\xd0\x76\xdb\xd5\x38\xde\x77\x17\xb2\x59\xe8\xf1\x10\x53"
	"\xa2\x19\xc9\x2b\x14\x30\x55\x7a\xeb\x08\xde\x41\x20\x63\x05\x05\x5e\xac\x7c\x09\x15\xa9\xd9\xbc\xac\x6a\x08\xa7\xf9\x12\x4d\x8c"
	"\x4e\x12\xac\x84\x5e\x80\xb8\xd0\x89\x5f\x83\xa9\x3b\x4f\x2d\xc0\xe4\x91\xde\x40\xf2\xa5\x46\xde\xea\xb6\xe4\x05\x24\x07\xd8\x76"
	"\x70\x58\x84\x81\x24\x6b\x3d\x66\x89\xec\x13\x23\x2f\x3b\x70\xb1\x77\x90\x15\x71\xc1\x91\xa4\x83\x10\x1b\x6e\xba\xf5\x24\xd0\xd7"
	"\x4e\xfe\xd1\xc0\x36\xc7\x2e\xe9\xb8\xb8\x5c\x75\xb7\x95\xc4\x4e\x25\x1a\xba\x6d\xa3\x69\x40\x13\x9a\xb8\x84\xa4\x8b\x42\x61\x84"
	"\xc8\xe6\x83\x49\xa6\x90\x50\x15\x5b\xd8\x86\x95\xb0\xac\xc0\x83\xc5\xdc\x8c\x86\x26\xfd\x45\x00\x00\x08\x61\x01\x00\x12\x89\x07"
	"\x42"
;