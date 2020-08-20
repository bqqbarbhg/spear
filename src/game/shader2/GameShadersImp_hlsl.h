#include "GameShaders_hlsl.h"
#include "game/ShadersDesc.h"

const SpShaderInfo spShaders[] = {
	{
		"TestMesh",
		{ {
			{  },
			0, 1,
		}, {
			{ {0,2}, {1,2} },
			1, 4,
		} }
	},
	{
		"TestSkin",
		{ {
			{  },
			5, 1,
		}, {
			{ {0,2}, {1,2} },
			6, 4,
		} }
	},
};

const SpPermutationInfo spPermutations[] = {
	{ { 1 }, {  }, { 1,2,3,4,5 }, 0, 1480 },
	{ { 2 }, { 1,2,3,4 }, {  }, 1480, 4897 },
	{ { 2 }, { 1,2,3,4 }, {  }, 6377, 4897 },
	{ { 2 }, { 5,2,3,4 }, {  }, 11274, 5203 },
	{ { 2 }, { 5,2,3,4 }, {  }, 16477, 5203 },
	{ { 3,4 }, {  }, { 1,6,7,8,9,10 }, 21680, 1825 },
	{ { 2 }, { 1,6,7,8 }, {  }, 23505, 4851 },
	{ { 2 }, { 1,6,7,8 }, {  }, 28356, 4851 },
	{ { 2 }, { 5,6,7,8 }, {  }, 33207, 5157 },
	{ { 2 }, { 5,6,7,8 }, {  }, 38364, 5157 },
};

const SpUniformBlockInfo spUniformBlocks[] = {
	{ }, // Null uniform block
	{ "Transform", 64 },
	{ "Pixel", 1056 },
	{ "SkinTransform", 64 },
	{ "Bones", 3072 },
};
const SpSamplerInfo spSamplers[] = {
	{ }, // Null sampler
	{ "shadowGrid3D", (uint32_t)SG_IMAGETYPE_3D },
	{ "albedoAtlas", (uint32_t)SG_IMAGETYPE_2D },
	{ "normalAtlas", (uint32_t)SG_IMAGETYPE_2D },
	{ "maskAtlas", (uint32_t)SG_IMAGETYPE_2D },
	{ "shadowGridArray", (uint32_t)SG_IMAGETYPE_ARRAY },
	{ "albedoTexture", (uint32_t)SG_IMAGETYPE_2D },
	{ "normalTexture", (uint32_t)SG_IMAGETYPE_2D },
	{ "maskTexture", (uint32_t)SG_IMAGETYPE_2D },
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
	"\x28\xb5\x2f\xfd\x60\x01\xa9\xdd\x4b\x00\x1a\x50\x14\x0f\x2b\xd0\x6c\xac\x0e\xa8\x18\x8e\xa2\x0c\x83\xcd\x21\x5e\x44\x1e\x37\x8e"
	"\x5d\x3d\x8f\xe3\xa4\x9a\xe2\x0d\x40\x0d\xcc\x8a\x84\x60\x86\x10\xe8\x97\x32\xcd\x20\x57\xb3\xf4\x30\x34\xe3\x00\xe8\x00\xe9\x00"
	"\x71\x8a\x0c\x2b\x5a\x92\x5b\xb9\x6c\x70\x3e\x84\x9d\xef\x12\xdc\x13\x07\xe2\xbc\xaa\x26\x3c\x17\x83\xb1\x38\x3c\xb4\x26\x79\x85"
	"\xc7\x89\x82\xa4\x0d\x79\x97\x8b\x18\x8c\x38\xd1\x6c\x69\xe2\xb9\xf6\xa3\xcb\xc7\x24\xa9\x6f\xb8\xd3\x14\x27\x12\x0f\x74\xc4\x8a"
	"\xaa\x62\xc4\x16\xbe\xa1\x6b\x6a\xe2\xb9\x60\x9c\xa8\x4b\x71\x8a\x97\x36\xc4\x69\xd3\x62\x8e\x7d\x27\x1e\x09\x0f\xdc\xd0\x65\x83"
	"\x29\xb7\x82\x27\xdd\xaa\x3d\x57\x6d\xc3\xad\x64\x2c\xbe\xde\x6f\xed\xe3\x0b\x61\x6a\x99\x77\x31\x8c\x17\x08\x53\x9d\xeb\x86\xe2"
	"\x0f\xb8\x86\xaf\x26\x6f\x13\x57\x8e\xa9\x46\xd3\x5d\x30\x99\x85\x32\xd9\x54\x63\xca\xc9\xb8\xcc\xe0\x58\x9c\xcb\xaa\x5e\x38\x9c"
	"\xcd\x56\x3f\x3c\x2d\xcf\x73\x4d\x78\x2e\xab\x56\x9c\x2e\x9b\x97\x52\x6f\x74\xb3\x4d\x9d\xcd\x5b\x94\x9d\x1a\xbe\xdb\x15\x21\x0b"
	"\x14\x28\x68\x60\x70\xa0\xf2\x01\xdf\x6e\xd5\x35\x5f\x2e\x9b\xaf\xf4\xbc\x56\x93\x78\x24\x3c\x5f\x2a\xd6\x9a\x21\x9b\x50\xae\x31"
	"\x1d\xd6\x39\x7f\x74\x48\xe3\x08\xf9\xd1\xb1\x5b\xf6\x66\x23\x27\x04\x67\x67\xc4\x72\x49\xa2\x6f\xe6\x81\x01\x02\x04\x19\xc4\xb2"
	"\x23\x6b\x43\x8f\x0e\xa3\x06\xba\xe2\x51\x23\xad\x7d\xec\x72\xf1\xc0\x88\xbe\x19\xcb\xa5\xa8\x89\xc0\x54\x0b\x81\xa1\x64\x0e\xaf"
	"\x1e\x58\x02\x1c\x4c\x3d\x21\x7a\xab\x97\xbb\x55\x4d\x4c\xb5\x89\xa1\x62\x66\x72\x59\xc6\xe5\xcb\x21\x6b\x88\x4d\x95\x68\xdd\x66"
	"\xe9\xae\x13\x58\x55\x11\x79\x6a\x52\x52\x54\x9e\x56\x33\xdc\x0c\xbd\xd8\x29\x33\x97\x33\xaa\xa7\xa6\x60\x6a\x39\x6d\x52\x34\xa4"
	"\xca\x54\x8c\xc0\xbb\x99\x09\x91\x4f\x60\x01\x52\x51\xdc\xe9\x4d\x2b\x11\x5f\x7b\x31\xd9\xbd\xa5\xe9\x06\xcf\x0a\x80\x67\xc2\x85"
	"\xdd\x83\xd8\x2b\xa7\xe3\x74\x97\xa0\x9c\x2f\xc6\x3c\xba\x57\x2f\x9d\x6f\x4f\x50\x82\x2c\xdb\x69\xc7\x3e\xfd\x7f\xae\x09\x9b\x12"
	"\xcf\xa5\xbb\x9b\x71\x79\xab\x19\x97\xa1\xad\x4e\x4d\x72\x50\xd2\xa2\x62\x43\x56\x94\x54\x38\x1a\x0c\xc7\x43\x36\x04\x38\x1a\xd6"
	"\x63\x8a\x91\x23\x3a\xaa\xc7\xa3\x6a\x34\x02\x5d\xbe\xdc\xa6\xdc\xd4\xe8\x76\x35\x23\x3a\x84\xdf\xf4\x00\xe5\xcb\xc4\x14\x17\xbc"
	"\x14\xa3\x28\x12\xb1\x58\x24\x97\x1e\x60\xe4\x92\x05\xa2\x7a\xe2\x4e\x4f\xcb\x15\x97\x40\xb4\x53\xb1\x47\xae\xf0\x05\xc3\xb5\x38"
	"\x54\x9c\x9a\x22\x34\xbb\x8d\x6a\x3d\x4f\xe0\xcf\x84\xc4\xe4\x50\x60\x28\xfe\x75\xf6\x04\xa1\x4b\x90\xa5\x1c\x2a\x99\x12\x8a\x28"
	"\x83\xef\x6d\xd8\xb4\x12\xe0\x7b\xaa\x66\x35\x20\x6d\x97\x80\x6a\x58\x88\xb0\x47\xa0\xe0\xc7\x29\x42\xb6\xcb\x14\x43\x85\x57\x12"
	"\xa4\x6f\x04\x8a\xce\x75\xf2\x05\xf6\xa9\x2d\xcd\x78\xb9\xab\x99\xb9\xa0\x6e\x2a\x56\x60\x9f\xeb\x41\x55\x49\x49\x51\xd7\x83\x78"
	"\xc5\xb1\xc7\xa9\x67\xe6\x16\xcd\xf3\x62\x79\x98\x4c\xa8\x9e\x9d\x9d\x1c\x7a\x05\x07\x07\x8b\x45\x8a\xd8\x1d\x5e\x5d\xba\xa4\xd1"
	"\xd4\x0c\xe6\x15\x9e\xb6\xc3\x50\x2e\x67\x0e\x90\x05\x06\x06\x28\xbc\xc0\x09\x3c\x18\x15\x43\x80\x7c\x30\x28\x8b\x9a\xaa\x28\x4e"
	"\x52\xbb\x1b\xcd\x28\xb0\x2b\xbe\x78\x24\x72\xec\x10\x89\x04\xbb\x64\x6f\x36\x8a\xde\xe7\x7d\x50\xc3\x8d\xe3\xe4\x66\xd3\x7a\xb3"
	"\x69\xa9\xe1\x6d\x8b\x1a\xde\xa0\xa4\x24\x28\x25\x12\x0f\x54\x71\x97\xf3\x3c\xaf\xd3\x89\x52\x52\xd4\xf7\x29\xa4\xa4\xa8\x3d\x45"
	"\xdf\xd3\xae\xde\xc5\xcf\x55\xed\x7a\x4b\x3e\x28\x07\x97\x5b\x54\xa6\xdb\x35\xb0\xd5\x08\x0a\xf5\xb2\x99\x29\xa6\x66\xb6\xba\xd5"
	"\x4e\x52\xde\xa7\x2f\x51\xf9\x52\xcc\x14\x65\x2f\x75\xf6\xc4\x61\x8f\xf6\x8b\x7f\x36\x5a\x68\xab\x9a\x6d\x7c\x16\xc2\xf7\x5a\x4d"
	"\xb4\x8e\xcf\x72\x94\x6f\x62\xcc\x00\x91\xde\x07\xb1\xe7\xfa\xa2\xeb\x92\x45\x9a\x58\xc7\x14\x0b\x57\x3c\x76\xe8\xf8\x76\xa2\x08"
	"\xbb\x8c\x3c\xec\x92\x04\x2b\x88\xc3\xd4\x6a\xf6\x84\x2d\x6e\x69\x50\x2f\xdf\xa4\xd8\xd3\xc3\x2b\x0e\xfb\x04\x7b\x44\x9e\x09\xf4"
	"\xd4\xa2\x77\xb0\x33\xe8\xa9\x41\x09\x7b\x03\xe8\x8b\x47\xff\xf6\x04\x25\x08\x7a\xd0\x53\xbb\xb4\x20\xbd\xd5\x8d\x26\xa0\x45\x51"
	"\x0d\xc7\xe3\x21\xa5\x6d\x5d\x70\x5f\x90\x73\x05\x07\x8b\xce\x91\xc7\x4e\x71\x82\x78\xa8\x72\xf3\x54\x2a\x55\x33\x03\x01\x80\x24"
	"\x08\x82\x09\x22\x0e\xa8\x61\x1a\xf5\x96\x72\x12\xc0\x91\x30\x0c\x92\x24\x88\x31\xa5\x90\x31\x08\x8c\x48\x00\x00\x10\x00\x40\x04"
	"\x00\x00\x00\x02\x04\x0c\x38\x81\xfd\xfe\x28\xe4\x68\xe5\x62\xe3\xfc\xf5\xb4\x36\x05\xda\x8a\x1d\x30\xa1\x3d\x0f\x0a\xe4\xb6\x09"
	"\xb8\x66\x70\x7b\xa9\xb4\xfc\xaa\xb9\xd7\xc6\xe0\xa2\xcf\x16\x72\x06\x92\x78\x53\x51\x7d\xa5\xba\x6b\x9d\x31\x3a\x61\x81\xd1\xbd"
	"\xf6\x51\x21\xb9\xd1\x78\x32\x4d\x89\x0e\xdd\x16\xd8\x42\x8e\x9e\xd6\x62\xb8\x09\xff\x58\x5b\xdf\xeb\xb3\x5a\x39\x40\xe5\x1f\x54"
	"\xeb\xe8\xbc\xb4\xd4\xd6\xbd\xa1\x44\xf6\x2c\x90\x16\xef\x6e\xb7\xf6\x5e\x96\x55\x57\x7a\x54\xc1\xac\x26\x29\x20\x2b\x89\x59\x87"
	"\xbe\x71\xd6\x99\x01\xb4\x0e\x6b\x1e\x16\x51\xbf\x40\xa7\xe0\xc0\x6a\x15\xe9\x50\x2c\x5c\x46\x41\xfc\x84\x75\xf6\xcf\x56\xa5\xaa"
	"\x40\xc4\x7f\x6f\x8d\x70\xa4\x82\x20\xb2\x1f\xc6\xf5\x5f\xc8\x45\x2c\x78\x93\x44\x53\x44\xe1\x31\x79\x00\xf8\x11\x88\xb0\x4a\xa2"
	"\xcc\xd7\x24\xef\xbf\x55\x60\x72\xbc\x6e\x9f\xb9\x4e\xd7\xf9\xad\x53\x3b\x87\x3d\xc6\x41\x4b\x9e\xba\x15\x20\x9c\x9c\x00\xfc\xb0"
	"\xda\x57\xeb\xa1\xaa\x97\x84\xaf\x98\xff\x9c\xa2\x1a\x6c\x03\x57\x3c\xb8\xab\x19\xfc\xe8\xcb\xf9\x1e\x40\x15\xc7\x25\x5d\x8e\x4a"
	"\xd9\x89\x0b\x95\x2a\x90\x3c\xa3\x44\x76\x71\xe9\x02\xb6\x8c\x4b\xc2\x7d\xdc\xe9\xb2\x88\x13\x34\x50\xd6\xbc\x94\x86\x13\x97\xa2"
	"\x79\x94\x48\xad\x96\x2a\xda\x0e\x38\xc8\x4a\x48\x51\xc8\x43\xa4\x6b\xbc\xa8\xd0\x35\x53\x05\xa5\xb0\x1d\xa5\x47\x9a\xa5\x90\x23"
	"\x59\x66\x3f\xbf\x15\xbd\x5b\x5f\x74\x77\xbe\xf5\xa3\x5f\x67\xdf\xdd\x3e\x76\x92\x97\xa1\x1f\xdf\xfc\x54\x76\x71\x4c\xd7\xa5\x0f"
	"\xbe\xbb\x71\x65\x7f\xc3\x81\xcf\x2a\xa6\x63\xe7\xf4\x2c\xc0\xaa\x58\xbe\xbd\xca\x08\x22\xf2\x78\xe1\xf4\x39\xfb\x19\xe1\xb4\x6b"
	"\x05\x51\x25\x15\xd1\x50\x11\xac\xe2\x14\x4f\x2d\x51\x05\x2e\x77\x28\x7e\xfc\x33\xfc\xbf\x8c\x1b\xf4\xa7\x2c\x53\x65\xc3\x52\x53"
	"\x46\x95\x25\x63\xd6\x50\x9f\x64\x5c\xd3\xa7\x28\xf8\x37\xc8\xb2\x64\xcc\xde\x57\x0a\xe4\x39\xa9\xf6\x9c\x27\x61\x19\x1b\x7a\xfb"
	"\x54\xd9\x1f\x37\x85\x3b\x7b\x20\xf7\xe8\x92\x74\x87\x5a\xc6\xe1\x06\x81\xa6\xe5\xc6\xb4\x88\xb7\x37\x51\x77\xd8\x25\x3b\x48\x9c"
	"\x27\x2b\xe9\x7e\x7a\xee\x96\x6d\xd3\xd8\x00\x62\x31\x12\x7a\xb1\xdc\x62\x35\xd9\xd4\x90\x63\x27\x19\x14\xa8\x32\x8e\xb9\x63\x7a"
	"\xb2\x4c\xea\xbc\x6f\x5e\xd9\x25\xa9\xc9\xd9\x24\xec\x8b\x77\xc6\x27\x0c\x95\x33\x41\x52\x9d\x11\xfc\x8f\x34\xa9\x80\xd9\x85\x5b"
	"\x5d\xbe\x3a\xf3\xa9\x99\xe4\x2b\x61\xb0\xc6\xb1\x93\x03\x09\x32\x0b\xfb\x57\xd6\x04\xba\x6a\x06\x2d\x10\xc0\xb4\x92\x79\x34\xd1"
	"\x19\x80\x93\xa6\x0f\x76\x97\xd5\x90\x1e\xbf\x62\x10\xb5\xa6\xc0\x38\xa6\x5f\x3c\x31\xda\x49\x72\x02\x25\x60\x1b\xd0\xda\xfc\xd3"
	"\xbe\xb1\x30\xd4\x12\xa2\xbe\xf0\x9e\x6f\xf6\xe6\x65\x44\x00\x99\x00\x43\x14\xae\xcb\x87\x1a\xc0\x12\xc6\x6a\x78\xe2\x17\x30\xa2"
	"\x06\x8c\x34\xce\x2f\xd5\x20\x50\x35\xcf\x85\x90\xe2\x10\xf1\xe5\xfa\xb5\x9b\x21\xcc\xbf\xe8\x61\x51\xcc\xea\x00\x62\x28\xe8\xe1"
	"\x49\xf3\x17\xa5\xab\xea\xab\xcd\x8d\xc9\x94\xaa\x57\x6d\x26\x20\x38\x81\xb2\x5e\xb0\xb3\x26\x70\xf7\xc0\xc6\xc7\x8c\x4c\xc4\x4f"
	"\x00\x66\x44\xa1\xba\x54\x24\xa7\x29\x4a\x9c\xa5\x24\xdf\x83\x07\xd6\xae\x8c\x89\xae\x56\x92\x01\x47\x7e\x2f\x1d\xbb\x60\x08\xe2"
	"\xec\x7b\x4f\x1e\x5f\xb4\xf7\x83\x28\xa9\xc9\xab\xe2\x58\x14\x3b\x41\x08\x64\x4c\x25\x26\x2e\x17\x00\x68\xf8\x94\xda\x75\xc4\xea"
	"\x2a\xec\xb4\xa2\x29\x04\x40\xef\x48\xc3\x07\x79\xab\xd6\x26\xa9\xab\x98\x8d\x56\xc5\x44\xd2\xae\xb9\x8a\xeb\xd6\x09\xda\x17\xac"
	"\x42\x9b\xb4\xd1\x36\xfc\x31\x1b\xc5\xc0\x44\xb8\x18\xfb\x86\x5c\x2e\xdc\x6a\x43\xa8\x99\xdb\x52\x41\xe4\xca\xaf\xa5\xe7\x9f\x00"
	"\xa1\x47\x50\xc8\x5c\xa5\x17\xc9\x02\x5f\xe3\x2b\x90\xa5\xc1\x50\x92\x46\x12\xa2\x86\x2a\x55\x3a\x7a\x80\x1b\x7d\x89\x98\x4d\x88"
	"\x4f\x4b\xda\x1e\x08\x05\x07\x13\x4a\xb6\x55\x46\xcf\xd0\x42\x5f\xa6\xfa\x0c\xb5\x6c\xd9\x11\xa4\xec\x82\x23\x49\x4e\x95\x8c\x8f"
	"\xbc\xce\x5d\x90\xf6\x1a\xb5\x41\x80\x8b\x0d\xd0\xfb\xfe\x70\xd0\xa5\xe3\x05\xed\x40\xd1\xf8\x83\x08\x4d\x25\x0a\x68\x74\x8a\x31"
	"\x4a\x97\xe0\x5e\xab\x1f\x9d\x4f\xef\xf5\xe0\xcd\x7c\x69\x78\x15\x9e\x4d\xbf\x77\x04\x2c\x45\x9e\x9c\x05\x38\x36\x8a\x9f\x42\x4f"
	"\x87\xad\xa1\x2f\x4e\x86\x41\xce\x54\xe9\x3c\xbc\x24\xcd\x42\x9e\x4e\x88\x0c\x83\xeb\x57\x97\xba\x6b\x18\x2b\xa6\x5b\xf0\x91\xa4"
	"\x4f\xed\x01\x71\xa9\xab\x20\x1f\xfb\xda\x1e\xa5\x49\xaf\x05\x47\x5e\x41\x87\x95\x98\x8c\xb6\x76\x9a\xf3\x5e\x06\xa1\x6a\x59\x63"
	"\x3a\xfd\xb9\x24\x51\xab\x0a\x91\x58\x13\xc4\x00\x4d\xf1\xb3\x9b\x70\x2e\xbd\xde\xcd\xfd\x3b\xe9\x17\xba\xc1\x99\xf2\x10\x79\xb0"
	"\x87\x1b\x46\xce\x32\xcb\x5a\x3f\x8c\xcd\x6a\x7d\x58\x1d\x29\x1c\x82\x27\xf5\xb0\x22\x42\xf1\x09\xa5\x82\x42\xfc\x20\xb0\x04\x34"
	"\x81\xab\x23\x76\x50\x51\x16\xfb\xe5\x03\x49\x8b\xd0\xbe\x3e\xb4\x76\x1e\xbb\xf8\x14\xd7\x99\x53\x22\x21\xc7\x44\x74\x19\xe8\xb8"
	"\xf2\xa2\x27\xe1\x55\x81\x0b\x6c\xd5\xa6\x8c\xa4\x13\x7d\x55\x89\xab\x09\x74\xb7\x45\xd6\x49\xa7\x25\xdd\x6e\x0d\xdb\x2e\xe5\x1a"
	"\x88\x58\x45\x25\xac\xaf\x05\xf7\x2a\x19\x87\x9a\x38\xe1\xc3\xfa\x30\x58\x87\x9a\xec\xb4\xe1\x1d\x90\xba\x6c\xdb\xc3\x5a\x29\x04"
	"\x32\x37\x5e\xbd\x4c\xf6\xbf\x45\x8b\x5a\xfb\x08\x43\x27\x35\xdf\xf9\x9e\x4f\xa2\x57\xba\xf4\x7c\x9e\x24\xdb\x03\x8e\x8d\x20\x5d"
	"\x4f\x3a\x93\x60\xff\x1a\xee\xc4\xcd\xc6\x80\x3f\x53\x38\xab\x0c\x26\x69\x23\x48\x68\xf8\x91\xf2\xb1\x03\x03\x3e\x62\xef\xda\x1b"
	"\x1b\x2b\xab\x07\x03\xed\x71\x46\x21\x31\xe4\x26\x41\x7f\xb2\x3f\x2d\x19\xf0\x6f\x82\x02\xa4\x03\x86\x53\x77\x96\x68\x0b\xdd\xc0"
	"\x6e\x21\x83\x97\x67\xc1\x62\x93\xc3\x7f\x91\xe1\x08\xcc\x30\xcd\x92\x6b\x42\xd6\x12\x67\xd6\xc6\x9e\x76\x1a\x97\xd0\x7d\x71\x14"
	"\x7e\x02\x23\x07\xe0\x89\x24\x30\x78\x01\x7c\xbc\xc0\x52\x9b\x5a\xbc\x55\x0a\x41\x2d\xf1\x4b\xe6\x35\x47\xbf\xd9\xd0\x46\x93\x7b"
	"\x32\x4c\x52\xe7\x3f\xaf\x3e\xc5\x1f\x89\xf1\x16\x1a\xef\x28\xcc\x75\xdf\xb1\xff\x38\x30\xc8\xff\x8c\x28\x4b\x7c\x47\xff\x49\x7a"
	"\x4e\x11\x14\xdf\x1c\x20\x57\x51\x37\x99\x90\x88\xf2\xa7\xac\xbd\xc4\x1d\x17\x89\xd9\x29\xe7\x26\x7d\xe1\x25\x2a\xb0\x04\x79\x30"
	"\x0b\x67\x0c\x2f\xf7"
;