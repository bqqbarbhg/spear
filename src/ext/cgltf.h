/**
 * cgltf - a single-file glTF 2.0 parser written in C99.
 *
 * Version: 1.5
 *
 * Website: https://github.com/jkuhlmann/cgltf
 *
 * Distributed under the MIT License, see notice at the end of this file.
 *
 * Building:
 * Include this file where you need the struct and function
 * declarations. Have exactly one source file where you define
 * `CGLTF_IMPLEMENTATION` before including this file to get the
 * function definitions.
 *
 * Reference:
 * `cgltf_result cgltf_parse(const cgltf_options*, const void*,
 * cgltf_size, cgltf_data**)` parses both glTF and GLB data. If
 * this function returns `cgltf_result_success`, you have to call
 * `cgltf_free()` on the created `cgltf_data*` variable.
 * Note that contents of external files for buffers and images are not
 * automatically loaded. You'll need to read these files yourself using
 * URIs in the `cgltf_data` structure.
 *
 * `cgltf_options` is the struct passed to `cgltf_parse()` to control
 * parts of the parsing process. You can use it to force the file type
 * and provide memory allocation as well as file operation callbacks. 
 * Should be zero-initialized to trigger default behavior.
 *
 * `cgltf_data` is the struct allocated and filled by `cgltf_parse()`.
 * It generally mirrors the glTF format as described by the spec (see
 * https://github.com/KhronosGroup/glTF/tree/master/specification/2.0).
 *
 * `void cgltf_free(cgltf_data*)` frees the allocated `cgltf_data`
 * variable.
 *
 * `cgltf_result cgltf_load_buffers(const cgltf_options*, cgltf_data*,
 * const char* gltf_path)` can be optionally called to open and read buffer
 * files using the `FILE*` APIs. The `gltf_path` argument is the path to
 * the original glTF file, which allows the parser to resolve the path to
 * buffer files.
 *
 * `cgltf_result cgltf_load_buffer_base64(const cgltf_options* options,
 * cgltf_size size, const char* base64, void** out_data)` decodes
 * base64-encoded data content. Used internally by `cgltf_load_buffers()`
 * and may be useful if you're not dealing with normal files.
 *
 * `cgltf_result cgltf_parse_file(const cgltf_options* options, const
 * char* path, cgltf_data** out_data)` can be used to open the given
 * file using `FILE*` APIs and parse the data using `cgltf_parse()`.
 *
 * `cgltf_result cgltf_validate(cgltf_data*)` can be used to do additional
 * checks to make sure the parsed glTF data is valid.
 *
 * `cgltf_node_transform_local` converts the translation / rotation / scale properties of a node
 * into a mat4.
 *
 * `cgltf_node_transform_world` calls `cgltf_node_transform_local` on every ancestor in order
 * to compute the root-to-node transformation.
 *
 * `cgltf_accessor_unpack_floats` reads in the data from an accessor, applies sparse data (if any),
 * and converts them to floating point. Assumes that `cgltf_load_buffers` has already been called.
 * By passing null for the output pointer, users can find out how many floats are required in the
 * output buffer.
 *
 * `cgltf_accessor_num_components` is a tiny utility that tells you the dimensionality of
 * a certain accessor type. This can be used before `cgltf_accessor_unpack_floats` to help allocate
 * the necessary amount of memory.
 *
 * `cgltf_accessor_read_float` reads a certain element from a non-sparse accessor and converts it to
 * floating point, assuming that `cgltf_load_buffers` has already been called. The passed-in element
 * size is the number of floats in the output buffer, which should be in the range [1, 16]. Returns
 * false if the passed-in element_size is too small, or if the accessor is sparse.
 *
 * `cgltf_accessor_read_uint` is similar to its floating-point counterpart, but limited to reading
 * vector types and does not support matrix types. The passed-in element size is the number of uints
 * in the output buffer, which should be in the range [1, 4]. Returns false if the passed-in 
 * element_size is too small, or if the accessor is sparse.
 *
 * `cgltf_accessor_read_index` is similar to its floating-point counterpart, but it returns size_t
 * and only works with single-component data types.
 *
 * `cgltf_result cgltf_copy_extras_json(const cgltf_data*, const cgltf_extras*,
 * char* dest, cgltf_size* dest_size)` allows users to retrieve the "extras" data that
 * can be attached to many glTF objects (which can be arbitrary JSON data). The
 * `cgltf_extras` struct stores the offsets of the start and end of the extras JSON data
 * as it appears in the complete glTF JSON data. This function copies the extras data
 * into the provided buffer. If `dest` is NULL, the length of the data is written into
 * `dest_size`. You can then parse this data using your own JSON parser
 * or, if you've included the cgltf implementation using the integrated JSMN JSON parser.
 */
#ifndef CGLTF_H_INCLUDED__
#define CGLTF_H_INCLUDED__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t cgltf_size;
typedef float cgltf_float;
typedef int cgltf_int;
typedef unsigned int cgltf_uint;
typedef int cgltf_bool;

typedef enum cgltf_file_type
{
	cgltf_file_type_invalid,
	cgltf_file_type_gltf,
	cgltf_file_type_glb,
} cgltf_file_type;

typedef enum cgltf_result
{
	cgltf_result_success,
	cgltf_result_data_too_short,
	cgltf_result_unknown_format,
	cgltf_result_invalid_json,
	cgltf_result_invalid_gltf,
	cgltf_result_invalid_options,
	cgltf_result_file_not_found,
	cgltf_result_io_error,
	cgltf_result_out_of_memory,
	cgltf_result_legacy_gltf,
} cgltf_result;

typedef struct cgltf_memory_options
{
	void* (*alloc)(void* user, cgltf_size size);
	void (*free) (void* user, void* ptr);
	void* user_data;
} cgltf_memory_options;

typedef struct cgltf_file_options
{
	cgltf_result(*read)(const struct cgltf_memory_options* memory_options, const struct cgltf_file_options* file_options, const char* path, cgltf_size* size, void** data);
	void (*release)(const struct cgltf_memory_options* memory_options, const struct cgltf_file_options* file_options, void* data);
	void* user_data;
} cgltf_file_options;

typedef struct cgltf_options
{
	cgltf_file_type type; /* invalid == auto detect */
	cgltf_size json_token_count; /* 0 == auto */
	cgltf_memory_options memory;
	cgltf_file_options file;
} cgltf_options;

typedef enum cgltf_buffer_view_type
{
	cgltf_buffer_view_type_invalid,
	cgltf_buffer_view_type_indices,
	cgltf_buffer_view_type_vertices,
} cgltf_buffer_view_type;

typedef enum cgltf_attribute_type
{
	cgltf_attribute_type_invalid,
	cgltf_attribute_type_position,
	cgltf_attribute_type_normal,
	cgltf_attribute_type_tangent,
	cgltf_attribute_type_texcoord,
	cgltf_attribute_type_color,
	cgltf_attribute_type_joints,
	cgltf_attribute_type_weights,
} cgltf_attribute_type;

typedef enum cgltf_component_type
{
	cgltf_component_type_invalid,
	cgltf_component_type_r_8, /* BYTE */
	cgltf_component_type_r_8u, /* UNSIGNED_BYTE */
	cgltf_component_type_r_16, /* SHORT */
	cgltf_component_type_r_16u, /* UNSIGNED_SHORT */
	cgltf_component_type_r_32u, /* UNSIGNED_INT */
	cgltf_component_type_r_32f, /* FLOAT */
} cgltf_component_type;

typedef enum cgltf_type
{
	cgltf_type_invalid,
	cgltf_type_scalar,
	cgltf_type_vec2,
	cgltf_type_vec3,
	cgltf_type_vec4,
	cgltf_type_mat2,
	cgltf_type_mat3,
	cgltf_type_mat4,
} cgltf_type;

typedef enum cgltf_primitive_type
{
	cgltf_primitive_type_points,
	cgltf_primitive_type_lines,
	cgltf_primitive_type_line_loop,
	cgltf_primitive_type_line_strip,
	cgltf_primitive_type_triangles,
	cgltf_primitive_type_triangle_strip,
	cgltf_primitive_type_triangle_fan,
} cgltf_primitive_type;

typedef enum cgltf_alpha_mode
{
	cgltf_alpha_mode_opaque,
	cgltf_alpha_mode_mask,
	cgltf_alpha_mode_blend,
} cgltf_alpha_mode;

typedef enum cgltf_animation_path_type {
	cgltf_animation_path_type_invalid,
	cgltf_animation_path_type_translation,
	cgltf_animation_path_type_rotation,
	cgltf_animation_path_type_scale,
	cgltf_animation_path_type_weights,
} cgltf_animation_path_type;

typedef enum cgltf_interpolation_type {
	cgltf_interpolation_type_linear,
	cgltf_interpolation_type_step,
	cgltf_interpolation_type_cubic_spline,
} cgltf_interpolation_type;

typedef enum cgltf_camera_type {
	cgltf_camera_type_invalid,
	cgltf_camera_type_perspective,
	cgltf_camera_type_orthographic,
} cgltf_camera_type;

typedef enum cgltf_light_type {
	cgltf_light_type_invalid,
	cgltf_light_type_directional,
	cgltf_light_type_point,
	cgltf_light_type_spot,
} cgltf_light_type;

typedef struct cgltf_extras {
	cgltf_size start_offset;
	cgltf_size end_offset;
} cgltf_extras;

typedef struct cgltf_buffer
{
	cgltf_size size;
	char* uri;
	void* data; /* loaded by cgltf_load_buffers */
	cgltf_extras extras;
} cgltf_buffer;

typedef struct cgltf_buffer_view
{
	cgltf_buffer* buffer;
	cgltf_size offset;
	cgltf_size size;
	cgltf_size stride; /* 0 == automatically determined by accessor */
	cgltf_buffer_view_type type;
	cgltf_extras extras;
} cgltf_buffer_view;

typedef struct cgltf_accessor_sparse
{
	cgltf_size count;
	cgltf_buffer_view* indices_buffer_view;
	cgltf_size indices_byte_offset;
	cgltf_component_type indices_component_type;
	cgltf_buffer_view* values_buffer_view;
	cgltf_size values_byte_offset;
	cgltf_extras extras;
	cgltf_extras indices_extras;
	cgltf_extras values_extras;
} cgltf_accessor_sparse;

typedef struct cgltf_accessor
{
	cgltf_component_type component_type;
	cgltf_bool normalized;
	cgltf_type type;
	cgltf_size offset;
	cgltf_size count;
	cgltf_size stride;
	cgltf_buffer_view* buffer_view;
	cgltf_bool has_min;
	cgltf_float min[16];
	cgltf_bool has_max;
	cgltf_float max[16];
	cgltf_bool is_sparse;
	cgltf_accessor_sparse sparse;
	cgltf_extras extras;
} cgltf_accessor;

typedef struct cgltf_attribute
{
	char* name;
	cgltf_attribute_type type;
	cgltf_int index;
	cgltf_accessor* data;
} cgltf_attribute;

typedef struct cgltf_image
{
	char* name;
	char* uri;
	cgltf_buffer_view* buffer_view;
	char* mime_type;
	cgltf_extras extras;
} cgltf_image;

typedef struct cgltf_sampler
{
	cgltf_int mag_filter;
	cgltf_int min_filter;
	cgltf_int wrap_s;
	cgltf_int wrap_t;
	cgltf_extras extras;
} cgltf_sampler;

typedef struct cgltf_texture
{
	char* name;
	cgltf_image* image;
	cgltf_sampler* sampler;
	cgltf_extras extras;
} cgltf_texture;

typedef struct cgltf_texture_transform
{
	cgltf_float offset[2];
	cgltf_float rotation;
	cgltf_float scale[2];
	cgltf_int texcoord;
} cgltf_texture_transform;

typedef struct cgltf_texture_view
{
	cgltf_texture* texture;
	cgltf_int texcoord;
	cgltf_float scale; /* equivalent to strength for occlusion_texture */
	cgltf_bool has_transform;
	cgltf_texture_transform transform;
	cgltf_extras extras;
} cgltf_texture_view;

typedef struct cgltf_pbr_metallic_roughness
{
	cgltf_texture_view base_color_texture;
	cgltf_texture_view metallic_roughness_texture;

	cgltf_float base_color_factor[4];
	cgltf_float metallic_factor;
	cgltf_float roughness_factor;

	cgltf_extras extras;
} cgltf_pbr_metallic_roughness;

typedef struct cgltf_pbr_specular_glossiness
{
	cgltf_texture_view diffuse_texture;
	cgltf_texture_view specular_glossiness_texture;

	cgltf_float diffuse_factor[4];
	cgltf_float specular_factor[3];
	cgltf_float glossiness_factor;
} cgltf_pbr_specular_glossiness;

typedef struct cgltf_clearcoat
{
	cgltf_texture_view clearcoat_texture;
	cgltf_texture_view clearcoat_roughness_texture;
	cgltf_texture_view clearcoat_normal_texture;

	cgltf_float clearcoat_factor;
	cgltf_float clearcoat_roughness_factor;
} cgltf_clearcoat;

typedef struct cgltf_material
{
	char* name;
	cgltf_bool has_pbr_metallic_roughness;
	cgltf_bool has_pbr_specular_glossiness;
	cgltf_bool has_clearcoat;
	cgltf_pbr_metallic_roughness pbr_metallic_roughness;
	cgltf_pbr_specular_glossiness pbr_specular_glossiness;
	cgltf_clearcoat clearcoat;
	cgltf_texture_view normal_texture;
	cgltf_texture_view occlusion_texture;
	cgltf_texture_view emissive_texture;
	cgltf_float emissive_factor[3];
	cgltf_alpha_mode alpha_mode;
	cgltf_float alpha_cutoff;
	cgltf_bool double_sided;
	cgltf_bool unlit;
	cgltf_extras extras;
} cgltf_material;

typedef struct cgltf_morph_target {
	cgltf_attribute* attributes;
	cgltf_size attributes_count;
} cgltf_morph_target;

typedef struct cgltf_draco_mesh_compression {
	cgltf_buffer_view* buffer_view;
	cgltf_attribute* attributes;
	cgltf_size attributes_count;
} cgltf_draco_mesh_compression;

typedef struct cgltf_primitive {
	cgltf_primitive_type type;
	cgltf_accessor* indices;
	cgltf_material* material;
	cgltf_attribute* attributes;
	cgltf_size attributes_count;
	cgltf_morph_target* targets;
	cgltf_size targets_count;
	cgltf_extras extras;
	cgltf_bool has_draco_mesh_compression;
	cgltf_draco_mesh_compression draco_mesh_compression;
} cgltf_primitive;

typedef struct cgltf_mesh {
	char* name;
	cgltf_primitive* primitives;
	cgltf_size primitives_count;
	cgltf_float* weights;
	cgltf_size weights_count;
	char** target_names;
	cgltf_size target_names_count;
	cgltf_extras extras;
} cgltf_mesh;

typedef struct cgltf_node cgltf_node;

typedef struct cgltf_skin {
	char* name;
	cgltf_node** joints;
	cgltf_size joints_count;
	cgltf_node* skeleton;
	cgltf_accessor* inverse_bind_matrices;
	cgltf_extras extras;
} cgltf_skin;

typedef struct cgltf_camera_perspective {
	cgltf_float aspect_ratio;
	cgltf_float yfov;
	cgltf_float zfar;
	cgltf_float znear;
	cgltf_extras extras;
} cgltf_camera_perspective;

typedef struct cgltf_camera_orthographic {
	cgltf_float xmag;
	cgltf_float ymag;
	cgltf_float zfar;
	cgltf_float znear;
	cgltf_extras extras;
} cgltf_camera_orthographic;

typedef struct cgltf_camera {
	char* name;
	cgltf_camera_type type;
	union {
		cgltf_camera_perspective perspective;
		cgltf_camera_orthographic orthographic;
	} data;
	cgltf_extras extras;
} cgltf_camera;

typedef struct cgltf_light {
	char* name;
	cgltf_float color[3];
	cgltf_float intensity;
	cgltf_light_type type;
	cgltf_float range;
	cgltf_float spot_inner_cone_angle;
	cgltf_float spot_outer_cone_angle;
} cgltf_light;

struct cgltf_node {
	char* name;
	cgltf_node* parent;
	cgltf_node** children;
	cgltf_size children_count;
	cgltf_skin* skin;
	cgltf_mesh* mesh;
	cgltf_camera* camera;
	cgltf_light* light;
	cgltf_float* weights;
	cgltf_size weights_count;
	cgltf_bool has_translation;
	cgltf_bool has_rotation;
	cgltf_bool has_scale;
	cgltf_bool has_matrix;
	cgltf_float translation[3];
	cgltf_float rotation[4];
	cgltf_float scale[3];
	cgltf_float matrix[16];
	cgltf_extras extras;
};

typedef struct cgltf_scene {
	char* name;
	cgltf_node** nodes;
	cgltf_size nodes_count;
	cgltf_extras extras;
} cgltf_scene;

typedef struct cgltf_animation_sampler {
	cgltf_accessor* input;
	cgltf_accessor* output;
	cgltf_interpolation_type interpolation;
	cgltf_extras extras;
} cgltf_animation_sampler;

typedef struct cgltf_animation_channel {
	cgltf_animation_sampler* sampler;
	cgltf_node* target_node;
	cgltf_animation_path_type target_path;
	cgltf_extras extras;
} cgltf_animation_channel;

typedef struct cgltf_animation {
	char* name;
	cgltf_animation_sampler* samplers;
	cgltf_size samplers_count;
	cgltf_animation_channel* channels;
	cgltf_size channels_count;
	cgltf_extras extras;
} cgltf_animation;

typedef struct cgltf_asset {
	char* copyright;
	char* generator;
	char* version;
	char* min_version;
	cgltf_extras extras;
} cgltf_asset;

typedef struct cgltf_data
{
	cgltf_file_type file_type;
	void* file_data;

	cgltf_asset asset;

	cgltf_mesh* meshes;
	cgltf_size meshes_count;

	cgltf_material* materials;
	cgltf_size materials_count;

	cgltf_accessor* accessors;
	cgltf_size accessors_count;

	cgltf_buffer_view* buffer_views;
	cgltf_size buffer_views_count;

	cgltf_buffer* buffers;
	cgltf_size buffers_count;

	cgltf_image* images;
	cgltf_size images_count;

	cgltf_texture* textures;
	cgltf_size textures_count;

	cgltf_sampler* samplers;
	cgltf_size samplers_count;

	cgltf_skin* skins;
	cgltf_size skins_count;

	cgltf_camera* cameras;
	cgltf_size cameras_count;

	cgltf_light* lights;
	cgltf_size lights_count;

	cgltf_node* nodes;
	cgltf_size nodes_count;

	cgltf_scene* scenes;
	cgltf_size scenes_count;

	cgltf_scene* scene;

	cgltf_animation* animations;
	cgltf_size animations_count;

	cgltf_extras extras;

	char** extensions_used;
	cgltf_size extensions_used_count;

	char** extensions_required;
	cgltf_size extensions_required_count;

	const char* json;
	cgltf_size json_size;

	const void* bin;
	cgltf_size bin_size;

	cgltf_memory_options memory;
	cgltf_file_options file;
} cgltf_data;

cgltf_result cgltf_parse(
		const cgltf_options* options,
		const void* data,
		cgltf_size size,
		cgltf_data** out_data);

cgltf_result cgltf_parse_file(
		const cgltf_options* options,
		const char* path,
		cgltf_data** out_data);

cgltf_result cgltf_load_buffers(
		const cgltf_options* options,
		cgltf_data* data,
		const char* gltf_path);


cgltf_result cgltf_load_buffer_base64(const cgltf_options* options, cgltf_size size, const char* base64, void** out_data);

cgltf_result cgltf_validate(cgltf_data* data);

void cgltf_free(cgltf_data* data);

void cgltf_node_transform_local(const cgltf_node* node, cgltf_float* out_matrix);
void cgltf_node_transform_world(const cgltf_node* node, cgltf_float* out_matrix);

cgltf_bool cgltf_accessor_read_float(const cgltf_accessor* accessor, cgltf_size index, cgltf_float* out, cgltf_size element_size);
cgltf_bool cgltf_accessor_read_uint(const cgltf_accessor* accessor, cgltf_size index, cgltf_uint* out, cgltf_size element_size);
cgltf_size cgltf_accessor_read_index(const cgltf_accessor* accessor, cgltf_size index);

cgltf_size cgltf_num_components(cgltf_type type);

cgltf_size cgltf_accessor_unpack_floats(const cgltf_accessor* accessor, cgltf_float* out, cgltf_size float_count);

cgltf_result cgltf_copy_extras_json(const cgltf_data* data, const cgltf_extras* extras, char* dest, cgltf_size* dest_size);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef CGLTF_H_INCLUDED__ */
