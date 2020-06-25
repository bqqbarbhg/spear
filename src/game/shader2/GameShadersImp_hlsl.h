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
	{ { 1 }, {  }, { 1,2,3,4,5 }, 0, 1514 },
	{ { 2 }, { 1,2,3,4 }, {  }, 1514, 4539 },
	{ { 2 }, { 1,2,3,4 }, {  }, 6053, 4539 },
	{ { 2 }, { 5,2,3,4 }, {  }, 10592, 4873 },
	{ { 2 }, { 5,2,3,4 }, {  }, 15465, 4873 },
	{ { 3,4 }, {  }, { 1,6,7,8,9,10 }, 20338, 1825 },
	{ { 2 }, { 1 }, {  }, 22163, 4040 },
	{ { 2 }, { 1 }, {  }, 26203, 4040 },
	{ { 2 }, { 5 }, {  }, 30243, 4374 },
	{ { 2 }, { 5 }, {  }, 34617, 4374 },
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
	{ "a_tint", 4 },
	{ "a_uv", 1 },
	{ "a_normal", 2 },
	{ "a_tangent", 3 },
	{ "a_indices", 4 },
	{ "a_weights", 5 },
};

const char spShaderData[] = 
	"\x28\xb5\x2f\xfd\x60\x4f\x97\x6d\x47\x00\x0a\x4e\xb8\x0e\x2b\xc0\x8c\xac\x0e\x10\x2d\x07\x59\x08\x01\xe6\x94\x27\x85\x8c\x1d\x9d"
	"\x70\x41\x18\xd3\x56\x5b\x02\xae\xed\xdb\xd3\x11\xd8\xcd\x01\x10\xcf\x2f\x9f\x20\x08\x82\xc4\xfe\xed\x44\xdd\x00\xe2\x00\xe1\x00"
	"\x52\x64\x50\x91\x92\xe0\x8a\x45\x93\xf5\x21\x94\xe8\x3c\x82\x9d\xb4\x79\x9b\x4f\xd3\xc4\x53\x31\x18\x09\x87\x87\x94\x34\xaf\xf0"
	"\x28\xc9\x17\x75\x8f\x83\x37\x88\xc1\x88\x52\xd5\xb7\x2a\x9e\xaa\x3b\xf4\xf9\x90\xa6\xbc\x96\xb9\x5d\xb3\x42\xf1\x40\x47\x9c\xe4"
	"\x14\xa3\xb5\xf0\xee\x3d\x63\x15\x4f\x05\xa3\x24\x1f\x45\x29\x3e\xea\xde\x96\xd1\x7b\xb9\xf5\x4e\xf1\x44\x78\x60\xf7\x2c\x9a\x4c"
	"\xc1\x95\x4c\xab\x66\xef\x2b\xd5\xfe\xa9\xee\x19\x3f\xa0\x19\xc6\xc1\x19\x46\xcb\xcb\x75\x6b\xab\xa9\xf9\x02\x9e\x65\x9c\xd1\x1a"
	"\xcd\x95\xe7\x9a\xc9\x70\x96\x0b\x76\x51\x26\x5d\xdd\xb9\x6e\xb0\x4d\x33\x36\xd4\xe6\x73\xb2\x96\xcd\x46\xa3\xd9\x4e\xa6\xd7\xf3"
	"\x54\x13\x4f\x45\xd9\x69\xc3\x61\xf4\x73\x6e\xad\x6a\x34\xba\x31\x7a\x4a\x5e\x9a\x65\xac\x55\x11\xa2\x20\x41\x82\x86\x05\x06\x2a"
	"\x17\x70\x0d\xdb\x6d\x35\xde\x30\xfa\x69\x53\x7c\x33\x8a\x27\xc2\xb5\xc5\xe6\xbd\x19\xb0\x09\xe5\xfb\xc2\xe5\xad\x03\xd0\xad\x5f"
	"\x9d\x46\x33\x29\xc4\x76\x69\xe4\x7a\x24\xd9\x18\xf3\xb0\xe0\xc0\x01\x0c\x72\x75\x10\xd5\xbd\x43\x87\x31\xf3\xbe\x78\xcc\x4c\xaa"
	"\xbb\xf5\x99\xf0\xc0\xc8\xc6\x98\xeb\x51\xcc\x44\x5c\xec\x85\xb8\x50\x30\x87\x4f\x0e\xa8\x00\x0d\xe8\xa6\x90\xad\x59\xbc\x9d\x9a"
	"\x68\xae\x19\xbd\xc8\x88\xa1\x1b\x86\x6d\xba\x72\xd0\x1b\x42\x63\x25\x19\xa7\xdd\xda\xd9\x09\x9c\xa6\x08\x4d\xb3\x39\xa5\xd4\xf4"
	"\x6e\x06\x8c\xa9\x96\x4b\xa7\xde\x34\x56\x37\xdd\x94\x5c\xef\x96\x51\xca\x86\x4c\x18\x9b\x0f\x70\x30\x86\x42\xe8\x0f\x58\x80\x4c"
	"\x35\x73\xbb\xab\x95\x98\xf1\xc5\x17\x0c\x9f\xda\x55\x93\x69\x05\xc0\x33\x51\x59\xe7\x40\xeb\x76\xc3\x6d\xb8\x47\x4f\x6b\x9c\x2f"
	"\x87\xce\xb1\x16\x89\x4e\x27\xc1\xe8\x51\x97\xcb\x6e\x5d\xfa\xff\x54\x92\x25\x29\x9e\x6a\x73\x18\xdb\xb4\x66\xb1\x4d\x3f\xcd\xd2"
	"\x2c\x72\xc8\x49\xc9\xd8\x98\x93\x9c\x0b\x47\x83\xe1\x78\xcc\x46\x00\x47\x83\x7a\x48\x11\x72\x44\xc7\xf4\x78\x4c\x8d\x46\x0c\x8c"
	"\xc7\x63\x4a\xf1\x75\x54\x05\xbb\x0a\x6e\xbe\xd8\x6c\x34\xab\x1a\xde\x13\xcf\x44\x8c\xf0\x4f\x44\x11\x82\x1e\x89\x5c\x97\xe4\xd1"
	"\x03\x8c\x3c\xba\x5e\xdd\x34\x73\x9b\xc2\xeb\x8b\x4b\x5e\x95\xb0\xd6\x23\x55\xc2\xe1\xe2\x52\x8a\x8f\x72\x5a\xdd\x9b\x76\x12\xec"
	"\x48\x74\x12\x7c\x8f\xde\xad\x9b\x2a\x91\xac\x2f\x20\x4c\xc6\xb8\xa1\xab\x95\x89\xda\x02\xf3\x72\x08\x64\x83\xe2\x59\x80\xac\x43"
	"\xa0\xe4\x43\x85\x4f\xd1\xdb\x18\x02\x65\x6b\x3b\x79\x82\x75\xba\x6f\xc5\x70\x34\x67\x31\x54\x81\xd5\xd8\x9c\xe0\x3e\x95\x83\x69"
	"\x9a\x53\xca\xca\x41\xfc\xe2\xd6\x23\xdd\x14\x73\xaa\x6a\x6a\xb9\x1e\x26\x93\xba\x29\x77\xe9\xa6\x5e\xc1\xc1\xe1\xba\xa2\x68\xdd"
	"\xe1\xd3\xa3\x47\x99\x8c\xc5\x5c\x3e\x65\x7a\xb9\x4b\xdd\x34\xe6\x00\x51\x50\x40\x80\xc2\x09\x7c\xc0\x83\x31\x31\x02\xcc\x07\x43"
	"\x52\x52\x9a\xa0\x28\xcd\x0c\x6b\x15\xb3\xb0\xbe\x78\xc2\x23\x91\x5b\x87\x48\x24\xd6\x25\x9d\x46\xb3\xf8\xa1\xbf\x9d\x21\xe8\x07"
	"\x90\x21\xc5\x07\x37\xd4\xb6\x6d\x6a\xb4\xbd\x35\xda\x9e\x59\xd6\xb4\x98\x65\x0d\x4e\x19\xc1\x48\xa1\x78\xde\xb4\xe1\x8d\xc3\xe1"
	"\x48\x48\xc4\x39\xa5\xec\x74\x16\x73\x4a\xd9\x49\xd0\x3b\xa9\xb3\x1c\x9c\x9f\x6a\xea\xec\xa9\xf9\x90\x0d\x2c\xa7\xa4\xae\x1a\x2e"
	"\xb0\xd9\xdb\x80\x2b\x98\x9b\x31\x47\xb3\x11\x94\xca\xc1\xa8\xae\x17\x8b\xd1\xac\x66\xb9\x29\x39\x9d\xfd\x48\xea\x47\x51\x4b\x09"
	"\xe3\x7d\x24\xd1\x49\x9b\xf5\x78\x9f\xf0\xce\x8d\xf7\x69\x36\xbb\x8d\xce\x7d\x19\xe3\x9b\x89\xd7\xd1\xb9\xdf\x28\x1d\x9a\x33\x03"
	"\x64\x72\x3a\xd0\x7a\xaa\x0e\xf4\x3c\xba\xa2\x89\xb5\x80\xd4\xad\xc5\x42\x15\x6f\xfd\x3d\x63\xc8\xfa\x84\x38\xd6\xa3\x08\x4e\x6f"
	"\xcb\xf5\x66\x3a\xc9\x4a\xf8\xc5\xc0\x5a\x3a\xd2\xa2\x93\x1e\x3e\x6d\xd6\x27\xd6\x23\x34\x45\xb0\x69\x06\x5d\xc2\x3a\xc3\xa6\x19"
	"\x8c\xac\x33\x78\xf3\xd0\x3b\x9d\x04\xa3\xf7\x1e\x36\xcd\x3e\xa9\xb7\x35\xab\xd5\xcb\xe9\x9a\x21\x25\x65\xc3\x11\x82\x49\xa8\x52"
	"\x03\x55\x2a\xa6\x66\x46\x02\x08\x92\x20\x00\x92\x09\x22\x8e\xa8\x61\x1a\xf5\x98\xe9\x12\xc0\x91\x30\x4d\x82\x28\x88\x31\x85\x14"
	"\x31\x08\x8c\x48\x00\x00\x10\x00\x40\x08\x08\x00\x02\x00\x01\x02\xcd\x93\x11\x0e\x36\x82\x0a\xe2\x70\x2b\x09\xcd\x01\x67\x00\xcb"
	"\x57\xb1\xf9\xd6\xa7\xa0\x6d\x3e\x70\xfb\xb1\xeb\x2c\x4d\xe0\xf6\xc0\x44\xd2\xb8\xfd\x8f\x61\xe7\x48\xd0\x50\xd7\xc1\xb3\xed\xe1"
	"\x56\x4d\xb5\x0f\xda\xbc\x16\xdd\xda\x7d\xf9\x50\x61\x61\x4a\x04\x88\x4a\x0d\xd0\x49\x04\x22\x51\x6f\x68\x0e\x20\xd2\x2f\x7d\x88"
	"\x68\xdd\x23\x2a\x4e\xbf\xae\xe2\x35\x84\x73\x0a\xda\x50\x80\x54\x4f\x5c\x43\x30\x94\x31\xe2\xd5\x67\xcf\xad\xd2\xc3\x11\x25\xfe"
	"\xc1\x69\x18\x53\x2b\x36\xa2\xfe\x85\x98\xf6\x56\x5d\x8a\xe0\x1a\xf5\xed\x29\x35\xa3\xc7\x01\x90\x8a\x40\xab\x2e\xe9\xd2\x7b\x77"
	"\xad\x3f\x5f\xc4\x95\x9e\x13\x83\xca\xd7\x69\xfa\xd4\xbc\x07\x2e\x7a\x92\xab\xcc\x3c\x69\x3b\xa0\x39\xb9\x06\x1c\x92\xaa\x2a\xe8"
	"\xe7\xca\x23\xc0\x27\xa2\x9a\x76\xa8\xe4\x5e\xfd\x85\x47\xa1\x6a\x30\x26\xf3\x72\x3e\x0a\x60\xa5\x81\xcd\xc3\xc2\x54\xec\x38\x7e"
	"\x1a\x50\x14\x85\x50\x8a\x3e\x60\x73\x03\xc6\x43\x26\x41\x6e\x37\x29\x19\xc4\xb3\x1d\x12\x8b\x96\xdf\x73\xe8\xd5\x7b\xae\x3e\x0a"
	"\x7b\x84\xd6\xc2\x74\x77\x13\xa7\x4b\x20\x8c\x69\x03\x00\x19\x80\xd9\xd7\x7d\x7c\x8e\x60\x35\xce\xd3\x92\xaa\x7b\xa2\x4f\xd0\xe5"
	"\x06\x6c\x86\xd9\xb5\x64\xc9\x31\x09\x0d\xc6\xcc\xcb\xb6\xaf\xb8\xef\x66\xda\x40\xd8\xfc\x59\x37\x68\x17\xc7\x75\x51\x10\xe0\xdd"
	"\x76\x63\x5b\x26\x84\xf6\xec\x31\x6f\x84\xb3\xb3\x3a\xa5\xe2\x3a\xe5\x62\x8c\xe3\xf3\x03\x1c\x41\x56\x8c\xec\xf0\xad\x98\x02\xad"
	"\x10\x9b\xf3\xc0\x98\xaf\x1c\xe4\xda\x59\x2f\xf0\x07\xed\x40\x69\x7c\x05\xfe\x9f\x1c\x7e\x10\xac\x1a\x07\x32\x4b\x5d\x3c\x1c\xa8"
	"\x60\x10\xdf\x2e\xd2\xbe\x81\x36\xe3\x80\x85\xb0\x53\x40\x92\xb3\xb1\x13\xad\x15\x72\x91\xa4\x4d\x34\x71\x48\x9e\x38\x3e\x14\xa7"
	"\x5d\xa9\xfc\x2a\xdd\xe8\x42\x03\xbe\x25\x60\x98\x58\x24\xa5\x20\xc6\x3b\x2c\xcf\x4e\x11\xc7\x59\x8a\x77\xb9\x3d\xeb\x5d\x03\xa8"
	"\x96\x03\xd1\x1f\x09\x58\x30\x38\xb4\x42\x36\xd5\xc8\xf8\x49\xed\x03\x91\x9d\x67\x40\x2f\xe2\x65\x96\xc8\x54\xb8\x7c\xf2\xfa\x33"
	"\xd2\x11\x99\x78\x65\x45\x63\x26\xc6\xb5\xd5\xa3\x64\xa7\x01\x9f\x91\xe4\xbc\xd1\x92\x43\xda\x30\x57\x32\xbe\x6a\x18\x74\x25\xe4"
	"\x40\xea\xb9\x06\xfc\x57\x70\x27\x99\x72\xce\x11\x5a\xc3\xb5\x2a\x51\xd5\x24\xca\x54\xfe\xe7\xff\x1a\x68\x08\x2c\x29\x8d\x90\x1c"
	"\xfa\x23\x32\x07\xf5\x06\x61\xd4\x4d\xd7\x0a\x9d\x30\xe1\x25\x10\x77\x96\x75\x91\x26\x10\xda\x4a\x24\x9b\xa9\xe7\x0f\x8f\xc3\x5d"
	"\x9e\x98\x37\x8a\x09\x85\x77\x8c\x5b\xde\xcb\x91\x0d\x10\xb5\xf2\xf8\xe7\x24\x2f\xe4\xd7\xf4\x6e\xeb\xf1\xe1\xe2\xc9\x23\x90\x4f"
	"\xe6\x5a\x4d\x16\x48\x61\xb5\x5e\x24\x48\x54\x0c\x9b\x5f\xc8\x6a\xa0\x82\x75\xb8\x8f\xe7\xbc\xc5\xe0\xd0\x34\xaa\x1e\xb9\xb5\xad"
	"\x21\x24\x9c\xfe\xe2\xb8\x53\xa8\x43\xd8\xf4\x21\xfa\xbb\x00\x65\xf4\xe7\xf6\xaf\x06\x40\xd8\x4c\xcf\x35\xdc\x1a\x01\x43\xf0\xeb"
	"\x11\x1c\x48\xb6\x09\xda\x65\x61\xab\x8d\x8a\x2a\xd7\xa1\xbf\x3c\x8e\x88\x76\xbe\xa7\xe3\xc1\xf3\x5e\x09\x9d\x50\xf6\x07\x16\xa9"
	"\x31\x32\x2d\x1a\x63\xa7\x66\xbf\x8c\xa1\x96\x65\xed\xa8\xc6\xc4\x44\xcc\x5d\x92\x6c\x82\x51\xec\xbe\x75\x03\xc8\x9e\x72\xef\x98"
	"\xac\xd1\xc1\x0e\x6e\xb5\x48\xb8\x45\x2c\x44\xe7\x50\x4b\xf1\x19\x02\x09\xbf\x86\x12\x10\x1e\x36\xe2\x06\xf9\xcd\xad\x80\x64\x8f"
	"\x93\x71\xf3\xc8\x09\x85\x11\x04\x18\x54\x0d\x76\xfc\x7a\x6b\x89\xf0\xb5\x0c\x0d\x68\xde\x2e\xc3\x6a\x7d\x3b\xa0\x3b\x31\x16\x27"
	"\x03\xad\x49\x1f\x15\xb4\x41\x51\xe8\xd4\xb5\x80\x28\x32\x81\x53\x57\x61\x5e\x1f\x13\xd6\x77\x82\xf5\xa9\xe1\xab\x9f\x50\x7a\x82"
	"\x23\x50\x29\x12\xcc\x2c\xc6\x1a\xbe\x3e\x2b\x50\x39\x64\xc8\x7d\x99\x31\x78\x48\xa6\x2d\x67\x10\x97\x1a\x2c\x87\x74\xa2\x95\xa1"
	"\xa8\xff\x78\xd9\xb8\x46\xba\xba\xde\xe6\x8f\xb0\x3f\xf9\x14\xb0\x06\xba\x70\x71\x4c\x96\x6d\xca\x20\x3d\xef\x70\x88\x0b\x5c\xe0"
	"\x89\xa1\x3c\xd4\xf6\xcb\x3c\x27\x3b\x94\x8a\xda\xfd\x13\x0d\x07\x10\x8d\x46\xed\x8c\x2b\x82\x58\xca\x29\x23\xfb\x38\x8e\x46\xeb"
	"\x0f\x39\xa0\x93\x94\x4b\x3f\xf8\x47\x8f\x11\x0d\x3e\xb2\xbe\x09\xf9\x33\x87\xf1\x03\xd9\xdd\xeb\x5c\xab\x8f\x22\x17\x6c\x36\x6c"
	"\x1f\xa9\x98\xcf\x9d\x13\x7c\xeb\x0f\xcb\x10\x0f\x09\x8c\x2d\xec\x81\x83\xec\x77\xcb\x8e\x27\x05\xd5\x2e\x35\xe8\xe7\x01\x3b\x8a"
	"\x13\x17\x19\x2c\x21\xaf\x47\xd3\xdc\x53\x38\xfa\x18\xd1\xbd\xdb\x75\x5a\xfc\xf5\x68\x4f\xf6\x72\x12\xc3\x7d\x22\xf1\x88\x5d\xf3"
	"\x22\x44\x78\xa8\xc9\xb0\x7b\xc6\xb6\x78\xb8\xaa\x3d\x67\xb7\x2a\x2d\x78\x40\x5e\x95\x59\x46\x25\x9a\x7b\x07\xb9\x8c\xe7\x46\xe7"
	"\x4e\x64\x27\x1c\xe9\xc3\xf6\xde\xa8\x95\x67\xdb\x66\xcd\xfe\x2b\x63\xdf\xb2\x68\x90\x63\xcf\x2d\x70\x95\xf4\x9d\xc7\x6c\x2f\x3c"
	"\x06\x54\xe8\xf3\x6c\xfc\x60\x70\x63\x14\x0a\x2a\xd2\xbd\x03\xf4\x82\x51\x40\xd8\xd9\x70\xf6\x6b\xa6\x66\x87\xe1\xf5\x6c\x96\x81"
	"\xdc\x1e\xb1\x8d\x35\xb4\xf5\x49\x50\x87\xee\xd8\xf7\xa2\x7c\x4e\x58\xcf\x6b\x2c\x48\xc3\xbd\x2c\x9f\x56\x79\xee\x76\x52\x7f\xcc"
	"\x06\x30\x38\xf0\x50\x48\x0c\x89\x35\xbd\x45\x86\xf5\x23\xe7\x80\x9c\xd0\xd3\x86\x18\xe4\x19\x2c\xd8\x87\x2b\x99\xf3\x3c\x71\x2b"
	"\xda\x5c\x19\x33\xfd\x24\x56\xe2\x16\xc0\xb1\xff\x09\xc0\x32\x30\x0b\x93\x00\xe1\x0b\x0c\xe1\x05\x20\xc3\x60\xfb\x8b\x3c\xe1\xd3"
	"\x18\x5b\x97\x17\x39\x12\xcc\x26\x45\x9a\x62\xca\xe4\x49\x9d\x6d\x7b\x35\x55\x7f\x0c\xac\xb7\x76\x02\x03\x1b\x4f\x5b\x12\x9e\x3d"
	"\x4e\x66\xfe\xc9\xce\x4f\x8b\x03\xcb\xfc\xd1\x8b\xbe\xc4\x07\xda\xaf\xec\xc8\xdd\xa0\x6e\x12\x92\x80\xc5\x6f\x99\x85\x22\x97\x5f"
	"\x32\xf0\x3a\x47\x5d\x6c\x32\x5a\xfc\x00\x76\x6a\x3c\x5c\x58\xe1\x6b\x58\x71\x66\xf0\x66\xf9"
;