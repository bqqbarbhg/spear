#include "GameShaders_ios.h"
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
	{ { 1 }, {  }, { 1,2,3,4 }, 0, 974 },
	{ { 2 }, { 1,2,3,4 }, {  }, 974, 6835 },
	{ { 2 }, { 1,2,3,4 }, {  }, 7809, 6835 },
	{ { 2 }, { 5,2,3,4 }, {  }, 14644, 7534 },
	{ { 2 }, { 5,2,3,4 }, {  }, 22178, 7534 },
	{ { 3,4 }, {  }, { 1,5,6,7,8,9 }, 29712, 2089 },
	{ { 2 }, { 1 }, {  }, 31801, 6411 },
	{ { 2 }, { 1 }, {  }, 38212, 6411 },
	{ { 2 }, { 5 }, {  }, 44623, 7110 },
	{ { 2 }, { 5 }, {  }, 51733, 7110 },
};

const SpUniformBlockInfo spUniformBlocks[] = {
	{ }, // Null uniform block
	{ "Transform", 128 },
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
};
const SpAttribInfo spAttribs[] = {
	{ }, // Null attrib
	{ "a_position", 0 },
	{ "a_normal", 1 },
	{ "a_tangent", 2 },
	{ "a_uv", 3 },
	{ "a_uv", 1 },
	{ "a_normal", 2 },
	{ "a_tangent", 3 },
	{ "a_indices", 4 },
	{ "a_weights", 5 },
};

const char spShaderData[] = 
	"\x28\xb5\x2f\xfd\x60\xdb\xe4\xed\x5b\x00\xea\x63\x80\x12\x2b\xc0\xac\xa8\x0c\xd0\x01\xc9\xd3\xe6\xa8\x22\xf2\x91\x25\x59\xc1\x8e"
	"\x60\x9d\x7b\xe6\x02\xd5\x36\x2c\x51\xcd\x4d\x15\x45\x69\xab\x93\x6d\x4a\x78\x59\x04\x41\xd8\x16\x05\xe4\x1e\x01\x1f\x01\x1b\x01"
	"\x7a\x32\xe9\x9f\x14\xd3\x91\xd4\x5e\x1b\x7a\xbe\xda\x92\x12\x75\xdb\xaa\x4e\x26\xca\x5e\x4c\xaa\x15\xe1\xa3\xaa\x35\x4d\x16\x87"
	"\x38\xf0\x6e\x51\xb9\x10\xd5\x95\x1f\x5b\xec\x4a\x34\x1a\x01\xb8\xb5\xb2\xae\xbb\xde\x96\x02\xbf\x55\x03\x85\x00\x4f\xd9\xad\x73"
	"\xe5\xfe\xe4\x76\xae\x73\xa5\x5e\xea\x2b\xb9\xe2\xc9\xd7\xaa\x52\xb7\x25\x00\x40\xa3\x91\xd1\x80\xe4\x7d\x75\xbb\x2d\x97\xe5\xba"
	"\x38\xde\x31\x8d\x73\xbd\x27\x2f\x61\x64\x58\xe3\x5e\x03\x9f\x69\x8e\x6f\xa4\x26\xdf\x97\x9a\x7e\xe9\x8c\x08\x2f\xb9\x6d\x29\x94"
	"\xab\x42\x22\x79\x1a\xa7\x4f\xc8\xb5\xf3\xac\x27\xcf\x50\x50\xe4\x3b\xd9\x4a\xed\xfc\x7e\xbb\x34\x66\xfb\xb6\x16\x59\x3f\xc6\xfa"
	"\x13\xd6\x66\xfb\x55\x71\x77\xef\xb5\xe6\x4b\x41\x21\x31\xe1\xca\xcf\x69\xb9\x26\xcb\x47\x79\x2b\x62\xcf\xb6\x92\x6b\x87\x10\x0a"
	"\x0a\xf7\xf2\x44\xe3\xb6\x2d\xbf\x24\x92\xcf\xfa\xf6\xcb\xb5\x35\x59\x5e\xf6\x4b\x4f\x2f\xdf\x5a\x65\x34\x20\xb8\x56\x5b\x67\x48"
	"\xff\x89\xcd\x5e\x35\xa6\xfc\xff\x37\x22\x34\xaa\xad\x29\x3c\xd7\xcd\xe0\xfb\xe4\xda\x9d\x37\x1a\x90\xb7\x93\x6a\x71\x5b\xd5\xfc"
	"\x95\x7c\xf9\xd5\x68\x10\xb0\x68\x16\xa8\xea\xf2\x06\x81\x8a\x8b\xa9\xce\x27\x5c\xbb\xad\x18\xde\x52\x17\x9e\xe4\x2b\x20\x10\x42"
	"\x28\xde\x3f\xbe\x91\xfe\xd5\x5e\xa9\x9f\x1b\x5b\xd5\xc4\xb6\xd8\xaa\x2a\x6f\x90\xd4\xdd\x08\x2e\x9a\xaa\x16\xfc\x09\x30\x50\xd2"
	"\xee\xab\x63\xb6\x3b\x0f\x9c\x2c\x9f\x60\x79\xad\xf8\x84\x9e\x6f\x24\x42\x42\xc2\xf6\x7a\xe1\x6a\xb1\x8e\x3e\x32\xa2\xd0\x83\xe4"
	"\x9b\x52\x3e\x50\xf4\x24\x04\x65\xf3\x1c\x5a\x43\xe4\x1d\xef\xd0\x1c\x9d\x32\x42\x1f\x66\x44\x46\xc7\x48\xef\x30\x51\xea\xae\xd6"
	"\xcb\x67\x9e\xa1\x50\x2e\x35\xf9\x04\xb9\x2b\xbe\x26\xd6\xe2\x93\x43\xc3\x02\x03\x2f\x5f\xf3\x2c\xc7\x36\x63\xe1\xa5\x77\x04\xda"
	"\x53\xae\x84\x93\x73\x3c\x7d\xe4\x84\xa6\x33\x21\x8d\x2f\x42\x93\x42\x50\x42\xb1\x71\x2a\x9d\x2d\xf5\x90\xfe\xf1\x0a\xff\xf0\xf0"
	"\xd2\x37\x40\x40\x44\x20\xb0\x4a\xe5\xf1\xce\xf1\xad\xb5\xbe\x21\xf0\x77\xbc\xf4\x11\xac\x52\x89\xd5\xc2\xc2\xf5\x21\xf2\x0c\x44"
	"\xd7\x54\x44\x73\x80\x00\xc1\xd4\x10\x79\xe7\x26\xd6\x1e\xe5\x15\xde\xb5\xf6\x8f\x7f\x94\x18\x27\xf6\x8e\xc7\x5e\xfa\xf8\x9a\xf7"
	"\x8c\xc2\xbf\x06\x36\xab\xaa\x78\x32\x60\x40\xe7\x95\xf2\xd2\x63\xee\xa1\x6b\x2a\xf2\x09\xcf\x3c\xca\x27\x50\xd5\x6e\x8b\xde\x2e"
	"\x26\x17\x5b\x4c\x26\x26\x87\x86\x73\x13\xcf\xea\x05\xd7\xd6\xa6\x69\x9a\xde\xe6\x8b\x73\x75\xaf\x49\x24\xbf\x35\x0e\x11\xf2\x8a"
	"\x7b\x01\x4d\xd9\x0a\x5c\xa3\x44\xf2\x4c\xcf\x96\xf2\x22\x49\x9f\x67\x4e\x67\x3e\x27\xa2\xef\xbd\x09\x61\x84\x50\x94\x10\x3a\xce"
	"\xf3\x44\x32\x3e\x10\xa5\x0f\x04\x1f\x07\x82\x24\x14\xbd\x28\xa3\x68\x42\x51\x64\xb0\x89\x41\x9c\xe5\x1f\xbf\xf0\x16\x69\xd5\xca"
	"\x66\xdd\x7f\x32\x91\x62\x34\x96\x47\xb3\xc6\xe2\xa6\x6c\x2c\x16\x49\x1f\xc8\x99\x4f\x46\xe8\x3c\x8f\xf3\x44\xf0\x3d\x51\xf4\xe0"
	"\x83\xf3\xc5\x88\xd2\x28\x1f\x87\x3a\x70\x5e\xb4\xf5\x8e\x88\x46\x18\x3d\x93\x3e\x33\x8a\xa2\x68\x52\x1a\xa1\xa4\x33\x92\x51\x74"
	"\x9e\x18\x35\x11\x3a\x18\x5f\x8c\xd2\x91\x8e\x8c\x22\x8d\x91\x46\xe8\x49\x28\xdf\x63\x78\x0c\xc3\x2a\x2a\x7b\x42\x33\xd8\x35\xe3"
	"\x39\xe7\x19\xe7\x78\x87\x42\xc0\xe9\xe1\x21\x12\x81\x78\x26\xc2\x3d\x3c\x13\x89\xfc\xe3\x21\x9a\xc5\xb8\xd2\x3f\x38\xf7\x78\x1a"
	"\x1d\xea\x3c\x13\x92\x11\x84\x0e\x84\x2f\x9a\xf0\xc1\xe6\xb5\x5f\xa8\x57\xb6\xbd\x57\xac\x71\x0d\x5b\x8b\xc5\xcf\x64\x18\x58\xc3"
	"\xb4\x46\x7a\x07\xb5\xb5\x3e\x80\x59\x06\xb2\x5e\x03\x11\x5f\x09\xe0\x01\x06\x87\xf4\x10\x26\x76\x5b\x90\x1e\xc2\x31\xad\x6b\x08"
	"\x13\x9d\x5f\x89\x57\x90\xbe\x76\x6c\xa9\x0b\xbf\x2e\x55\x17\xdc\xca\xdd\x0a\xf0\x4b\x29\x63\x0c\xb4\x79\xed\xb5\x9e\xad\x06\x8a"
	"\x56\xac\xe4\xde\x2b\xb9\xc7\x6a\xe7\xfc\xaa\x9d\x23\x7c\xc6\x31\x5c\x5b\x97\x0b\x06\xc6\x8b\x11\x42\x97\x65\xed\xa5\x47\x8c\x10"
	"\x62\x8d\xe3\xb1\x06\xbb\x0d\x94\xd3\x45\x6f\x27\xb1\x0b\xa7\x4f\x80\x22\xb5\x97\xc9\xed\xe4\x52\xf9\xca\x08\xad\x8b\x0f\x68\x06"
	"\x14\xde\xfa\x92\xef\xc5\x44\x8c\xa0\xa4\xe2\x27\xb7\x59\x1b\xb8\x64\x4c\x86\x41\x19\x99\x27\x92\x51\x7c\xf3\x89\x64\xf4\x38\x74"
	"\xca\x08\x27\x28\x23\x94\xf1\x81\x1e\xe8\x81\x1e\xa8\x3f\x03\xf9\x33\x8f\x3b\x0f\x42\xe8\x3c\xe8\x1c\x46\xa3\xf2\xa5\xf7\xf5\x4c"
	"\x86\x35\x8c\xf4\x8f\xe3\x67\x30\xb0\xc6\xa2\xd1\xf4\x12\xbf\xc2\x67\xf8\xe1\x96\x6f\x05\xf1\x93\xe1\xb6\xb5\xc6\x94\x87\xa1\xc6"
	"\x7a\xe9\x33\xfc\x05\xb2\xba\x77\x9d\xa8\x68\x4d\x5c\x32\x49\xfa\x67\xca\x1c\x8c\xf4\xc8\x97\x4e\x19\x89\x90\x98\x2f\x3d\x18\x20"
	"\x7d\x6b\xc7\x3b\xfe\x22\x3d\xe3\x28\x85\x9c\x27\x3a\x33\x8a\x32\x8a\x0d\xbb\x8f\x82\x35\xd2\x47\x1e\xb8\x2c\x5f\xac\x71\xe4\xdb"
	"\x5a\xc4\x4b\x4b\x7a\x09\xe9\x2f\x78\x4d\x30\xa5\x2f\x20\x1d\x23\xfd\x83\x16\xdf\xf1\x19\xd6\x38\xa6\x45\xf4\x5a\xcd\xd6\xf3\xcd"
	"\xf6\x7b\xbe\xf8\xe2\x76\xcb\x08\x5f\x93\x17\xbc\x56\x01\x82\xf1\xa8\x32\x03\x97\x2a\x44\x33\x02\x00\x00\x40\x00\x00\xc2\x09\x00"
	"\x88\xe3\x61\x16\xa4\x59\x6d\x39\x12\x40\xa1\x58\x96\xa3\x40\x8a\x23\x39\x86\x19\x64\x0c\x40\x06\x08\x0c\x00\x08\x00\x00\x20\x00"
	"\x02\x00\x24\x01\x07\x9c\xbc\x5e\xef\x2f\x6e\x29\xc4\xa2\x16\xb6\xe1\xad\xda\x74\xa4\x75\xdc\x51\xb8\xce\x5c\x16\x3d\x5e\x5f\xe3"
	"\x93\x0e\x19\x5e\xa3\x53\xea\xbd\xa7\x73\x87\x9f\x2c\x5c\x5b\x94\x40\x16\xfb\x71\xeb\x23\x70\xca\x82\x07\x1a\xb6\x28\xb9\x08\x6e"
	"\x31\x78\x67\x6d\xa6\xe6\xe4\x8e\xfb\x8a\xee\xd0\x75\x87\x66\x77\xd6\x74\x72\xc3\x6f\xde\x8a\x0d\x4a\x78\x01\x42\xec\xa6\xc8\x51"
	"\x1c\x7e\xc2\x3e\xfc\xd3\x7c\xa5\x4c\x0f\x27\x73\x81\xd3\x10\x17\xce\x5b\x38\x8a\x55\x14\x10\xfd\x5c\x8d\x37\x39\x09\x48\xa3\x3c"
	"\xb6\x6a\x85\x61\x7d\x64\x76\x9b\x5e\x06\xdd\x11\x89\x3a\xd0\xa9\x29\x9c\x4d\xf7\xe7\x04\xf0\x65\xad\x8c\x6c\x3d\x78\x9b\xce\x04"
	"\x2b\xa6\xbb\xd3\xce\x32\xc6\xca\xd2\x42\x4c\xec\xfe\x53\xa5\xa4\x18\xa3\x42\x99\x3a\x46\xdb\x40\xfc\xec\x2d\x3a\xb9\xa9\x5f\xac"
	"\xdc\xd7\x65\x82\x22\x9b\x4a\x43\xab\x3b\x9a\xfb\x83\x63\x9d\x30\x70\x07\x56\x5a\x72\x9c\x50\x8e\xb1\xc3\xa8\x29\xcd\x57\xcf\x57"
	"\xe4\x00\xa7\x8f\x81\x1a\x2a\x34\xd7\x67\xc3\x45\x57\x7b\x05\x9f\xda\x4a\x09\x0f\x60\xa1\x9d\x51\xb2\xaa\xbc\x95\xb9\x0d\xb1\x76"
	"\xf4\x3f\x17\x67\x19\xe4\xe8\xc0\xcf\x16\xfe\x27\x88\xe4\xe4\x25\x9e\x2c\xab\x59\x94\x51\x3d\x47\x34\xad\x63\xcf\xed\x3a\xee\x29"
	"\xa1\x9b\xd6\x07\xd1\xbd\x90\x8b\x7d\xd3\x36\x35\xd1\x17\xa7\x87\xf3\x95\x66\x23\x92\x71\xbd\xe5\x34\x4d\x12\x43\x1a\x68\x2d\xf5"
	"\xb3\xbc\x39\x38\x22\xab\xd3\x26\xfd\xbd\x85\x8f\x75\xad\xdb\x22\x02\x4e\x96\xd0\xba\x6d\xe4\x82\xa6\x56\xdc\x41\xe7\x7b\x28\x26"
	"\xe7\x9c\x3b\x6e\x5a\xdc\x86\xaf\xb6\x9e\x25\x49\x4d\x6c\x41\x3e\xd1\x2d\xf5\xad\x6f\x7a\xba\xda\x34\x71\x2c\x93\x3c\xed\xe2\x56"
	"\x8b\xce\x25\x63\x87\xf9\xfb\x99\xb8\x2b\xbb\xfc\x91\x8a\xb1\x0e\xc4\x05\x44\x36\x35\x42\xb6\x2a\x24\x7a\xed\xf6\x73\x87\x3e\xdf"
	"\xb1\xf7\x3f\x91\x93\xa7\x73\xd7\x5e\x38\x52\x14\xe1\xcd\x58\xd3\x06\x87\xbd\x1c\x18\xfd\x39\x98\xde\xd6\xe7\x33\xd7\xbf\x7a\xf8"
	"\x65\xda\x13\x32\xee\xb4\xc0\x07\x38\x55\x8e\xc8\x3a\xb6\x1b\x66\xe3\x1c\x11\xbf\xcc\xb5\x2d\xa2\xe7\x5a\xf0\x9d\x9a\x8b\x34\x4e"
	"\xac\xc9\xd8\xe1\x54\x1b\x08\x5e\x5a\x05\x0c\x31\x18\x31\x63\x49\x06\x53\x7b\xa3\x1f\xc0\x01\x3d\x9c\x0e\x17\xda\xd8\xb8\x0b\x5a"
	"\x28\x9c\x49\x41\xa1\xa4\x5d\x22\x48\xa0\xea\x02\x99\x8b\xd2\x08\x64\x43\xa6\xc2\xf7\x98\xc2\xde\x05\x2c\xb3\xb1\x33\x98\xad\x7a"
	"\x41\x37\xc3\x83\xb4\xfb\x79\x19\x99\xbc\xaa\xd2\x00\xf7\xb7\x95\x30\xfb\x28\x6d\x15\xc8\xd4\x95\x41\xe2\x11\xea\x93\xc2\x07\xbc"
	"\x20\xb6\x08\x14\x63\x01\x8a\x80\x3e\xd8\x49\x2b\xe2\x2c\xd8\xd4\x8a\xa8\xaf\x1c\x8e\x62\x9d\xe6\x2b\x06\xfc\x59\xc9\xab\xb9\xf5"
	"\xb1\x2d\xbe\x2a\xeb\x91\xdd\x0c\x76\xa3\xe9\x85\xa3\xdf\xe1\x32\xe9\xa0\x5c\xb5\xc5\xac\x22\xa2\x30\xbf\x7a\x68\xef\xa3\xf8\x49"
	"\xe3\x24\x87\x87\xce\x43\x79\xfc\x18\xc0\x8c\x1f\xfe\x09\x08\xce\x9f\xdb\xe4\x99\x24\x74\xa5\xc3\x83\x66\xd5\xac\x88\xb9\xe0\xd3"
	"\x8c\xb8\x38\x16\xce\xf7\xee\x44\xd7\x1c\x0e\x1a\x4c\xa4\xbf\xa3\xd5\x88\x40\x42\xe5\x6c\xe9\xa4\x4c\x24\x88\x2f\xdf\x0f\x0f\x5d"
	"\x60\xdb\x8e\x94\x3e\xc9\xa1\x0f\xeb\x0c\x16\xfc\x5c\xa9\xf1\xed\x88\xa8\xaa\x94\x24\x53\x65\x52\x6f\x08\x48\xd2\x5e\x9b\x6d\x4b"
	"\x5b\x03\x1e\x6d\x5d\x73\x80\x21\x6c\xd4\x60\x6d\x1d\xae\x45\x1b\x26\x41\x23\x40\xa1\x0d\x21\xc3\x08\x24\x83\x26\x42\xe0\x92\xc5"
	"\x67\xf1\x20\xea\x30\x61\xb5\x4b\x80\xc1\xde\x91\x82\x09\x0d\x27\xc8\x11\xd1\xce\x73\x18\xe5\xfb\x4c\xcb\xf2\xc3\x14\x10\x3a\x33"
	"\x72\x3d\xfd\x78\x69\x9c\x66\x5b\xd5\x3f\x14\x76\xf7\xa2\xc4\xe3\x31\xeb\x2c\x45\xe6\x56\xff\x89\x07\xee\xa3\x64\x6f\x4d\xcd\x4d"
	"\xb2\x68\xb9\xb3\x4e\xc1\x49\x50\x66\xcb\x18\x57\xe3\x86\x3d\x85\xba\x51\x15\x5d\x0a\x57\x19\xa4\xea\x83\x9d\xc7\x65\xba\xdd\x99"
	"\x5e\x2e\xbe\x7a\x61\xeb\xe4\xbc\xe6\xf3\x48\x1f\x9d\x83\x9c\x1a\x26\x03\x51\xc2\xd4\x42\x41\x2e\x64\x65\x47\xa4\x01\x0e\x1e\xb5"
	"\x6e\xc8\x85\xb0\x68\x20\xda\xbb\x45\xd7\x0f\x7b\xbd\xce\x4c\x88\x6a\xd4\x5b\x02\x79\xc1\x09\x6e\x12\xd6\x3b\xd1\x3f\x81\xd0\xc6"
	"\x9a\x74\x0d\x24\x55\x9a\x33\x1e\x12\xdd\xab\x05\x5f\xb2\x89\x81\x28\xa7\xdc\xfa\x7f\xa4\xb6\xc5\x8d\x05\x4a\xe8\x2e\x48\xcd\xcb"
	"\xff\xda\xe5\x2a\xac\x03\x17\xde\x22\x2f\x96\x7d\x9c\xf5\xe9\x33\x63\x74\x7d\xd1\x5c\xd9\xda\x7c\x81\x00\x14\xdb\x37\x64\xfe\x01"
	"\xfb\x55\xc3\x58\x27\xe4\xbd\x91\xb0\xde\x3a\xa1\x53\x69\xc7\x38\x84\x86\xb6\x14\x4c\x24\x24\x86\x9e\x45\x18\x63\xa1\xfc\x37\x06"
	"\x0e\x51\x24\xbb\xb1\x39\x3a\x0d\x96\x12\x6d\x1d\x46\x58\x50\xc0\xb7\x59\x78\x26\x88\xd7\x73\x46\xd0\x7a\x1c\xe2\xc9\x20\x8f\xc7"
	"\x77\x7f\x71\x7c\xb0\xb1\xa9\x17\xcb\xc6\x17\xd6\xca\x84\x8b\x4d\x0d\xc4\xb9\xc3\x86\xc4\xa1\x7c\x38\xa3\x79\xa1\x7e\x20\x6e\x71"
	"\x88\x44\x0d\xd7\x29\x76\x51\x05\xdc\xe8\x6e\xa0\xb1\x5e\x04\xf1\x4e\xc1\x35\xf4\x04\xda\x53\xd0\xcc\xc2\xc6\xef\xf8\xe6\x2f\x0d"
	"\xec\x1b\xd6\x9e\x13\x19\x25\xa1\x38\x7e\x69\x06\x93\x86\x31\xad\x38\x5f\x89\xe9\x5e\x0a\x07\x51\x80\x12\xf8\x55\x31\x1c\x68\xd3"
	"\x75\x3f\xad\x3a\xb0\x27\xa8\xb4\xa4\x8b\x13\x14\x2b\x91\x26\xee\xc2\xb3\xd6\xa8\x4f\x8a\xae\xf7\x53\x99\xbe\x12\x2c\xa8\xcf\x24"
	"\x61\xfc\x26\x98\x2c\x03\xe1\x9c\x25\xbc\xbc\x6a\x2a\x53\xa1\x48\x75\xf3\xae\xe7\x66\xe2\xc9\x50\x2f\xc5\xbd\xa9\x90\x0c\x64\x9b"
	"\x5d\x18\xf4\x71\x94\x1e\x6b\xa0\x1b\x50\x85\x6b\x94\x31\xc8\xea\xe9\xf1\x0e\x7d\xef\xa6\xcb\x86\x80\x09\x50\x26\x1a\x2c\xed\xbb"
	"\xf1\x92\x7f\x0d\x9e\x30\x56\xb0\x97\xa2\x7e\x59\x95\x83\xee\x0d\x5a\x2d\xd7\x56\x0d\x1e\xd0\x76\x68\x5f\x12\x6d\xcc\xf9\xa0\x6b"
	"\x16\xb4\x1c\xe6\xf7\xf7\xab\x0c\xcf\xeb\xee\x48\x36\xec\x6e\x9e\xc7\xc4\x85\xb0\xe6\x97\x34\xc9\x31\x63\x88\xc7\x34\x44\x0a\x7b"
	"\x23\xbc\x15\x1b\x7c\xef\xb4\x90\x93\x85\xc7\x04\x47\xd5\x76\x64\xfd\xf4\xd9\x98\xc8\x2c\xf9\x25\xe9\x84\x48\x28\x2a\xe2\xf4\xb0"
	"\xc0\x5f\xb1\x75\xa8\x97\x61\x38\xa2\xe1\x84\x17\xab\xb9\x63\x93\xc5\x67\xfe\x97\x0a\x47\x69\xf2\xf7\x1b\x74\xd9\x95\x51\xc1\x97"
	"\xe0\xc3\xe9\x8c\xfd\x77\xc0\x8e\x66\x52\xcf\x87\xc7\xbe\x50\x3f\x25\x9f\x28\x3f\x28\xe5\xdc\xa0\x63\x63\xb0\xfd\xd5\x36\x97\x0b"
	"\x1a\x77\xf1\xa5\x02\xa9\xbe\x43\xef\x12\x0c\x80\x33\x9c\x96\x7b\x73\x10\xfe\xba\x24\x8d\xcc\xa2\x71\x97\x03\xd7\x05\x8e\x1b\x65"
	"\x22\x23\x71\xaf\xe6\x3e\xbc\x37\x21\x80\x22\x9e\x97\xca\x06\x8a\x42\x8e\xa4\x2f\x7a\xfd\xd0\xfd\x50\x5f\x38\x09\x59\x5b\x4f\x9a"
	"\x8a\x2c\x81\x88\x88\x3c\x0d\x86\x3a\xa7\x41\x00\x80\xb9\xaf\x2b\x27\xdf\x69\x25\x73\xdc\x3f\x06\xd7\xd8\xee\x02\x05\x43\xc0\x9b"
	"\x39\xc9\x15\x5c\xf8\x14\x0c\x9a\xc4\x82\x6a\xc8\x2d\x61\xac\x9d\x98\x87\x5a\x2c\xac\xfd\x22\x77\x9a\xa8\x56\x44\x0f\x3c\x91\x5f"
	"\x21\xa7\x07\x2a\x2c\xaa\xfa\x39\xd7\xdf\x95\xc5\x65\x66\x3f\x0c\xd8\x0a\x2b\xc0\xb0\xef\xf1\xd7\x2c\xea\xde\x3c\xfd\x06\x70\x48"
	"\x11\xf4\xf8\xd9\x7b\xc2\xc7\x1e\x62\x4c\x02\x2a\x32\x44\x96\xa2\xcb\xe3\x56\x68\x3b\x45\x2e\xb1\x74\x4a\xc1\xbf\x31\x84\x79\x09"
	"\x89\x54\x91\x65\x53\x3b\x62\xba\xac\x83\xe1\x0d\x58\x81\x1b\x0d\xf5\x03\xed\x35\xbe\x1a\x72\xc2\xdb\x3f\x91\xc4\x15\xe0\xc0\x35"
	"\xe8\x7c\xb0\x04\x86\xb5\xaa\x40\x4a\xf5\x71\x48\x59\xeb\xbe\xac\x4e\x0a\x44\x87\xa7\x1f\x9f\xb8\xf9\x06\xa6\x07\xa5\x1c\x17\x3f"
	"\x1b\x87\x93\xb8\x9b\xe2\xe5\x55\x52\x97\x5e\xc0\x47\xdb\xbf\x84\x46\xaa\xa8\xe3\x36\x14\xb0\x74\xfd\x9b\x29\x8f\xec\xd0\x67\x4f"
	"\x37\x82\x57\xd8\xa3\xc1\x1e"
;