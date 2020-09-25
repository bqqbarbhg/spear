#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SP_COMPRESSION_NONE = 0,
	SP_COMPRESSION_ZSTD = 1,

	SP_COMPRESSION_TYPE_FIRST = SP_COMPRESSION_NONE,
	SP_COMPRESSION_TYPE_LAST = SP_COMPRESSION_ZSTD,
	SP_COMPRESSION_FORCE_U32 = 0x7fffffff,
} sp_compression_type;

typedef enum sp_format {

	SP_FORMAT_UNKNOWN,

	// Basic 8 bits per component
	SP_FORMAT_R8_UNORM, SP_FORMAT_R8_SNORM, SP_FORMAT_R8_UINT, SP_FORMAT_R8_SINT,
	SP_FORMAT_RG8_UNORM, SP_FORMAT_RG8_SNORM, SP_FORMAT_RG8_UINT, SP_FORMAT_RG8_SINT,
	SP_FORMAT_RGB8_UNORM, SP_FORMAT_RGB8_SNORM, SP_FORMAT_RGB8_UINT, SP_FORMAT_RGB8_SINT, SP_FORMAT_RGB8_SRGB,
	SP_FORMAT_RGBA8_UNORM, SP_FORMAT_RGBA8_SNORM, SP_FORMAT_RGBA8_UINT, SP_FORMAT_RGBA8_SINT, SP_FORMAT_RGBA8_SRGB,

	// Basic 16 bits per component
	SP_FORMAT_R16_UNORM, SP_FORMAT_R16_SNORM, SP_FORMAT_R16_UINT, SP_FORMAT_R16_SINT, SP_FORMAT_R16_FLOAT,
	SP_FORMAT_RG16_UNORM, SP_FORMAT_RG16_SNORM, SP_FORMAT_RG16_UINT, SP_FORMAT_RG16_SINT, SP_FORMAT_RG16_FLOAT,
	SP_FORMAT_RGB16_UNORM, SP_FORMAT_RGB16_SNORM, SP_FORMAT_RGB16_UINT, SP_FORMAT_RGB16_SINT, SP_FORMAT_RGB16_FLOAT,
	SP_FORMAT_RGBA16_UNORM, SP_FORMAT_RGBA16_SNORM, SP_FORMAT_RGBA16_UINT, SP_FORMAT_RGBA16_SINT, SP_FORMAT_RGBA16_FLOAT,

	// Basic 32 bits per component
	SP_FORMAT_R32_UNORM, SP_FORMAT_R32_SNORM, SP_FORMAT_R32_UINT, SP_FORMAT_R32_SINT, SP_FORMAT_R32_FLOAT,
	SP_FORMAT_RG32_UNORM, SP_FORMAT_RG32_SNORM, SP_FORMAT_RG32_UINT, SP_FORMAT_RG32_SINT, SP_FORMAT_RG32_FLOAT,
	SP_FORMAT_RGB32_UNORM, SP_FORMAT_RGB32_SNORM, SP_FORMAT_RGB32_UINT, SP_FORMAT_RGB32_SINT, SP_FORMAT_RGB32_FLOAT,
	SP_FORMAT_RGBA32_UNORM, SP_FORMAT_RGBA32_SNORM, SP_FORMAT_RGBA32_UINT, SP_FORMAT_RGBA32_SINT, SP_FORMAT_RGBA32_FLOAT,

	// Depth buffer
	SP_FORMAT_D16_UNORM,
	SP_FORMAT_D24S8_UNORM,
	SP_FORMAT_D32_FLOAT,
	SP_FORMAT_D32S8_FLOAT,

	// Bit-packing
	SP_FORMAT_RGB10A2_UNORM, SP_FORMAT_RGB10A2_UINT,
	SP_FORMAT_R11G11B10_FLOAT,

	// BCn compression
	SP_FORMAT_BC1_UNORM, SP_FORMAT_BC1_SRGB,
	SP_FORMAT_BC2_UNORM, SP_FORMAT_BC2_SRGB,
	SP_FORMAT_BC3_UNORM, SP_FORMAT_BC3_SRGB,
	SP_FORMAT_BC4_UNORM, SP_FORMAT_BC4_SNORM,
	SP_FORMAT_BC5_UNORM, SP_FORMAT_BC5_SNORM,
	SP_FORMAT_BC6_UFLOAT, SP_FORMAT_BC6_SFLOAT,
	SP_FORMAT_BC7_UNORM, SP_FORMAT_BC7_SRGB,

	// ASTC compression (2D)
	SP_FORMAT_ASTC4X4_UNORM, SP_FORMAT_ASTC4X4_SRGB,
	SP_FORMAT_ASTC5X4_UNORM, SP_FORMAT_ASTC5X4_SRGB,
	SP_FORMAT_ASTC5X5_UNORM, SP_FORMAT_ASTC5X5_SRGB,
	SP_FORMAT_ASTC6X5_UNORM, SP_FORMAT_ASTC6X5_SRGB,
	SP_FORMAT_ASTC6X6_UNORM, SP_FORMAT_ASTC6X6_SRGB,
	SP_FORMAT_ASTC8X5_UNORM, SP_FORMAT_ASTC8X5_SRGB,
	SP_FORMAT_ASTC8X6_UNORM, SP_FORMAT_ASTC8X6_SRGB,
	SP_FORMAT_ASTC10X5_UNORM, SP_FORMAT_ASTC10X5_SRGB,
	SP_FORMAT_ASTC10X6_UNORM, SP_FORMAT_ASTC10X6_SRGB,
	SP_FORMAT_ASTC8X8_UNORM, SP_FORMAT_ASTC8X8_SRGB,
	SP_FORMAT_ASTC10X8_UNORM, SP_FORMAT_ASTC10X8_SRGB,
	SP_FORMAT_ASTC10X10_UNORM, SP_FORMAT_ASTC10X10_SRGB,
	SP_FORMAT_ASTC12X10_UNORM, SP_FORMAT_ASTC12X10_SRGB,
	SP_FORMAT_ASTC12X12_UNORM, SP_FORMAT_ASTC12X12_SRGB,

	// Special footer
	SP_FORMAT_COUNT,
	SP_FORMAT_FORCE_U32 = 0x7fffffff,

} sp_format;

typedef enum sp_vertex_attrib {
	SP_VERTEX_ATTRIB_POSITION,
	SP_VERTEX_ATTRIB_NORMAL,
	SP_VERTEX_ATTRIB_TANGENT,
	SP_VERTEX_ATTRIB_TANGENT_SIGN,
	SP_VERTEX_ATTRIB_UV,
	SP_VERTEX_ATTRIB_COLOR,
	SP_VERTEX_ATTRIB_BONE_INDEX,
	SP_VERTEX_ATTRIB_BONE_WEIGHT,
	SP_VERTEX_ATTRIB_PADDING,

	SP_VERTEX_ATTRIB_COUNT,
	SP_VERTEX_ATTRIB_FORCE_U32 = 0x7fffffff,
} sp_vertex_attrib;

typedef enum sp_format_flags {
	SP_FORMAT_FLAG_INTEGER = 0x1,
	SP_FORMAT_FLAG_FLOAT = 0x2,
	SP_FORMAT_FLAG_NORMALIZED = 0x4,
	SP_FORMAT_FLAG_SIGNED = 0x8,
	SP_FORMAT_FLAG_SRGB = 0x10,
	SP_FORMAT_FLAG_COMPRESSED = 0x20,
	SP_FORMAT_FLAG_DEPTH = 0x40,
	SP_FORMAT_FLAG_STENCIL = 0x80,
	SP_FORMAT_FLAG_BASIC = 0x100,
} sp_format_flags;

typedef struct sp_format_info {
	sp_format format;
	const char *enum_name;
	const char *short_name;
	uint32_t num_components;
	uint32_t block_size;
	uint32_t block_x, block_y;
	uint32_t flags;
} sp_format_info;

sp_format sp_find_format(uint32_t num_components, uint32_t component_size, sp_format_flags flags);

extern const sp_format_info sp_format_infos[SP_FORMAT_COUNT];

size_t sp_get_compression_bound(sp_compression_type type, size_t src_size);
size_t sp_compress_buffer(sp_compression_type type, void *dst, size_t dst_size, const void *src, size_t src_size, int level);
size_t sp_decompress_buffer(sp_compression_type type, void *dst, size_t dst_size, const void *src, size_t src_size);

typedef enum spfile_header_magic {
	SPFILE_HEADER_SPTEX   = 0x78747073, // 'sptx'
	SPFILE_HEADER_SPMDL   = 0x646d7073, // 'spmd'
	SPFILE_HEADER_SPANIM  = 0x6e617073, // 'span'
	SPFILE_HEADER_SPSOUND = 0x646e7373, // 'ssnd'

	SPFILE_HEADER_FORCE_U32 = 0x7fffffff,
} spfile_header_magic;

typedef enum spfile_section_magic {
	SPFILE_SECTION_STRINGS   = 0x73727473, // 'strs'
	SPFILE_SECTION_BONES     = 0x656e6f62, // 'bone'
	SPFILE_SECTION_NODES     = 0x65646f6e, // 'node'
	SPFILE_SECTION_MATERIALS = 0x7374616d, // 'mats'
	SPFILE_SECTION_MESHES    = 0x6873656d, // 'mesh'
	SPFILE_SECTION_GEOMETRY  = 0x6d6f6567, // 'geom'
	SPFILE_SECTION_BVH_NODES = 0x6e687662, // 'bvhn'
	SPFILE_SECTION_BVH_TRIS  = 0x74687662, // 'bvht'
	SPFILE_SECTION_VERTEX    = 0x78747276, // 'vrtx'
	SPFILE_SECTION_INDEX     = 0x78646e69, // 'indx'
	SPFILE_SECTION_MIP       = 0x2070696d, // 'mip '
	SPFILE_SECTION_ANIMATION = 0x6d696e61, // 'anim'
	SPFILE_SECTION_AUDIO     = 0x6f696461, // 'adio'
	SPFILE_SECTION_TAKES     = 0x656b6174, // 'take'

	SPFILE_SECTION_FORCE_U32 = 0x7fffffff,
} spfile_section_magic;

typedef struct spfile_section
{
	spfile_section_magic magic;
	sp_compression_type compression_type;
	uint32_t index;
	uint32_t offset;
	uint32_t uncompressed_size;
	uint32_t compressed_size;
} spfile_section;

typedef struct spfile_header
{
	spfile_header_magic magic;
	uint32_t version;
	uint32_t header_info_size;
	uint32_t num_sections;
} spfile_header;

typedef struct spfile_string
{
	uint32_t offset;
	uint32_t length;
} spfile_string;

typedef struct spanim_bone
{
	uint32_t parent;
	spfile_string name;
} spanim_bone;

typedef struct spanim_info {
	double duration;
	uint32_t num_bones;
} spanim_info;

typedef struct spanim_header {
	spfile_header header;
	spanim_info info;
	spfile_section s_bones;     // spanim_bone[info.num_bones]
	spfile_section s_strings;   // char[uncompressed_size]
	spfile_section s_animation; // char[uncompressed_size]
} spanim_header;

#define SPMDL_MAX_VERTEX_BUFFERS 4
#define SPMDL_MAX_VERTEX_ATTRIBS 16
#define SPMDL_BVH_TRIANGLES 16

typedef struct spmdl_vec3
{
	float x, y, z;
} spmdl_vec3;

typedef struct spmdl_vec4
{
	float x, y, z, w;
} spmdl_vec4;

typedef struct spmdl_matrix
{
	spmdl_vec3 columns[4];
} spmdl_matrix;

typedef struct spmdl_node
{
	uint32_t parent;
	spfile_string name;
	spmdl_vec3 translation;
	spmdl_vec4 rotation;
	spmdl_vec3 scale;
	spmdl_matrix self_to_parent;
	spmdl_matrix self_to_root;
} spmdl_node;

typedef struct spmdl_bone
{
	uint32_t node;
	spmdl_matrix mesh_to_bone;
} spmdl_bone;

typedef struct spmdl_buffer
{
	uint32_t offset;
	uint32_t encoded_size;
	uint32_t stride;
} spmdl_buffer;

typedef struct spmdl_attrib
{
	sp_vertex_attrib attrib;
	sp_format format;
	uint32_t stream;
	uint32_t offset;
} spmdl_attrib;

typedef struct spmdl_material
{
	spfile_string name;
} spmdl_material;

typedef struct spmdl_mesh
{
	uint32_t node;
	uint32_t material;
	uint32_t num_indices;
	uint32_t num_vertices;
	uint32_t num_vertex_buffers;
	uint32_t num_attribs;
	uint32_t bone_offset;
	uint32_t num_bones;
	uint32_t bvh_index;
	spmdl_vec3 aabb_min;
	spmdl_vec3 aabb_max;
	spmdl_buffer vertex_buffers[SPMDL_MAX_VERTEX_BUFFERS];
	spmdl_buffer index_buffer;
	spmdl_attrib attribs[SPMDL_MAX_VERTEX_ATTRIBS];
} spmdl_mesh;

typedef struct spmdl_bvh_split
{
	spmdl_vec3 aabb_min, aabb_max;

	// If non-negative the split is a leaf node containing `num_triangles`
	// triangles startin from `data_index`.
	int32_t num_triangles;

	// Offset to child if `num_triangles>=0`, otherwise offset to triangles array
	uint32_t data_index;
} spmdl_bvh_split;

typedef struct spmdl_bvh_node
{
	spmdl_bvh_split splits[2];
} spmdl_bvh_node;

typedef struct spmdl_info {
	uint32_t num_nodes;
	uint32_t num_bones;
	uint32_t num_materials;
	uint32_t num_meshes;
	uint32_t num_bvh_nodes;
	uint32_t num_bvh_tris;
} spmdl_info;

typedef struct spmdl_header {
	spfile_header header;
	spmdl_info info;
	spfile_section s_nodes;     // spmdl_node[info.num_nodes]
	spfile_section s_bones;     // spmdl_bone[info.num_bones]
	spfile_section s_materials; // spmdl_material[info.num_materials]
	spfile_section s_meshes;    // spmdl_mesh[info.num_meshes]
	spfile_section s_bvh_nodes; // spmdl_bvh_node[info.num_bvh_nodes]
	spfile_section s_bvh_tris;  // uint32_t[info.num_bvh_tris * 3]
	spfile_section s_strings;   // char[uncompressed_size]
	spfile_section s_vertex;    // char[uncompressed_size]
	spfile_section s_index;     // char[uncompressed_size]
} spmdl_header;

typedef struct sptex_mip {
	uint32_t width, height;
	uint32_t compressed_data_offset;
	uint32_t compressed_data_size;
	uint32_t uncompressed_data_size;
	sp_compression_type compression_type;
} sptex_mip;

typedef struct sptex_info {
	sp_format format;
	uint16_t width, height;
	uint16_t uncropped_width, uncropped_height;
	uint16_t crop_min_x, crop_min_y;
	uint16_t crop_max_x, crop_max_y;
	uint32_t num_mips;
	uint32_t num_slices;
} sptex_info;

typedef struct sptex_header {
	spfile_header header;
	sptex_info info;
	spfile_section s_mips[16];
} sptex_header;

typedef enum {
	SPSOUND_FORMAT_NONE = 0,
	SPSOUND_FORMAT_PCM16 = 1,
	SPSOUND_FORMAT_VORBIS = 2,

	SPSOUND_FORMAT_FORCE_U32 = 0x7fffffff,
} spsound_format;

typedef struct spsound_take {
	spsound_format format;
	float length_in_seconds;
	uint32_t length_in_samples;
	uint32_t sample_rate;
	uint32_t num_channels;
	uint32_t temp_memory_required;
	uint32_t file_offset;
	uint32_t file_size;
} spsound_take;

typedef struct spsound_info {
	uint32_t num_takes;
	uint32_t temp;
} spsound_info;

typedef struct spsound_header {
	spfile_header header;
	spsound_info info;
	spfile_section s_takes;
	spfile_section s_audio;
} spsound_header;

typedef struct spfile_util {
	const void *data;
	size_t size;
	char *strings;
	size_t strings_size;
	void *page_to_free;
	bool failed;
} spfile_util;

bool spfile_util_init(spfile_util *su, const void *data, size_t size);
bool spfile_decode_section_to(spfile_util *su, const spfile_section *s, void *buffer);
void *spfile_decode_section(spfile_util *su, const spfile_section *s);
bool spfile_decode_strings_to(spfile_util *su, const spfile_section *s, char *buffer);
char *spfile_decode_strings(spfile_util *su, const spfile_section *s);
bool spfile_util_failed(spfile_util *su);
void spfile_util_free(spfile_util *su);

typedef struct spanim_util {
	spfile_util file;
} spanim_util;

bool spanim_util_init(spanim_util *su, const void *data, size_t size);

bool spanim_decode_strings_to(spanim_util *su, char *buffer);
bool spanim_decode_bones_to(spanim_util *su, spanim_bone *buffer);
bool spanim_decode_animation_to(spanim_util *su, char *buffer);

spanim_header spanim_decode_header(spanim_util *su);
char *spanim_decode_strings(spanim_util *su);
spanim_bone *spanim_decode_bones(spanim_util *su);
char *spanim_decode_animation(spanim_util *su);

typedef struct spmdl_util {
	spfile_util file;
} spmdl_util;

bool spmdl_util_init(spmdl_util *su, const void *data, size_t size);

bool spmdl_decode_strings_to(spmdl_util *su, char *buffer);
bool spmdl_decode_nodes_to(spmdl_util *su, spmdl_node *buffer);
bool spmdl_decode_bones_to(spmdl_util *su, spmdl_bone *buffer);
bool spmdl_decode_materials_to(spmdl_util *su, spmdl_material *buffer);
bool spmdl_decode_meshes_to(spmdl_util *su, spmdl_mesh *buffer);
bool spmdl_decode_bvh_nodes_to(spmdl_util *su, spmdl_bvh_node *buffer);
bool spmdl_decode_bvh_tris_to(spmdl_util *su, uint32_t *buffer);
bool spmdl_decode_vertex_to(spmdl_util *su, char *buffer);
bool spmdl_decode_index_to(spmdl_util *su, char *buffer);

spmdl_header spmdl_decode_header(spmdl_util *su);
char *spmdl_decode_strings(spmdl_util *su);
spmdl_node *spmdl_decode_nodes(spmdl_util *su);
spmdl_bone *spmdl_decode_bones(spmdl_util *su);
spmdl_material *spmdl_decode_materials(spmdl_util *su);
spmdl_mesh *spmdl_decode_meshes(spmdl_util *su);
spmdl_bvh_node *spmdl_decode_bvh_nodes(spmdl_util *su);
uint32_t *spmdl_decode_bvh_tris(spmdl_util *su);
char *spmdl_decode_vertex(spmdl_util *su);
char *spmdl_decode_index(spmdl_util *su);

typedef struct sptex_util {
	spfile_util file;
} sptex_util;

bool sptex_util_init(sptex_util *su, const void *data, size_t size);

bool sptex_decode_mip_to(sptex_util *su, uint32_t index, char *buffer);

sptex_header sptex_decode_header(sptex_util *su);
char *sptex_decode_mip(sptex_util *su, uint32_t index);

typedef struct spsound_util {
	spfile_util file;
} spsound_util;

bool spsound_util_init(spsound_util *su, const void *data, size_t size);

bool spsound_decode_takes_to(spsound_util *su, spsound_take *takes);
bool spsound_decode_audio_to(spsound_util *su, void *buffer);

spsound_header spsound_decode_header(spsound_util *su);
spsound_take *spsound_decode_takes(spsound_util *su);
void *spsound_decode_audio(spsound_util *su);


#ifdef __cplusplus
}
#endif
