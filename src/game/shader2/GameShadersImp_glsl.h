#include "GameShaders_glsl.h"
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
	{ { 1 }, {  }, { 1,2,3,4,5 }, 0, 1188 },
	{ { 2 }, { 1,2,3,4 }, {  }, 1188, 4219 },
	{ { 2 }, { 1,2,3,4 }, {  }, 5407, 4219 },
	{ { 2 }, { 5,2,3,4 }, {  }, 9626, 4487 },
	{ { 2 }, { 5,2,3,4 }, {  }, 14113, 4487 },
	{ { 3,4 }, {  }, { 1,6,7,8,9,10 }, 18600, 1527 },
	{ { 2 }, { 1,6,7,8 }, {  }, 20127, 4179 },
	{ { 2 }, { 1,6,7,8 }, {  }, 24306, 4179 },
	{ { 2 }, { 5,6,7,8 }, {  }, 28485, 4447 },
	{ { 2 }, { 5,6,7,8 }, {  }, 32932, 4447 },
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
	"\x28\xb5\x2f\xfd\x60\x03\x91\xfd\x45\x00\x4a\x4b\x28\x0e\x2b\xc0\x8c\xac\x0e\xbc\x2c\x47\x21\x8d\x2c\x3a\xe6\x49\x26\x13\x47\xa7"
	"\xba\xf7\xb1\xb9\x81\x56\xcd\x75\x43\xec\x1b\x1a\x82\xd9\x29\xf1\xfc\x85\x01\x00\x00\x80\xf2\x57\x03\x11\xd4\x00\xd6\x00\xdb\x00"
	"\x92\x5c\x3c\x1c\x8d\x07\x44\x3a\x04\x3c\x1c\x14\x24\x8a\xcf\x23\x3e\x26\x08\x64\x72\x38\x62\x14\x1b\x54\xa4\x24\xf9\xc1\x92\xd1"
	"\x19\x84\x9b\x2e\x82\x20\x17\x3d\xc7\x99\x70\xaa\x46\x83\x71\x78\x48\x45\xf2\x09\x8f\x91\xf4\x20\xee\x6d\x19\x7b\x1a\x8d\x18\xd5"
	"\xec\x56\x85\x53\x71\x7f\x2e\x21\x91\x94\x8e\xb5\xb6\xab\x56\x28\x1c\xf8\x88\x93\x34\xc5\x38\x09\x9c\x7b\xc7\x58\x85\x53\xc1\x18"
	"\x49\x87\x62\x14\x0f\x71\x0f\xbb\xe8\xbd\x7c\x9e\x53\x38\x12\x0e\xe4\x9e\x25\xa3\x1d\xf2\x83\xa6\x15\xdb\xfb\x4d\xf3\xc5\x38\x4e"
	"\x35\xf1\x50\xed\x53\x9b\x5e\x0a\x47\x22\xc3\x54\x3f\xfc\xd2\x19\x63\xb1\x76\x18\x29\xc9\xda\x99\xe6\x94\xe6\xd2\x5a\x34\x5b\xcb"
	"\x8e\xa5\x34\x5a\x85\x5e\x4e\x1b\xd7\x0c\xa6\xbb\x42\x4f\xc9\x4b\x2f\x9d\xb1\x54\xd9\x2c\x63\x15\xfa\xe9\x52\x35\xc7\xf6\x30\x4e"
	"\x9c\xed\xa6\x97\x7c\xdf\xd6\x72\x47\xaf\xe3\xc4\x69\x3a\xce\x36\xd3\x55\x13\x20\x12\xd2\x90\x28\x29\x9a\x9e\x18\xc9\x2b\xe3\x6a"
	"\x2d\xe6\x17\x67\x9c\x21\x22\x91\xcc\x4b\xb8\x4c\x66\xf1\x3f\x7f\x5b\x47\xc8\xfe\x7c\x9e\xc2\x65\x32\x92\x42\x60\x97\x32\x50\x0e"
	"\x49\x76\xb6\x1a\x1a\x24\x48\x50\x89\x50\xb8\x87\xe2\x22\x66\x5a\x3c\x62\x2a\x49\x71\x9f\x97\x0c\x07\x32\xec\x6c\x51\x0e\x45\x4c"
	"\xc4\xc5\x5e\x88\x8b\x43\x65\x01\x02\x25\xe0\x01\xdd\x34\xb2\xb1\xcd\xd8\xa9\x69\x42\x75\xbd\xe8\x45\x4a\xd1\xa2\x58\xa5\x82\x65"
	"\xd5\x42\xb6\x21\x32\x56\x72\x69\xf8\x56\xce\x4e\x22\x32\x7a\x65\xf4\x6e\x85\x6c\x55\xcb\xa5\x8f\xc8\x2a\x32\x59\x56\x27\xd8\xb2"
	"\x45\x21\xb2\x47\x26\x5d\x2f\x76\xd1\xcc\x71\x2a\x9d\x6f\xbe\x54\x32\xb6\xd8\x62\x8b\x2d\xb6\xd8\x02\x98\xdf\x20\x9d\xb7\x1a\x4d"
	"\xa3\x41\x2f\xcb\xb2\xbe\xfc\xf9\x66\x2d\x9b\x8e\x8b\xe6\x15\xf3\x8a\x47\xfd\x39\x7c\x6f\x52\xb9\x99\xff\x9f\x87\x0f\x5b\x7a\xdd"
	"\xb2\x59\x03\x3d\x24\x49\x49\xd1\x21\xa7\x15\xe4\x2a\xa8\xf9\x22\xe1\x9a\xf9\x5c\x23\x3f\x5f\xb5\x96\x65\x8c\xab\x18\xbd\x2a\xce"
	"\x7b\xc2\x99\x90\xf8\x27\x14\x4e\x55\x35\x9e\x77\x51\xc3\xcb\x2c\x74\x2e\x0e\x45\xcc\xf3\x5c\x2d\xab\x9a\x81\x81\x42\x91\x1c\x6a"
	"\x50\x40\x86\x43\x14\xca\x17\x97\xbc\xba\xb1\xf3\x0c\x55\xc2\x19\x0a\x8c\xc3\xc5\xa5\x28\x3e\xaa\xe1\xba\x37\xe5\x22\x08\x63\xf4"
	"\x48\x68\x2c\xde\x6d\xb8\x08\xbe\x43\xcf\xca\x92\xaa\xb5\xcd\xd5\xfd\xf9\xc5\x63\xe9\x9c\x11\x70\xf5\x61\xa2\xf6\x80\xbc\x9a\x02"
	"\x09\x81\xc2\x21\xf0\xcc\x47\x70\xd0\x4e\x05\x13\x20\xf4\x76\x8e\xe0\xb0\x33\x3b\x39\x63\x9e\xee\x5b\xad\x2d\xd3\xac\x45\x19\x6c"
	"\x88\x8c\xcd\x58\xbd\xe0\x3a\x15\x84\x69\x92\x52\x92\x2a\x08\xf1\x8b\xcf\x47\xba\xa9\x75\xaa\x66\xd4\xf2\x8d\x2d\xa5\x61\x32\xa9"
	"\x9b\x6a\x97\x6a\xea\x13\x16\x16\x28\x94\xf9\x43\x87\x30\x18\x6b\x69\x7a\xb5\x4b\xc5\xb2\xca\x42\x84\xc1\x81\x01\x09\x37\xf0\x02"
	"\x90\xc6\xd4\x10\x10\x5b\xb7\x75\x0f\x70\x43\x8c\x61\x32\x83\xf7\xce\xe0\x2d\x2f\x8d\x71\xbc\x34\x86\x52\x82\x20\xa4\x50\x38\x58"
	"\x96\xb1\x6d\xdb\x36\x9b\x28\xa5\x24\x75\xdd\x42\x4a\x49\xe2\xa2\xe7\x5c\xc4\xd9\x2d\xeb\x79\xa7\x9a\x9e\xb5\x0a\x47\x42\xa4\x07"
	"\x96\x53\x59\xc5\xf9\x00\x66\x2f\x82\xfc\x60\x5d\x9d\x37\x6c\x23\x1c\xea\xd6\xc8\xea\xc5\x5a\xd8\x52\x93\xdc\xba\x7d\x48\xca\x0e"
	"\xc5\x4c\x92\xee\xa1\x0d\x17\x61\xf1\x9e\xf1\xee\xc6\xfb\xb0\x74\x1d\xf7\xe9\x9c\x2f\x26\xde\x47\x77\x31\x87\x8e\x6a\xad\x10\x91"
	"\x5b\x07\xe7\xbb\xe7\x10\x05\x9a\x48\x1c\x52\x9f\x0b\x02\x55\xfc\xfc\xbb\xce\x9b\xe7\x79\xb6\x79\x08\x82\xd3\xe3\x6c\xc4\x70\xd1"
	"\x64\x9c\xf2\xdc\xc1\xf3\x2e\x5a\xfc\x39\x17\x35\xcc\x4f\x22\x32\xaa\x60\xd3\xeb\xcf\x37\x4f\x61\xd3\x0b\x42\xf3\x0e\x1e\xe3\xcf"
	"\x3b\x2e\x82\x1a\xed\xbd\x86\x4d\xaf\x4b\xea\x6d\x6c\x71\x85\x94\x24\x39\x1e\x10\x88\x84\x38\x2a\x82\x42\xa8\x42\xeb\x1c\x64\x88"
	"\x4a\x06\x00\x10\x10\x80\x00\x62\x09\x22\x12\xa7\x71\x98\x06\x59\xed\x12\x20\xb2\x34\x0e\x92\x24\x48\x31\x85\x8c\x31\x06\x04\x40"
	"\x00\x20\x02\x08\x81\x04\x04\x00\x00\x19\xc4\x80\xf6\x7c\xc5\xc2\x69\xa9\x5b\x1b\x68\x6d\x52\x76\x60\x76\xd5\xbe\x73\x81\x17\x54"
	"\x6e\x5e\x60\xd2\x85\xec\xd0\xee\xa8\x15\x38\x2d\x05\xea\x43\xdf\xcd\xc9\xac\xee\x49\xbc\xc5\x6c\x38\x4b\x2b\x34\x41\x7f\xe2\xd5"
	"\x51\x4d\x34\x63\x5e\x66\x17\xb0\x5b\xa2\x4f\x92\xf5\xd6\x6e\xd7\x7c\x10\x9e\x78\x2e\xd3\x38\x15\x78\x15\x0e\xbb\x51\x5a\xf0\x9d"
	"\x0e\x47\xe9\x80\x0c\x50\xc4\xd1\x28\xcd\xf5\x28\x9a\x78\x47\x01\x77\x94\xe6\x23\x52\x64\x32\x6c\x96\x95\x03\xf4\xd1\x34\x0a\x6f"
	"\x89\x9a\xc8\x73\x39\x53\x82\x8e\xa3\x87\x7a\x53\x8b\x46\x7a\x06\xde\x18\x88\xc5\x92\x55\x18\x47\x18\x93\x1b\xda\xa3\x5e\x47\x45"
	"\xf5\x35\xcc\x88\x16\x7b\x66\xb0\xfd\x58\xa0\x90\xb2\x95\x92\x0a\xfd\x6b\xb2\x63\xa1\x3b\x7c\x6e\x9d\x54\xa1\xff\xd4\x4d\x2c\x25"
	"\xa7\x57\xee\xb8\x2d\xe1\x6c\x7f\x3e\xd1\xe1\xa5\x5e\xae\x38\xf6\xd8\x8c\x26\xc7\xd7\x89\xd1\xcc\x01\x8b\x82\xd5\x97\x72\x5e\x0a"
	"\x4d\xc7\x4d\x54\xf4\xd9\x0d\xed\x43\x1c\xad\x2c\xca\xc5\x12\x9b\x28\xed\x31\xe9\x97\x4e\x37\x49\x13\x0a\x16\xc8\x5e\x07\xa2\x23"
	"\x35\xe1\x3c\x7f\x8c\xb4\x9d\xda\xa8\x87\x03\x35\x52\x80\x03\x08\x68\xe4\x41\x24\xd9\x62\xec\xca\x8c\x9c\x7b\x30\xa4\x1d\x74\x15"
	"\xd1\xbc\xf2\x8c\x96\xf5\x2a\x46\xd5\x3a\x92\x02\x41\x1b\xd8\x28\x54\xc2\x20\x7c\xba\xdb\x68\xfb\x02\x47\xe6\x10\x38\xe9\x83\x82"
	"\x64\xca\x10\x60\x30\x43\x25\xb1\xc2\xf6\x9e\x36\xad\xaf\x3f\x08\x6c\xfb\x70\x84\x20\xe7\x22\x97\x38\xa4\x30\xc1\x60\x71\x81\xb5"
	"\xc5\xb5\xdf\x5b\x8e\xfc\x72\x02\xb9\x92\x68\xdf\x58\x9e\x58\x5c\x60\xd2\x65\xf5\x0c\x43\x70\x3f\x4d\xcf\x20\xd8\x0c\xca\x56\x0b"
	"\x65\xfd\x60\xcd\x42\xec\xde\x71\xf2\x4f\x36\x08\x5e\xf3\x33\xed\xd8\x29\xe2\x95\x44\xf7\xb3\x9d\x6a\x71\xe0\x5b\xf7\x61\xa2\x7e"
	"\x17\x47\x45\xf1\x50\x19\x7b\xc9\xb6\x42\xc4\xda\x28\x04\x6b\x54\xe7\x54\x8a\x80\xa9\x87\xfa\xde\xd8\xe4\x94\x4b\x2b\xee\x1e\x04"
	"\xa4\xed\x9a\xc7\x41\x46\x2a\xf4\x89\x70\x44\xad\xf7\x2b\x64\x55\x01\x88\x7f\xa8\x57\x3a\xa9\xa1\xb5\x88\x82\xd5\xbf\x95\x10\x31"
	"\x38\x42\x53\xcf\xf2\x07\xd2\xcc\x0f\xa0\x5c\xc7\x97\xcf\x80\x91\xd8\x5e\x4e\x92\xdd\x95\x6f\xb7\x29\x7e\xe2\x4f\xb9\x0c\x80\xbb"
	"\x41\x67\x3e\xb3\x2a\x85\x2a\xa0\xaa\xaf\xb2\x39\x5f\xca\x01\x3d\xab\xa2\x18\xc2\x95\xf2\xa2\x2b\x1b\xeb\xcb\x25\x65\x36\xb9\xca"
	"\x47\x77\x1d\xd4\x05\xa1\xf7\x2f\x2f\x23\x48\xfe\x10\xa7\x1d\x89\x58\xe8\xdf\x9e\xab\x19\x26\xc1\x6c\xcc\xf9\x94\x67\xab\xed\x7d"
	"\xe9\xb5\x83\x36\x26\xa5\xbe\x50\x91\x2b\x3a\x01\xd7\xae\xdb\x55\xa3\xba\x80\x7f\xb7\xaa\x48\x64\xdb\x5f\x63\x5a\x39\x28\x37\x49"
	"\x85\x12\xe4\x54\x04\xd7\x26\x6b\x78\xd6\xed\x0b\x48\x56\x32\x79\xce\x0f\x5e\xbc\x3c\x0f\x04\xbc\x26\x63\x3f\x98\x8a\xee\xee\x71"
	"\x48\xc4\x0d\x82\x86\x89\x8d\xec\x4d\x06\xe3\xbc\x02\x0e\xb3\xa5\x26\xd0\xbc\x45\x41\x7a\xf3\x80\x22\xdc\x74\x1d\x95\x27\x91\x88"
	"\xc2\x91\x0b\xad\xc5\x22\x26\x1b\xcb\x04\xc1\xa3\x3a\xca\xd3\x2f\x4e\xd2\xbd\x92\xa9\xb7\x67\xdc\x0d\x35\x93\x55\x9c\x0a\x16\x57"
	"\xab\x1c\x30\xd2\x7c\x77\x9f\x0e\xa7\xef\x94\x47\xc5\xac\xb6\xd2\x0e\xfa\x51\x3d\x90\xd7\x40\x28\x41\x31\xf1\xb3\xc5\xc3\x29\xf4"
	"\x07\xd1\xcb\xae\x14\x7d\x50\x91\x91\x3d\x1c\x0a\x18\xff\x42\x24\x11\x67\x3e\x1e\x01\x11\x5b\x06\x47\xb0\xec\xa8\xa1\x7e\xb1\xb0"
	"\xeb\x81\xa6\xe0\x0b\x83\x87\xba\x14\xae\xea\x80\xd2\xde\xa1\xee\x5d\xb3\x3a\x00\x4e\xb5\x86\x51\xc7\xd1\x8b\x17\xe2\x10\x07\xc0"
	"\x1e\x3d\xce\x96\x58\xe4\x48\x09\x3a\xf2\x06\x8e\x78\x5f\xcf\xc3\xad\xe4\x1f\xcc\xcc\x16\x13\x17\xc1\xa0\x32\x1c\xaa\xe5\xe6\x59"
	"\x4d\x99\x8e\x7c\x7c\xe6\x07\x99\x89\x2a\xe7\x8e\xbb\x9c\x70\x34\x87\x97\x13\x11\xb0\xaf\x2f\xb7\xf0\xd4\x14\x49\xf6\xc1\xfe\x19"
	"\xd4\x53\x00\x83\xee\xba\x22\xe0\x13\x24\x52\xe5\x9b\xa5\x1c\x0a\x14\x64\x72\x40\x80\x5e\xce\x7e\x8c\xba\x05\x64\x7a\x66\xdc\x3f"
	"\xc0\x1f\x05\xcf\x53\xd9\xaa\x7f\x03\x1b\xec\xd0\xab\x58\x75\x29\x33\xae\x76\x37\x09\xff\x4c\x0b\x86\x04\x2b\x08\x32\x9b\xc2\xb4"
	"\xf2\xc4\x74\xd5\xda\xb5\x13\x21\x6f\x35\xbd\xfb\x52\xc4\xf5\x4f\x2c\xd3\xa9\x2f\x3d\x95\x73\xc5\x9a\x76\x2e\xad\x9a\xa0\xd4\x02"
	"\x13\xf6\x70\x39\xce\xab\x08\x5d\xf4\xc3\xa1\x17\xce\xa0\x84\xe7\x86\x53\x7b\xeb\x73\xbf\x0b\x31\x72\x90\x58\x4d\xe4\x96\x98\x42"
	"\x8f\x20\x16\x86\x79\xc9\x38\x00\x50\x37\x27\xb6\x48\x94\x06\x15\x0d\x0b\x48\xc2\xf2\x4c\xa6\xe0\xe1\xbe\x8e\x28\xb9\x60\x67\xab"
	"\xff\x0b\x9b\x80\xc1\xc9\x11\x6d\xcb\x90\x03\xb1\x92\x3c\x6e\x47\x4b\x64\xb5\x33\x44\xe9\x3e\xec\x53\x18\x6d\xe3\xb3\x13\x44\xed"
	"\xa5\x02\xa8\x00\x74\x34\x9d\xa2\x44\xb1\x5e\x73\xe9\xf4\xdc\x49\x7a\xcb\xc8\x1f\x41\x42\xce\x27\xcd\x03\x10\xfe\xf7\xc7\x12\x1f"
	"\x80\xef\x07\xa8\x52\xa4\xc6\xaa\xb7\xdf\xf5\x64\xc3\x3e\x43\xb4\xbf\x60\xf7\x4d\xf8\xbd\x10\xa7\x8c\x6f\x88\x8e\x33\x7f\x79\xd5"
	"\x94\x49\x80\xe1\x4a\xd6\x7c\x5b\xf3\x71\x63\x6f\xed\x5b\xae\x99\x7c\xf0\xcf\x67\x1c\x68\x72\x13\x53\xec\xfa\x2d\xc4\x7e\x3a\xd7"
	"\x66\x09\x9b\x73\x21\x1e\xd5\x54\x40\x01\xea\x05\x7f\x41\xeb\x89\x42\x34\x66\x46\xeb\x6f\x51\xa1\x0f\x09\xa4\x2b\x01\x62\x68\x05"
	"\x2d\x60\x7d\x7c\xc5\xfe\x59\x31\x03\xa5\xce\x7f\xd3\x7f\xa2\x67\x1a\xd9\x6e\x90\x0a\x32\x2b\x7a\xf4\x06\x07\xa2\xce\x1b\xf4\x17"
	"\x98\x96\x92\x98\x13\xdb\xa8\x77\x3a\x5d\x77\xb5\xd2\x7e\xd4\xb2\x37\x35\xa8\x1e\xd2\x9c\xce\x8f\x3a\x4f\xbb\x52\x1a\x6f\xb4\x05"
	"\xa5\x53\x88\xd4\x2b\x95\x9d\x0e\xa1\xb7\x9d\x6b\x99\x84\x6c\x51\x20\x15\xa0\x7e\x7c\x52\xfc\x3b\x04\x5b\x00\x54\x0a\xb2\x99\x89"
	"\xb4\x3d\x88\xda\xc2\x9f\x8a\x60\x3d"
;