#ifndef SOKOL_GFX_INCLUDED
/*
    sokol_gfx.h -- simple 3D API wrapper

    Project URL: https://github.com/floooh/sokol

    zlib/libpng license

    Copyright (c) 2018 Andre Weissflog

    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.

        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.

        3. This notice may not be removed or altered from any source
        distribution.
*/
#define SOKOL_GFX_INCLUDED (1)
#include <stdint.h>
#include <stdbool.h>

#ifndef SOKOL_API_DECL
#if defined(_WIN32) && defined(SOKOL_DLL) && defined(SOKOL_IMPL)
#define SOKOL_API_DECL __declspec(dllexport)
#elif defined(_WIN32) && defined(SOKOL_DLL)
#define SOKOL_API_DECL __declspec(dllimport)
#else
#define SOKOL_API_DECL extern
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4201)   /* nonstandard extension used: nameless struct/union */
#endif

/*
    Resource id typedefs:

    sg_buffer:      vertex- and index-buffers
    sg_image:       textures and render targets
    sg_shader:      vertex- and fragment-shaders, uniform blocks
    sg_pipeline:    associated shader and vertex-layouts, and render states
    sg_pass:        a bundle of render targets and actions on them
    sg_context:     a 'context handle' for switching between 3D-API contexts

    Instead of pointers, resource creation functions return a 32-bit
    number which uniquely identifies the resource object.

    The 32-bit resource id is split into a 16-bit pool index in the lower bits,
    and a 16-bit 'unique counter' in the upper bits. The index allows fast
    pool lookups, and combined with the unique-mask it allows to detect
    'dangling accesses' (trying to use an object which no longer exists, and
    its pool slot has been reused for a new object)

    The resource ids are wrapped into a struct so that the compiler
    can complain when the wrong resource type is used.
*/
// bqq mod
#include "sokol_defs.h"

/*
    various compile-time constants

    FIXME: it may make sense to convert some of those into defines so
    that the user code can override them.
*/
enum {
    SG_INVALID_ID = 0,
    SG_NUM_SHADER_STAGES = 2,
    SG_NUM_INFLIGHT_FRAMES = 2,
    SG_MAX_COLOR_ATTACHMENTS = 4,
    SG_MAX_SHADERSTAGE_BUFFERS = 8,
    SG_MAX_SHADERSTAGE_IMAGES = 12,
    SG_MAX_SHADERSTAGE_UBS = 4,
    SG_MAX_UB_MEMBERS = 16,
    SG_MAX_VERTEX_ATTRIBUTES = 16,      /* NOTE: actual max vertex attrs can be less on GLES2, see sg_limits! */
    SG_MAX_MIPMAPS = 16,
    SG_MAX_TEXTUREARRAY_LAYERS = 128
};

/*
    sg_backend

    The active 3D-API backend, use the function sg_query_backend()
    to get the currently active backend.

    For returned value corresponds with the compile-time define to select
    a backend, with the only exception of SOKOL_GLES3: this may
    return SG_BACKEND_GLES2 if the backend has to fallback to GLES2 mode
    because GLES3 isn't supported.
*/
typedef enum sg_backend {
    SG_BACKEND_GLCORE33,
    SG_BACKEND_GLES2,
    SG_BACKEND_GLES3,
    SG_BACKEND_D3D11,
    SG_BACKEND_METAL_IOS,
    SG_BACKEND_METAL_MACOS,
    SG_BACKEND_METAL_SIMULATOR,
    SG_BACKEND_DUMMY,
} sg_backend;

/*
    sg_pixel_format

    sokol_gfx.h basically uses the same pixel formats as WebGPU, since these
    are supported on most newer GPUs. GLES2 and WebGL has a much smaller
    subset of available pixel formats. Call sg_query_pixelformat() to check
    at runtime if a pixel format supports the desired features.

    A pixelformat name consist of three parts:

        - components (R, RG, RGB or RGBA)
        - bit width per component (8, 16 or 32)
        - component data type:
            - unsigned normalized (no postfix)
            - signed normalized (SN postfix)
            - unsigned integer (UI postfix)
            - signed integer (SI postfix)
            - float (F postfix)

    Not all pixel formats can be used for everything, call sg_query_pixelformat()
    to inspect the capabilities of a given pixelformat. The function returns
    an sg_pixelformat_info struct with the following bool members:

        - sample: the pixelformat can be sampled as texture at least with
                  nearest filtering
        - filter: the pixelformat can be samples as texture with linear
                  filtering
        - render: the pixelformat can be used for render targets
        - blend:  blending is supported when using the pixelformat for
                  render targets
        - msaa:   multisample-antiliasing is supported when using the
                  pixelformat for render targets
        - depth:  the pixelformat can be used for depth-stencil attachments

    When targeting GLES2/WebGL, the only safe formats to use
    as texture are SG_PIXELFORMAT_R8 and SG_PIXELFORMAT_RGBA8. For rendering
    in GLES2/WebGL, only SG_PIXELFORMAT_RGBA8 is safe. All other formats
    must be checked via sg_query_pixelformats().

    The default pixel format for texture images is SG_PIXELFORMAT_RGBA8.

    The default pixel format for render target images is platform-dependent:
        - for Metal and D3D11 it is SG_PIXELFORMAT_BGRA8
        - for GL backends it is SG_PIXELFORMAT_RGBA8

    This is mainly because of the default framebuffer which is setup outside
    of sokol_gfx.h. On some backends, using BGRA for the default frame buffer
    allows more efficient frame flips. For your own offscreen-render-targets,
    use whatever renderable pixel format is convenient for you.
*/
typedef enum sg_pixel_format {
    _SG_PIXELFORMAT_DEFAULT,    /* value 0 reserved for default-init */
    SG_PIXELFORMAT_NONE,

    SG_PIXELFORMAT_R8,
    SG_PIXELFORMAT_R8SN,
    SG_PIXELFORMAT_R8UI,
    SG_PIXELFORMAT_R8SI,

    SG_PIXELFORMAT_R16,
    SG_PIXELFORMAT_R16SN,
    SG_PIXELFORMAT_R16UI,
    SG_PIXELFORMAT_R16SI,
    SG_PIXELFORMAT_R16F,
    SG_PIXELFORMAT_RG8,
    SG_PIXELFORMAT_RG8SN,
    SG_PIXELFORMAT_RG8UI,
    SG_PIXELFORMAT_RG8SI,

    SG_PIXELFORMAT_R32UI,
    SG_PIXELFORMAT_R32SI,
    SG_PIXELFORMAT_R32F,
    SG_PIXELFORMAT_RG16,
    SG_PIXELFORMAT_RG16SN,
    SG_PIXELFORMAT_RG16UI,
    SG_PIXELFORMAT_RG16SI,
    SG_PIXELFORMAT_RG16F,
    SG_PIXELFORMAT_RGBA8,
    SG_PIXELFORMAT_RGBA8SN,
    SG_PIXELFORMAT_RGBA8UI,
    SG_PIXELFORMAT_RGBA8SI,
    SG_PIXELFORMAT_BGRA8,
    SG_PIXELFORMAT_RGB10A2,
    SG_PIXELFORMAT_RG11B10F,
    SG_PIXELFORMAT_BQQ_SRGBA8,

    SG_PIXELFORMAT_RG32UI,
    SG_PIXELFORMAT_RG32SI,
    SG_PIXELFORMAT_RG32F,
    SG_PIXELFORMAT_RGBA16,
    SG_PIXELFORMAT_RGBA16SN,
    SG_PIXELFORMAT_RGBA16UI,
    SG_PIXELFORMAT_RGBA16SI,
    SG_PIXELFORMAT_RGBA16F,

    SG_PIXELFORMAT_RGBA32UI,
    SG_PIXELFORMAT_RGBA32SI,
    SG_PIXELFORMAT_RGBA32F,

    SG_PIXELFORMAT_DEPTH,
    SG_PIXELFORMAT_DEPTH_STENCIL,

    SG_PIXELFORMAT_BC1_RGBA,
    SG_PIXELFORMAT_BQQ_BC1_SRGB,
    SG_PIXELFORMAT_BC2_RGBA,
    SG_PIXELFORMAT_BC3_RGBA,
    SG_PIXELFORMAT_BQQ_BC3_SRGB,
    SG_PIXELFORMAT_BC4_R,
    SG_PIXELFORMAT_BC4_RSN,
    SG_PIXELFORMAT_BC5_RG,
    SG_PIXELFORMAT_BC5_RGSN,
    SG_PIXELFORMAT_BC6H_RGBF,
    SG_PIXELFORMAT_BC6H_RGBUF,
    SG_PIXELFORMAT_BC7_RGBA,
    SG_PIXELFORMAT_PVRTC_RGB_2BPP,
    SG_PIXELFORMAT_PVRTC_RGB_4BPP,
    SG_PIXELFORMAT_PVRTC_RGBA_2BPP,
    SG_PIXELFORMAT_PVRTC_RGBA_4BPP,
    SG_PIXELFORMAT_ETC2_RGB8,
    SG_PIXELFORMAT_ETC2_RGB8A1,


    _SG_PIXELFORMAT_NUM,
    _SG_PIXELFORMAT_FORCE_U32 = 0x7FFFFFFF
} sg_pixel_format;

/*
    Runtime information about a pixel format, returned
    by sg_query_pixelformat().
*/
typedef struct sg_pixelformat_info {
    bool sample;        /* pixel format can be sampled in shaders */
    bool filter;        /* pixel format can be sampled with filtering */
    bool render;        /* pixel format can be used as render target */
    bool blend;         /* alpha-blending is supported */
    bool msaa;          /* pixel format can be used as MSAA render target */
    bool depth;         /* pixel format is a depth format */
} sg_pixelformat_info;

/*
    Runtime information about available optional features,
    returned by sg_query_features()
*/
typedef struct sg_features {
    bool instancing;
    bool origin_top_left;
    bool multiple_render_targets;
    bool msaa_render_targets;
    bool imagetype_3d;          /* creation of SG_IMAGETYPE_3D images is supported */
    bool imagetype_array;       /* creation of SG_IMAGETYPE_ARRAY images is supported */
    bool image_clamp_to_border; /* border color and clamp-to-border UV-wrap mode is supported */
} sg_features;

/*
    Runtime information about resource limits, returned by sg_query_limit()
*/
typedef struct sg_limits {
    uint32_t max_image_size_2d;         /* max width/height of SG_IMAGETYPE_2D images */
    uint32_t max_image_size_cube;       /* max width/height of SG_IMAGETYPE_CUBE images */
    uint32_t max_image_size_3d;         /* max width/height/depth of SG_IMAGETYPE_3D images */
    uint32_t max_image_size_array;
    uint32_t max_image_array_layers;
    uint32_t max_vertex_attrs;          /* <= SG_MAX_VERTEX_ATTRIBUTES (only on some GLES2 impls) */
} sg_limits;

/*
    sg_resource_state

    The current state of a resource in its resource pool.
    Resources start in the INITIAL state, which means the
    pool slot is unoccupied and can be allocated. When a resource is
    created, first an id is allocated, and the resource pool slot
    is set to state ALLOC. After allocation, the resource is
    initialized, which may result in the VALID or FAILED state. The
    reason why allocation and initialization are separate is because
    some resource types (e.g. buffers and images) might be asynchronously
    initialized by the user application. If a resource which is not
    in the VALID state is attempted to be used for rendering, rendering
    operations will silently be dropped.

    The special INVALID state is returned in sg_query_xxx_state() if no
    resource object exists for the provided resource id.
*/
typedef enum sg_resource_state {
    SG_RESOURCESTATE_INITIAL,
    SG_RESOURCESTATE_ALLOC,
    SG_RESOURCESTATE_VALID,
    SG_RESOURCESTATE_FAILED,
    SG_RESOURCESTATE_INVALID,
    _SG_RESOURCESTATE_FORCE_U32 = 0x7FFFFFFF
} sg_resource_state;

/*
    sg_usage

    A resource usage hint describing the update strategy of
    buffers and images. This is used in the sg_buffer_desc.usage
    and sg_image_desc.usage members when creating buffers
    and images:

    SG_USAGE_IMMUTABLE:     the resource will never be updated with
                            new data, instead the data content of the
                            resource must be provided on creation
    SG_USAGE_DYNAMIC:       the resource will be updated infrequently
                            with new data (this could range from "once
                            after creation", to "quite often but not
                            every frame")
    SG_USAGE_STREAM:        the resource will be updated each frame
                            with new content

    The rendering backends use this hint to prevent that the
    CPU needs to wait for the GPU when attempting to update
    a resource that might be currently accessed by the GPU.

    Resource content is updated with the function sg_update_buffer() for
    buffer objects, and sg_update_image() for image objects. Only
    one update is allowed per frame and resource object. The
    application must update all data required for rendering (this
    means that the update data can be smaller than the resource size,
    if only a part of the overall resource size is used for rendering,
    you only need to make sure that the data that *is* used is valid.

    The default usage is SG_USAGE_IMMUTABLE.
*/
typedef enum sg_usage {
    _SG_USAGE_DEFAULT,      /* value 0 reserved for default-init */
    SG_USAGE_IMMUTABLE,
    SG_USAGE_DYNAMIC,
    SG_USAGE_STREAM,
    _SG_USAGE_NUM,
    _SG_USAGE_FORCE_U32 = 0x7FFFFFFF
} sg_usage;

/*
    sg_buffer_type

    This indicates whether a buffer contains vertex- or index-data,
    used in the sg_buffer_desc.type member when creating a buffer.

    The default value is SG_BUFFERTYPE_VERTEXBUFFER.
*/
typedef enum sg_buffer_type {
    _SG_BUFFERTYPE_DEFAULT,         /* value 0 reserved for default-init */
    SG_BUFFERTYPE_VERTEXBUFFER,
    SG_BUFFERTYPE_INDEXBUFFER,
    _SG_BUFFERTYPE_NUM,
    _SG_BUFFERTYPE_FORCE_U32 = 0x7FFFFFFF
} sg_buffer_type;

/*
    sg_index_type

    Indicates whether indexed rendering (fetching vertex-indices from an
    index buffer) is used, and if yes, the index data type (16- or 32-bits).
    This is used in the sg_pipeline_desc.index_type member when creating a
    pipeline object.

    The default index type is SG_INDEXTYPE_NONE.
*/
typedef enum sg_index_type {
    _SG_INDEXTYPE_DEFAULT,   /* value 0 reserved for default-init */
    SG_INDEXTYPE_NONE,
    SG_INDEXTYPE_UINT16,
    SG_INDEXTYPE_UINT32,
    _SG_INDEXTYPE_NUM,
    _SG_INDEXTYPE_FORCE_U32 = 0x7FFFFFFF
} sg_index_type;

/*
    sg_image_type

    Indicates the basic image type (2D-texture, cubemap, 3D-texture
    or 2D-array-texture). 3D- and array-textures are not supported
    on the GLES2/WebGL backend. The image type is used in the
    sg_image_desc.type member when creating an image.

    The default image type when creating an image is SG_IMAGETYPE_2D.
*/
typedef enum sg_image_type {
    _SG_IMAGETYPE_DEFAULT,  /* value 0 reserved for default-init */
    SG_IMAGETYPE_2D,
    SG_IMAGETYPE_CUBE,
    SG_IMAGETYPE_3D,
    SG_IMAGETYPE_ARRAY,
    _SG_IMAGETYPE_NUM,
    _SG_IMAGETYPE_FORCE_U32 = 0x7FFFFFFF
} sg_image_type;

/*
    sg_cube_face

    The cubemap faces. Use these as indices in the sg_image_desc.content
    array.
*/
typedef enum sg_cube_face {
    SG_CUBEFACE_POS_X,
    SG_CUBEFACE_NEG_X,
    SG_CUBEFACE_POS_Y,
    SG_CUBEFACE_NEG_Y,
    SG_CUBEFACE_POS_Z,
    SG_CUBEFACE_NEG_Z,
    SG_CUBEFACE_NUM,
    _SG_CUBEFACE_FORCE_U32 = 0x7FFFFFFF
} sg_cube_face;

/*
    sg_shader_stage

    There are 2 shader stages: vertex- and fragment-shader-stage.
    Each shader stage consists of:

    - one slot for a shader function (provided as source- or byte-code)
    - SG_MAX_SHADERSTAGE_UBS slots for uniform blocks
    - SG_MAX_SHADERSTAGE_IMAGES slots for images used as textures by
      the shader function
*/
typedef enum sg_shader_stage {
    SG_SHADERSTAGE_VS,
    SG_SHADERSTAGE_FS,
    _SG_SHADERSTAGE_FORCE_U32 = 0x7FFFFFFF
} sg_shader_stage;

/*
    sg_primitive_type

    This is the common subset of 3D primitive types supported across all 3D
    APIs. This is used in the sg_pipeline_desc.primitive_type member when
    creating a pipeline object.

    The default primitive type is SG_PRIMITIVETYPE_TRIANGLES.
*/
typedef enum sg_primitive_type {
    _SG_PRIMITIVETYPE_DEFAULT,  /* value 0 reserved for default-init */
    SG_PRIMITIVETYPE_POINTS,
    SG_PRIMITIVETYPE_LINES,
    SG_PRIMITIVETYPE_LINE_STRIP,
    SG_PRIMITIVETYPE_TRIANGLES,
    SG_PRIMITIVETYPE_TRIANGLE_STRIP,
    _SG_PRIMITIVETYPE_NUM,
    _SG_PRIMITIVETYPE_FORCE_U32 = 0x7FFFFFFF
} sg_primitive_type;

/*
    sg_filter

    The filtering mode when sampling a texture image. This is
    used in the sg_image_desc.min_filter and sg_image_desc.mag_filter
    members when creating an image object.

    The default filter mode is SG_FILTER_NEAREST.
*/
typedef enum sg_filter {
    _SG_FILTER_DEFAULT, /* value 0 reserved for default-init */
    SG_FILTER_NEAREST,
    SG_FILTER_LINEAR,
    SG_FILTER_NEAREST_MIPMAP_NEAREST,
    SG_FILTER_NEAREST_MIPMAP_LINEAR,
    SG_FILTER_LINEAR_MIPMAP_NEAREST,
    SG_FILTER_LINEAR_MIPMAP_LINEAR,
    _SG_FILTER_NUM,
    _SG_FILTER_FORCE_U32 = 0x7FFFFFFF
} sg_filter;

/*
    sg_wrap

    The texture coordinates wrapping mode when sampling a texture
    image. This is used in the sg_image_desc.wrap_u, .wrap_v
    and .wrap_w members when creating an image.

    The default wrap mode is SG_WRAP_REPEAT.

    NOTE: SG_WRAP_CLAMP_TO_BORDER is not supported on all backends
    and platforms. To check for support, call sg_query_features()
    and check the "clamp_to_border" boolean in the returned
    sg_features struct.

    Platforms which don't support SG_WRAP_CLAMP_TO_BORDER will silently fall back
    to SG_WRAP_CLAMP_TO_EDGE without a validation error.

    Platforms which support clamp-to-border are:

        - all desktop GL platforms
        - Metal on macOS
        - D3D11

    Platforms which do not support clamp-to-border:

        - GLES2/3 and WebGL/WebGL2
        - Metal on iOS
*/
typedef enum sg_wrap {
    _SG_WRAP_DEFAULT,   /* value 0 reserved for default-init */
    SG_WRAP_REPEAT,
    SG_WRAP_CLAMP_TO_EDGE,
    SG_WRAP_CLAMP_TO_BORDER,
    SG_WRAP_MIRRORED_REPEAT,
    _SG_WRAP_NUM,
    _SG_WRAP_FORCE_U32 = 0x7FFFFFFF
} sg_wrap;

/*
    sg_border_color

    The border color to use when sampling a texture, and the UV wrap
    mode is SG_WRAP_CLAMP_TO_BORDER.

    The default border color is SG_BORDERCOLOR_OPAQUE_BLACK
*/
typedef enum sg_border_color {
    _SG_BORDERCOLOR_DEFAULT,    /* value 0 reserved for default-init */
    SG_BORDERCOLOR_TRANSPARENT_BLACK,
    SG_BORDERCOLOR_OPAQUE_BLACK,
    SG_BORDERCOLOR_OPAQUE_WHITE,
    _SG_BORDERCOLOR_NUM,
    _SG_BORDERCOLOR_FORCE_U32 = 0x7FFFFFFF
} sg_border_color;

/*
    sg_vertex_format

    The data type of a vertex component. This is used to describe
    the layout of vertex data when creating a pipeline object.
*/
typedef enum sg_vertex_format {
    SG_VERTEXFORMAT_INVALID,
    SG_VERTEXFORMAT_FLOAT,
    SG_VERTEXFORMAT_FLOAT2,
    SG_VERTEXFORMAT_FLOAT3,
    SG_VERTEXFORMAT_FLOAT4,
    SG_VERTEXFORMAT_BYTE4,
    SG_VERTEXFORMAT_BYTE4N,
    SG_VERTEXFORMAT_UBYTE4,
    SG_VERTEXFORMAT_UBYTE4N,
    SG_VERTEXFORMAT_SHORT2,
    SG_VERTEXFORMAT_SHORT2N,
    SG_VERTEXFORMAT_USHORT2N,
    SG_VERTEXFORMAT_SHORT4,
    SG_VERTEXFORMAT_SHORT4N,
    SG_VERTEXFORMAT_USHORT4N,
    SG_VERTEXFORMAT_UINT10_N2,
    _SG_VERTEXFORMAT_NUM,
    _SG_VERTEXFORMAT_FORCE_U32 = 0x7FFFFFFF
} sg_vertex_format;

/*
    sg_vertex_step

    Defines whether the input pointer of a vertex input stream is advanced
    'per vertex' or 'per instance'. The default step-func is
    SG_VERTEXSTEP_PER_VERTEX. SG_VERTEXSTEP_PER_INSTANCE is used with
    instanced-rendering.

    The vertex-step is part of the vertex-layout definition
    when creating pipeline objects.
*/
typedef enum sg_vertex_step {
    _SG_VERTEXSTEP_DEFAULT,     /* value 0 reserved for default-init */
    SG_VERTEXSTEP_PER_VERTEX,
    SG_VERTEXSTEP_PER_INSTANCE,
    _SG_VERTEXSTEP_NUM,
    _SG_VERTEXSTEP_FORCE_U32 = 0x7FFFFFFF
} sg_vertex_step;

/*
    sg_uniform_type

    The data type of a uniform block member. This is used to
    describe the internal layout of uniform blocks when creating
    a shader object.
*/
typedef enum sg_uniform_type {
    SG_UNIFORMTYPE_INVALID,
    SG_UNIFORMTYPE_FLOAT,
    SG_UNIFORMTYPE_FLOAT2,
    SG_UNIFORMTYPE_FLOAT3,
    SG_UNIFORMTYPE_FLOAT4,
    SG_UNIFORMTYPE_MAT4,
    _SG_UNIFORMTYPE_NUM,
    _SG_UNIFORMTYPE_FORCE_U32 = 0x7FFFFFFF
} sg_uniform_type;

/*
    sg_cull_mode

    The face-culling mode, this is used in the
    sg_pipeline_desc.rasterizer.cull_mode member when creating a
    pipeline object.

    The default cull mode is SG_CULLMODE_NONE
*/
typedef enum sg_cull_mode {
    _SG_CULLMODE_DEFAULT,   /* value 0 reserved for default-init */
    SG_CULLMODE_NONE,
    SG_CULLMODE_FRONT,
    SG_CULLMODE_BACK,
    _SG_CULLMODE_NUM,
    _SG_CULLMODE_FORCE_U32 = 0x7FFFFFFF
} sg_cull_mode;

/*
    sg_face_winding

    The vertex-winding rule that determines a front-facing primitive. This
    is used in the member sg_pipeline_desc.rasterizer.face_winding
    when creating a pipeline object.

    The default winding is SG_FACEWINDING_CW (clockwise)
*/
typedef enum sg_face_winding {
    _SG_FACEWINDING_DEFAULT,    /* value 0 reserved for default-init */
    SG_FACEWINDING_CCW,
    SG_FACEWINDING_CW,
    _SG_FACEWINDING_NUM,
    _SG_FACEWINDING_FORCE_U32 = 0x7FFFFFFF
} sg_face_winding;

/*
    sg_compare_func

    The compare-function for depth- and stencil-ref tests.
    This is used when creating pipeline objects in the members:

    sg_pipeline_desc
        .depth_stencil
            .depth_compare_func
            .stencil_front.compare_func
            .stencil_back.compare_func

    The default compare func for depth- and stencil-tests is
    SG_COMPAREFUNC_ALWAYS.
*/
typedef enum sg_compare_func {
    _SG_COMPAREFUNC_DEFAULT,    /* value 0 reserved for default-init */
    SG_COMPAREFUNC_NEVER,
    SG_COMPAREFUNC_LESS,
    SG_COMPAREFUNC_EQUAL,
    SG_COMPAREFUNC_LESS_EQUAL,
    SG_COMPAREFUNC_GREATER,
    SG_COMPAREFUNC_NOT_EQUAL,
    SG_COMPAREFUNC_GREATER_EQUAL,
    SG_COMPAREFUNC_ALWAYS,
    _SG_COMPAREFUNC_NUM,
    _SG_COMPAREFUNC_FORCE_U32 = 0x7FFFFFFF
} sg_compare_func;

/*
    sg_stencil_op

    The operation performed on a currently stored stencil-value when a
    comparison test passes or fails. This is used when creating a pipeline
    object in the members:

    sg_pipeline_desc
        .depth_stencil
            .stencil_front
                .fail_op
                .depth_fail_op
                .pass_op
            .stencil_back
                .fail_op
                .depth_fail_op
                .pass_op

    The default value is SG_STENCILOP_KEEP.
*/
typedef enum sg_stencil_op {
    _SG_STENCILOP_DEFAULT,      /* value 0 reserved for default-init */
    SG_STENCILOP_KEEP,
    SG_STENCILOP_ZERO,
    SG_STENCILOP_REPLACE,
    SG_STENCILOP_INCR_CLAMP,
    SG_STENCILOP_DECR_CLAMP,
    SG_STENCILOP_INVERT,
    SG_STENCILOP_INCR_WRAP,
    SG_STENCILOP_DECR_WRAP,
    _SG_STENCILOP_NUM,
    _SG_STENCILOP_FORCE_U32 = 0x7FFFFFFF
} sg_stencil_op;

/*
    sg_blend_factor

    The source and destination factors in blending operations.
    This is used in the following members when creating a pipeline object:

    sg_pipeline_desc
        .blend
            .src_factor_rgb
            .dst_factor_rgb
            .src_factor_alpha
            .dst_factor_alpha

    The default value is SG_BLENDFACTOR_ONE for source
    factors, and SG_BLENDFACTOR_ZERO for destination factors.
*/
typedef enum sg_blend_factor {
    _SG_BLENDFACTOR_DEFAULT,    /* value 0 reserved for default-init */
    SG_BLENDFACTOR_ZERO,
    SG_BLENDFACTOR_ONE,
    SG_BLENDFACTOR_SRC_COLOR,
    SG_BLENDFACTOR_ONE_MINUS_SRC_COLOR,
    SG_BLENDFACTOR_SRC_ALPHA,
    SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
    SG_BLENDFACTOR_DST_COLOR,
    SG_BLENDFACTOR_ONE_MINUS_DST_COLOR,
    SG_BLENDFACTOR_DST_ALPHA,
    SG_BLENDFACTOR_ONE_MINUS_DST_ALPHA,
    SG_BLENDFACTOR_SRC_ALPHA_SATURATED,
    SG_BLENDFACTOR_BLEND_COLOR,
    SG_BLENDFACTOR_ONE_MINUS_BLEND_COLOR,
    SG_BLENDFACTOR_BLEND_ALPHA,
    SG_BLENDFACTOR_ONE_MINUS_BLEND_ALPHA,
    _SG_BLENDFACTOR_NUM,
    _SG_BLENDFACTOR_FORCE_U32 = 0x7FFFFFFF
} sg_blend_factor;

/*
    sg_blend_op

    Describes how the source and destination values are combined in the
    fragment blending operation. It is used in the following members when
    creating a pipeline object:

    sg_pipeline_desc
        .blend
            .op_rgb
            .op_alpha

    The default value is SG_BLENDOP_ADD.
*/
typedef enum sg_blend_op {
    _SG_BLENDOP_DEFAULT,    /* value 0 reserved for default-init */
    SG_BLENDOP_ADD,
    SG_BLENDOP_SUBTRACT,
    SG_BLENDOP_REVERSE_SUBTRACT,
    _SG_BLENDOP_NUM,
    _SG_BLENDOP_FORCE_U32 = 0x7FFFFFFF
} sg_blend_op;

/*
    sg_color_mask

    Selects the color channels when writing a fragment color to the
    framebuffer. This is used in the members
    sg_pipeline_desc.blend.color_write_mask when creating a pipeline object.

    The default colormask is SG_COLORMASK_RGBA (write all colors channels)
*/
typedef enum sg_color_mask {
    _SG_COLORMASK_DEFAULT = 0,      /* value 0 reserved for default-init */
    SG_COLORMASK_NONE = (0x10),     /* special value for 'all channels disabled */
    SG_COLORMASK_R = (1<<0),
    SG_COLORMASK_G = (1<<1),
    SG_COLORMASK_B = (1<<2),
    SG_COLORMASK_A = (1<<3),
    SG_COLORMASK_RGB = 0x7,
    SG_COLORMASK_RGBA = 0xF,
    _SG_COLORMASK_FORCE_U32 = 0x7FFFFFFF
} sg_color_mask;

/*
    sg_action

    Defines what action should be performed at the start of a render pass:

    SG_ACTION_CLEAR:    clear the render target image
    SG_ACTION_LOAD:     load the previous content of the render target image
    SG_ACTION_DONTCARE: leave the render target image content undefined

    This is used in the sg_pass_action structure.

    The default action for all pass attachments is SG_ACTION_CLEAR, with the
    clear color rgba = {0.5f, 0.5f, 0.5f, 1.0f], depth=1.0 and stencil=0.

    If you want to override the default behaviour, it is important to not
    only set the clear color, but the 'action' field as well (as long as this
    is in its _SG_ACTION_DEFAULT, the value fields will be ignored).
*/
typedef enum sg_action {
    _SG_ACTION_DEFAULT,
    SG_ACTION_CLEAR,
    SG_ACTION_LOAD,
    SG_ACTION_DONTCARE,
    _SG_ACTION_NUM,
    _SG_ACTION_FORCE_U32 = 0x7FFFFFFF
} sg_action;

/*
    sg_pass_action

    The sg_pass_action struct defines the actions to be performed
    at the start of a rendering pass in the functions sg_begin_pass()
    and sg_begin_default_pass().

    A separate action and clear values can be defined for each
    color attachment, and for the depth-stencil attachment.

    The default clear values are defined by the macros:

    - SG_DEFAULT_CLEAR_RED:     0.5f
    - SG_DEFAULT_CLEAR_GREEN:   0.5f
    - SG_DEFAULT_CLEAR_BLUE:    0.5f
    - SG_DEFAULT_CLEAR_ALPHA:   1.0f
    - SG_DEFAULT_CLEAR_DEPTH:   1.0f
    - SG_DEFAULT_CLEAR_STENCIL: 0
*/
typedef struct sg_color_attachment_action {
    sg_action action;
    float val[4];
} sg_color_attachment_action;

typedef struct sg_depth_attachment_action {
    sg_action action;
    float val;
} sg_depth_attachment_action;

typedef struct sg_stencil_attachment_action {
    sg_action action;
    uint8_t val;
} sg_stencil_attachment_action;

typedef struct sg_pass_action {
    uint32_t _start_canary;
    sg_color_attachment_action colors[SG_MAX_COLOR_ATTACHMENTS];
    sg_depth_attachment_action depth;
    sg_stencil_attachment_action stencil;
    uint32_t _end_canary;
} sg_pass_action;

/*
    sg_bindings

    The sg_bindings structure defines the resource binding slots
    of the sokol_gfx render pipeline, used as argument to the
    sg_apply_bindings() function.

    A resource binding struct contains:

    - 1..N vertex buffers
    - 0..N vertex buffer offsets
    - 0..1 index buffers
    - 0..1 index buffer offsets
    - 0..N vertex shader stage images
    - 0..N fragment shader stage images

    The max number of vertex buffer and shader stage images
    are defined by the SG_MAX_SHADERSTAGE_BUFFERS and
    SG_MAX_SHADERSTAGE_IMAGES configuration constants.

    The optional buffer offsets can be used to group different chunks
    of vertex- and/or index-data into the same buffer objects.
*/
typedef struct sg_bindings {
    uint32_t _start_canary;
    sg_buffer vertex_buffers[SG_MAX_SHADERSTAGE_BUFFERS];
    int vertex_buffer_offsets[SG_MAX_SHADERSTAGE_BUFFERS];
    sg_buffer index_buffer;
    int index_buffer_offset;
    sg_image vs_images[SG_MAX_SHADERSTAGE_IMAGES];
    sg_image fs_images[SG_MAX_SHADERSTAGE_IMAGES];
    uint32_t _end_canary;
} sg_bindings;

/*
    sg_buffer_desc

    Creation parameters for sg_buffer objects, used in the
    sg_make_buffer() call.

    The default configuration is:

    .size:      0       (this *must* be set to a valid size in bytes)
    .type:      SG_BUFFERTYPE_VERTEXBUFFER
    .usage:     SG_USAGE_IMMUTABLE
    .content    0
    .label      0       (optional string label for trace hooks)

    The dbg_label will be ignored by sokol_gfx.h, it is only useful
    when hooking into sg_make_buffer() or sg_init_buffer() via
    the sg_install_trace_hook

    ADVANCED TOPIC: Injecting native 3D-API buffers:

    The following struct members allow to inject your own GL, Metal
    or D3D11 buffers into sokol_gfx:

    .gl_buffers[SG_NUM_INFLIGHT_FRAMES]
    .mtl_buffers[SG_NUM_INFLIGHT_FRAMES]
    .d3d11_buffer

    You must still provide all other members except the .content member, and
    these must match the creation parameters of the native buffers you
    provide. For SG_USAGE_IMMUTABLE, only provide a single native 3D-API
    buffer, otherwise you need to provide SG_NUM_INFLIGHT_FRAMES buffers
    (only for GL and Metal, not D3D11). Providing multiple buffers for GL and
    Metal is necessary because sokol_gfx will rotate through them when
    calling sg_update_buffer() to prevent lock-stalls.

    Note that it is expected that immutable injected buffer have already been
    initialized with content, and the .content member must be 0!

    Also you need to call sg_reset_state_cache() after calling native 3D-API
    functions, and before calling any sokol_gfx function.
*/
typedef struct sg_buffer_desc {
    uint32_t _start_canary;
    int size;
    sg_buffer_type type;
    sg_usage usage;
    const void* content;
    const char* label;
    /* GL specific */
    uint32_t gl_buffers[SG_NUM_INFLIGHT_FRAMES];
    /* Metal specific */
    const void* mtl_buffers[SG_NUM_INFLIGHT_FRAMES];
    /* D3D11 specific */
    const void* d3d11_buffer;
    uint32_t _end_canary;
} sg_buffer_desc;

/*
    sg_subimage_content

    Pointer to and size of a subimage-surface data, this is
    used to describe the initial content of immutable-usage images,
    or for updating a dynamic- or stream-usage images.

    For 3D- or array-textures, one sg_subimage_content item
    describes an entire mipmap level consisting of all array- or
    3D-slices of the mipmap level. It is only possible to update
    an entire mipmap level, not parts of it.
*/
typedef struct sg_subimage_content {
    const void* ptr;    /* pointer to subimage data */
    int size;           /* size in bytes of pointed-to subimage data */
} sg_subimage_content;

/*
    sg_image_content

    Defines the content of an image through a 2D array
    of sg_subimage_content structs. The first array dimension
    is the cubemap face, and the second array dimension the
    mipmap level.
*/
typedef struct sg_image_content {
    sg_subimage_content subimage[SG_CUBEFACE_NUM][SG_MAX_MIPMAPS];
} sg_image_content;

/*
    sg_image_desc

    Creation parameters for sg_image objects, used in the
    sg_make_image() call.

    The default configuration is:

    .type:              SG_IMAGETYPE_2D
    .render_target:     false
    .width              0 (must be set to >0)
    .height             0 (must be set to >0)
    .depth/.layers:     1
    .num_mipmaps:       1
    .usage:             SG_USAGE_IMMUTABLE
    .pixel_format:      SG_PIXELFORMAT_RGBA8 for textures, backend-dependent
                        for render targets (RGBA8 or BGRA8)
    .sample_count:      1 (only used in render_targets)
    .min_filter:        SG_FILTER_NEAREST
    .mag_filter:        SG_FILTER_NEAREST
    .wrap_u:            SG_WRAP_REPEAT
    .wrap_v:            SG_WRAP_REPEAT
    .wrap_w:            SG_WRAP_REPEAT (only SG_IMAGETYPE_3D)
    .border_color       SG_BORDERCOLOR_OPAQUE_BLACK
    .max_anisotropy     1 (must be 1..16)
    .min_lod            0.0f
    .max_lod            FLT_MAX
    .content            an sg_image_content struct to define the initial content
    .label              0       (optional string label for trace hooks)

    SG_IMAGETYPE_ARRAY and SG_IMAGETYPE_3D are not supported on
    WebGL/GLES2, use sg_query_features().imagetype_array and
    sg_query_features().imagetype_3d at runtime to check
    if array- and 3D-textures are supported.

    Images with usage SG_USAGE_IMMUTABLE must be fully initialized by
    providing a valid .content member which points to
    initialization data.

    ADVANCED TOPIC: Injecting native 3D-API textures:

    The following struct members allow to inject your own GL, Metal
    or D3D11 textures into sokol_gfx:

    .gl_textures[SG_NUM_INFLIGHT_FRAMES]
    .mtl_textures[SG_NUM_INFLIGHT_FRAMES]
    .d3d11_texture

    The same rules apply as for injecting native buffers
    (see sg_buffer_desc documentation for more details).
*/
typedef struct sg_image_desc {
    uint32_t _start_canary;
    sg_image_type type;
    bool render_target;
    int width;
    int height;
    union {
        int depth;
        int layers;
    };
    int num_mipmaps;
    sg_usage usage;
    sg_pixel_format pixel_format;
    int sample_count;
    sg_filter min_filter;
    sg_filter mag_filter;
    sg_wrap wrap_u;
    sg_wrap wrap_v;
    sg_wrap wrap_w;
    sg_border_color border_color;
    uint32_t max_anisotropy;
    float min_lod;
    float max_lod;
    sg_image_content content;
    const char* label;
    bool bqq_copy_target;
    /* GL specific */
    uint32_t gl_textures[SG_NUM_INFLIGHT_FRAMES];
    /* Metal specific */
    const void* mtl_textures[SG_NUM_INFLIGHT_FRAMES];
    /* D3D11 specific */
    const void* d3d11_texture;
    uint32_t _end_canary;
} sg_image_desc;

/*
    sg_shader_desc

    The structure sg_shader_desc defines all creation parameters
    for shader programs, used as input to the sg_make_shader() function:

    - reflection information for vertex attributes (vertex shader inputs):
        - vertex attribute name (required for GLES2, optional for GLES3 and GL)
        - a semantic name and index (required for D3D11)
    - for each vertex- and fragment-shader-stage:
        - the shader source or bytecode
        - an optional entry function name
        - reflection info for each uniform block used by the shader stage:
            - the size of the uniform block in bytes
            - reflection info for each uniform block member (only required for GL backends):
                - member name
                - member type (SG_UNIFORMTYPE_xxx)
                - if the member is an array, the number of array items
        - reflection info for the texture images used by the shader stage:
            - the image type (SG_IMAGETYPE_xxx)
            - the name of the texture sampler (required for GLES2, optional everywhere else)

    For all GL backends, shader source-code must be provided. For D3D11 and Metal,
    either shader source-code or byte-code can be provided.

    For D3D11, if source code is provided, the d3dcompiler_47.dll will be loaded
    on demand. If this fails, shader creation will fail.
*/
typedef struct sg_shader_attr_desc {
    const char* name;           /* GLSL vertex attribute name (only required for GLES2) */
    const char* sem_name;       /* HLSL semantic name */
    int sem_index;              /* HLSL semantic index */
} sg_shader_attr_desc;

typedef struct sg_shader_uniform_desc {
    const char* name;
    sg_uniform_type type;
    int array_count;
} sg_shader_uniform_desc;

typedef struct sg_shader_uniform_block_desc {
    int size;
    sg_shader_uniform_desc uniforms[SG_MAX_UB_MEMBERS];
} sg_shader_uniform_block_desc;

typedef struct sg_shader_image_desc {
    const char* name;
    sg_image_type type;
} sg_shader_image_desc;

typedef struct sg_shader_stage_desc {
    const char* source;
    const uint8_t* byte_code;
    int byte_code_size;
    const char* entry;
    sg_shader_uniform_block_desc uniform_blocks[SG_MAX_SHADERSTAGE_UBS];
    sg_shader_image_desc images[SG_MAX_SHADERSTAGE_IMAGES];
} sg_shader_stage_desc;

typedef struct sg_shader_desc {
    uint32_t _start_canary;
    sg_shader_attr_desc attrs[SG_MAX_VERTEX_ATTRIBUTES];
    sg_shader_stage_desc vs;
    sg_shader_stage_desc fs;
    const char* label;
    uint32_t _end_canary;
} sg_shader_desc;

/*
    sg_pipeline_desc

    The sg_pipeline_desc struct defines all creation parameters
    for an sg_pipeline object, used as argument to the
    sg_make_pipeline() function:

    - the vertex layout for all input vertex buffers
    - a shader object
    - the 3D primitive type (points, lines, triangles, ...)
    - the index type (none, 16- or 32-bit)
    - depth-stencil state
    - alpha-blending state
    - rasterizer state

    If the vertex data has no gaps between vertex components, you can omit
    the .layout.buffers[].stride and layout.attrs[].offset items (leave them
    default-initialized to 0), sokol will then compute the offsets and strides
    from the vertex component formats (.layout.attrs[].offset). Please note
    that ALL vertex attribute offsets must be 0 in order for the the
    automatic offset computation to kick in.

    The default configuration is as follows:

    .layout:
        .buffers[]:         vertex buffer layouts
            .stride:        0 (if no stride is given it will be computed)
            .step_func      SG_VERTEXSTEP_PER_VERTEX
            .step_rate      1
        .attrs[]:           vertex attribute declarations
            .buffer_index   0 the vertex buffer bind slot
            .offset         0 (offsets can be omitted if the vertex layout has no gaps)
            .format         SG_VERTEXFORMAT_INVALID (must be initialized!)
    .shader:            0 (must be intilized with a valid sg_shader id!)
    .primitive_type:    SG_PRIMITIVETYPE_TRIANGLES
    .index_type:        SG_INDEXTYPE_NONE
    .depth_stencil:
        .stencil_front, .stencil_back:
            .fail_op:               SG_STENCILOP_KEEP
            .depth_fail_op:         SG_STENCILOP_KEEP
            .pass_op:               SG_STENCILOP_KEEP
            .compare_func           SG_COMPAREFUNC_ALWAYS
        .depth_compare_func:    SG_COMPAREFUNC_ALWAYS
        .depth_write_enabled:   false
        .stencil_enabled:       false
        .stencil_read_mask:     0
        .stencil_write_mask:    0
        .stencil_ref:           0
    .blend:
        .enabled:               false
        .src_factor_rgb:        SG_BLENDFACTOR_ONE
        .dst_factor_rgb:        SG_BLENDFACTOR_ZERO
        .op_rgb:                SG_BLENDOP_ADD
        .src_factor_alpha:      SG_BLENDFACTOR_ONE
        .dst_factor_alpha:      SG_BLENDFACTOR_ZERO
        .op_alpha:              SG_BLENDOP_ADD
        .color_write_mask:      SG_COLORMASK_RGBA
        .color_attachment_count 1
        .color_format           SG_PIXELFORMAT_RGBA8
        .depth_format           SG_PIXELFORMAT_DEPTHSTENCIL
        .blend_color:           { 0.0f, 0.0f, 0.0f, 0.0f }
    .rasterizer:
        .alpha_to_coverage_enabled:     false
        .cull_mode:                     SG_CULLMODE_NONE
        .face_winding:                  SG_FACEWINDING_CW
        .sample_count:                  1
        .depth_bias:                    0.0f
        .depth_bias_slope_scale:        0.0f
        .depth_bias_clamp:              0.0f
    .label  0       (optional string label for trace hooks)
*/
typedef struct sg_buffer_layout_desc {
    int stride;
    sg_vertex_step step_func;
    int step_rate;
} sg_buffer_layout_desc;

typedef struct sg_vertex_attr_desc {
    int buffer_index;
    int offset;
    sg_vertex_format format;
} sg_vertex_attr_desc;

typedef struct sg_layout_desc {
    sg_buffer_layout_desc buffers[SG_MAX_SHADERSTAGE_BUFFERS];
    sg_vertex_attr_desc attrs[SG_MAX_VERTEX_ATTRIBUTES];
} sg_layout_desc;

typedef struct sg_stencil_state {
    sg_stencil_op fail_op;
    sg_stencil_op depth_fail_op;
    sg_stencil_op pass_op;
    sg_compare_func compare_func;
} sg_stencil_state;

typedef struct sg_depth_stencil_state {
    sg_stencil_state stencil_front;
    sg_stencil_state stencil_back;
    sg_compare_func depth_compare_func;
    bool depth_write_enabled;
    bool stencil_enabled;
    uint8_t stencil_read_mask;
    uint8_t stencil_write_mask;
    uint8_t stencil_ref;
} sg_depth_stencil_state;

typedef struct sg_blend_state {
    bool enabled;
    sg_blend_factor src_factor_rgb;
    sg_blend_factor dst_factor_rgb;
    sg_blend_op op_rgb;
    sg_blend_factor src_factor_alpha;
    sg_blend_factor dst_factor_alpha;
    sg_blend_op op_alpha;
    uint8_t color_write_mask;
    int color_attachment_count;
    sg_pixel_format color_format;
    sg_pixel_format depth_format;
    float blend_color[4];
} sg_blend_state;

typedef struct sg_rasterizer_state {
    bool alpha_to_coverage_enabled;
    sg_cull_mode cull_mode;
    sg_face_winding face_winding;
    int sample_count;
    float depth_bias;
    float depth_bias_slope_scale;
    float depth_bias_clamp;
} sg_rasterizer_state;

typedef struct sg_pipeline_desc {
    uint32_t _start_canary;
    sg_layout_desc layout;
    sg_shader shader;
    sg_primitive_type primitive_type;
    sg_index_type index_type;
    sg_depth_stencil_state depth_stencil;
    sg_blend_state blend;
    sg_rasterizer_state rasterizer;
    const char* label;
    uint32_t _end_canary;
} sg_pipeline_desc;

/*
    sg_pass_desc

    Creation parameters for an sg_pass object, used as argument
    to the sg_make_pass() function.

    A pass object contains 1..4 color-attachments and none, or one,
    depth-stencil-attachment. Each attachment consists of
    an image, and two additional indices describing
    which subimage the pass will render: one mipmap index, and
    if the image is a cubemap, array-texture or 3D-texture, the
    face-index, array-layer or depth-slice.

    Pass images must fulfill the following requirements:

    All images must have:
    - been created as render target (sg_image_desc.render_target = true)
    - the same size
    - the same sample count

    In addition, all color-attachment images must have the same
    pixel format.
*/
typedef struct sg_attachment_desc {
    sg_image image;
    int mip_level;
    union {
        int face;
        int layer;
        int slice;
    };
} sg_attachment_desc;

typedef struct sg_pass_desc {
    uint32_t _start_canary;
    sg_attachment_desc color_attachments[SG_MAX_COLOR_ATTACHMENTS];
    sg_attachment_desc depth_stencil_attachment;
    const char* label;
    uint32_t _end_canary;
} sg_pass_desc;

/*
    sg_trace_hooks

    Installable callback function to keep track of the sokol_gfx calls,
    this is useful for debugging, or keeping track of resource creation
    and destruction.

    Trace hooks are installed with sg_install_trace_hooks(), this returns
    another sg_trace_hooks functions with the previous set of
    trace hook function pointers. These should be invoked by the
    new trace hooks to form a proper call chain.
*/
typedef struct sg_trace_hooks {
    void* user_data;
    void (*reset_state_cache)(void* user_data);
    void (*make_buffer)(const sg_buffer_desc* desc, sg_buffer result, void* user_data);
    void (*make_image)(const sg_image_desc* desc, sg_image result, void* user_data);
    void (*make_shader)(const sg_shader_desc* desc, sg_shader result, void* user_data);
    void (*make_pipeline)(const sg_pipeline_desc* desc, sg_pipeline result, void* user_data);
    void (*make_pass)(const sg_pass_desc* desc, sg_pass result, void* user_data);
    void (*destroy_buffer)(sg_buffer buf, void* user_data);
    void (*destroy_image)(sg_image img, void* user_data);
    void (*destroy_shader)(sg_shader shd, void* user_data);
    void (*destroy_pipeline)(sg_pipeline pip, void* user_data);
    void (*destroy_pass)(sg_pass pass, void* user_data);
    void (*update_buffer)(sg_buffer buf, const void* data_ptr, int data_size, void* user_data);
    void (*update_image)(sg_image img, const sg_image_content* data, void* user_data);
    void (*append_buffer)(sg_buffer buf, const void* data_ptr, int data_size, int result, void* user_data);
    void (*begin_default_pass)(const sg_pass_action* pass_action, int width, int height, void* user_data);
    void (*begin_pass)(sg_pass pass, const sg_pass_action* pass_action, void* user_data);
    void (*apply_viewport)(int x, int y, int width, int height, bool origin_top_left, void* user_data);
    void (*apply_scissor_rect)(int x, int y, int width, int height, bool origin_top_left, void* user_data);
    void (*apply_pipeline)(sg_pipeline pip, void* user_data);
    void (*apply_bindings)(const sg_bindings* bindings, void* user_data);
    void (*apply_uniforms)(sg_shader_stage stage, int ub_index, const void* data, int num_bytes, void* user_data);
    void (*draw)(int base_element, int num_elements, int num_instances, void* user_data);
    void (*end_pass)(void* user_data);
    void (*commit)(void* user_data);
    void (*alloc_buffer)(sg_buffer result, void* user_data);
    void (*alloc_image)(sg_image result, void* user_data);
    void (*alloc_shader)(sg_shader result, void* user_data);
    void (*alloc_pipeline)(sg_pipeline result, void* user_data);
    void (*alloc_pass)(sg_pass result, void* user_data);
    void (*init_buffer)(sg_buffer buf_id, const sg_buffer_desc* desc, void* user_data);
    void (*init_image)(sg_image img_id, const sg_image_desc* desc, void* user_data);
    void (*init_shader)(sg_shader shd_id, const sg_shader_desc* desc, void* user_data);
    void (*init_pipeline)(sg_pipeline pip_id, const sg_pipeline_desc* desc, void* user_data);
    void (*init_pass)(sg_pass pass_id, const sg_pass_desc* desc, void* user_data);
    void (*fail_buffer)(sg_buffer buf_id, void* user_data);
    void (*fail_image)(sg_image img_id, void* user_data);
    void (*fail_shader)(sg_shader shd_id, void* user_data);
    void (*fail_pipeline)(sg_pipeline pip_id, void* user_data);
    void (*fail_pass)(sg_pass pass_id, void* user_data);
    void (*push_debug_group)(const char* name, void* user_data);
    void (*pop_debug_group)(void* user_data);
    void (*err_buffer_pool_exhausted)(void* user_data);
    void (*err_image_pool_exhausted)(void* user_data);
    void (*err_shader_pool_exhausted)(void* user_data);
    void (*err_pipeline_pool_exhausted)(void* user_data);
    void (*err_pass_pool_exhausted)(void* user_data);
    void (*err_context_mismatch)(void* user_data);
    void (*err_pass_invalid)(void* user_data);
    void (*err_draw_invalid)(void* user_data);
    void (*err_bindings_invalid)(void* user_data);
} sg_trace_hooks;

/*
    sg_buffer_info
    sg_image_info
    sg_shader_info
    sg_pipeline_info
    sg_pass_info

    These structs contain various internal resource attributes which
    might be useful for debug-inspection. Please don't rely on the
    actual content of those structs too much, as they are quite closely
    tied to sokol_gfx.h internals and may change more frequently than
    the other public API elements.

    The *_info structs are used as the return values of the following functions:

    sg_query_buffer_info()
    sg_query_image_info()
    sg_query_shader_info()
    sg_query_pipeline_info()
    sg_query_pass_info()
*/
typedef struct sg_slot_info {
    sg_resource_state state;    /* the current state of this resource slot */
    uint32_t res_id;        /* type-neutral resource if (e.g. sg_buffer.id) */
    uint32_t ctx_id;        /* the context this resource belongs to */
} sg_slot_info;

typedef struct sg_buffer_info {
    sg_slot_info slot;              /* resource pool slot info */
    uint32_t update_frame_index;    /* frame index of last sg_update_buffer() */
    uint32_t append_frame_index;    /* frame index of last sg_append_buffer() */
    int append_pos;                 /* current position in buffer for sg_append_buffer() */
    bool append_overflow;           /* is buffer in overflow state (due to sg_append_buffer) */
    int num_slots;                  /* number of renaming-slots for dynamically updated buffers */
    int active_slot;                /* currently active write-slot for dynamically updated buffers */
} sg_buffer_info;

typedef struct sg_image_info {
    sg_slot_info slot;              /* resource pool slot info */
    uint32_t upd_frame_index;       /* frame index of last sg_update_image() */
    int num_slots;                  /* number of renaming-slots for dynamically updated images */
    int active_slot;                /* currently active write-slot for dynamically updated images */
} sg_image_info;

typedef struct sg_shader_info {
    sg_slot_info slot;              /* resoure pool slot info */
} sg_shader_info;

typedef struct sg_pipeline_info {
    sg_slot_info slot;              /* resource pool slot info */
} sg_pipeline_info;

typedef struct sg_pass_info {
    sg_slot_info slot;              /* resource pool slot info */
    sg_pixelformat_info color_format;
    sg_pixelformat_info depth_format;
} sg_pass_info;

/*
    sg_desc

    The sg_desc struct contains configuration values for sokol_gfx,
    it is used as parameter to the sg_setup() call.

    The default configuration is:

    .buffer_pool_size:      128
    .image_pool_size:       128
    .shader_pool_size:      32
    .pipeline_pool_size:    64
    .pass_pool_size:        16
    .context_pool_size:     16

    GL specific:
    .gl_force_gles2
        if this is true the GL backend will act in "GLES2 fallback mode" even
        when compiled with SOKOL_GLES3, this is useful to fall back
        to traditional WebGL if a browser doesn't support a WebGL2 context

    Metal specific:
        (NOTE: All Objective-C object references are transferred through
        a bridged (const void*) to sokol_gfx, which will use a unretained
        bridged cast (__bridged id<xxx>) to retrieve the Objective-C
        references back. Since the bridge cast is unretained, the caller
        must hold a strong reference to the Objective-C object for the
        duration of the sokol_gfx call!

    .mtl_device
        a pointer to the MTLDevice object
    .mtl_renderpass_descriptor_cb
        a C callback function to obtain the MTLRenderPassDescriptor for the
        current frame when rendering to the default framebuffer, will be called
        in sg_begin_default_pass()
    .mtl_drawable_cb
        a C callback function to obtain a MTLDrawable for the current
        frame when rendering to the default framebuffer, will be called in
        sg_end_pass() of the default pass
    .mtl_global_uniform_buffer_size
        the size of the global uniform buffer in bytes, this must be big
        enough to hold all uniform block updates for a single frame,
        the default value is 4 MByte (4 * 1024 * 1024)
    .mtl_sampler_cache_size
        the number of slots in the sampler cache, the Metal backend
        will share texture samplers with the same state in this
        cache, the default value is 64

    D3D11 specific:
    .d3d11_device
        a pointer to the ID3D11Device object, this must have been created
        before sg_setup() is called
    .d3d11_device_context
        a pointer to the ID3D11DeviceContext object
    .d3d11_render_target_view_cb
        a C callback function to obtain a pointer to the current
        ID3D11RenderTargetView object of the default framebuffer,
        this function will be called in sg_begin_pass() when rendering
        to the default framebuffer
    .d3d11_depth_stencil_view_cb
        a C callback function to obtain a pointer to the current
        ID3D11DepthStencilView object of the default framebuffer,
        this function will be called in sg_begin_pass() when rendering
        to the default framebuffer
*/
typedef struct sg_desc {
    uint32_t _start_canary;
    int buffer_pool_size;
    int image_pool_size;
    int shader_pool_size;
    int pipeline_pool_size;
    int pass_pool_size;
    int context_pool_size;
    /* GL specific */
    bool gl_force_gles2;
    /* Metal-specific */
    const void* mtl_device;
    const void* (*mtl_renderpass_descriptor_cb)(void);
    const void* (*mtl_drawable_cb)(void);
    int mtl_global_uniform_buffer_size;
    int mtl_sampler_cache_size;
    /* D3D11-specific */
    const void* d3d11_device;
    const void* d3d11_device_context;
    const void* (*d3d11_render_target_view_cb)(void);
    const void* (*d3d11_depth_stencil_view_cb)(void);
    uint32_t _end_canary;
} sg_desc;

/* setup and misc functions */
SOKOL_API_DECL void sg_setup(const sg_desc* desc);
SOKOL_API_DECL void sg_shutdown(void);
SOKOL_API_DECL bool sg_isvalid(void);
SOKOL_API_DECL void sg_reset_state_cache(void);
SOKOL_API_DECL sg_trace_hooks sg_install_trace_hooks(const sg_trace_hooks* trace_hooks);
SOKOL_API_DECL void sg_push_debug_group(const char* name);
SOKOL_API_DECL void sg_pop_debug_group(void);

/* resource creation, destruction and updating */
SOKOL_API_DECL sg_buffer sg_make_buffer(const sg_buffer_desc* desc);
SOKOL_API_DECL sg_image sg_make_image(const sg_image_desc* desc);
SOKOL_API_DECL sg_shader sg_make_shader(const sg_shader_desc* desc);
SOKOL_API_DECL sg_pipeline sg_make_pipeline(const sg_pipeline_desc* desc);
SOKOL_API_DECL sg_pass sg_make_pass(const sg_pass_desc* desc);
SOKOL_API_DECL void sg_destroy_buffer(sg_buffer buf);
SOKOL_API_DECL void sg_destroy_image(sg_image img);
SOKOL_API_DECL void sg_destroy_shader(sg_shader shd);
SOKOL_API_DECL void sg_destroy_pipeline(sg_pipeline pip);
SOKOL_API_DECL void sg_destroy_pass(sg_pass pass);
SOKOL_API_DECL void sg_update_buffer(sg_buffer buf, const void* data_ptr, int data_size);
SOKOL_API_DECL void sg_update_image(sg_image img, const sg_image_content* data);
SOKOL_API_DECL int sg_append_buffer(sg_buffer buf, const void* data_ptr, int data_size);
SOKOL_API_DECL bool sg_query_buffer_overflow(sg_buffer buf);

/* rendering functions */
SOKOL_API_DECL void sg_begin_default_pass(const sg_pass_action* pass_action, int width, int height);
SOKOL_API_DECL void sg_begin_pass(sg_pass pass, const sg_pass_action* pass_action);
SOKOL_API_DECL void sg_apply_viewport(int x, int y, int width, int height, bool origin_top_left);
SOKOL_API_DECL void sg_apply_scissor_rect(int x, int y, int width, int height, bool origin_top_left);
SOKOL_API_DECL void sg_apply_pipeline(sg_pipeline pip);
SOKOL_API_DECL void sg_apply_bindings(const sg_bindings* bindings);
SOKOL_API_DECL void sg_apply_uniforms(sg_shader_stage stage, int ub_index, const void* data, int num_bytes);
SOKOL_API_DECL void sg_draw(int base_element, int num_elements, int num_instances);
SOKOL_API_DECL void sg_end_pass(void);
SOKOL_API_DECL void sg_commit(void);

/* getting information */
SOKOL_API_DECL sg_desc sg_query_desc(void);
SOKOL_API_DECL sg_backend sg_query_backend(void);
SOKOL_API_DECL sg_features sg_query_features(void);
SOKOL_API_DECL sg_limits sg_query_limits(void);
SOKOL_API_DECL sg_pixelformat_info sg_query_pixelformat(sg_pixel_format fmt);
/* get current state of a resource (INITIAL, ALLOC, VALID, FAILED, INVALID) */
SOKOL_API_DECL sg_resource_state sg_query_buffer_state(sg_buffer buf);
SOKOL_API_DECL sg_resource_state sg_query_image_state(sg_image img);
SOKOL_API_DECL sg_resource_state sg_query_shader_state(sg_shader shd);
SOKOL_API_DECL sg_resource_state sg_query_pipeline_state(sg_pipeline pip);
SOKOL_API_DECL sg_resource_state sg_query_pass_state(sg_pass pass);
/* get runtime information about a resource */
SOKOL_API_DECL sg_buffer_info sg_query_buffer_info(sg_buffer buf);
SOKOL_API_DECL sg_image_info sg_query_image_info(sg_image img);
SOKOL_API_DECL sg_shader_info sg_query_shader_info(sg_shader shd);
SOKOL_API_DECL sg_pipeline_info sg_query_pipeline_info(sg_pipeline pip);
SOKOL_API_DECL sg_pass_info sg_query_pass_info(sg_pass pass);
/* get resource creation desc struct with their default values replaced */
SOKOL_API_DECL sg_buffer_desc sg_query_buffer_defaults(const sg_buffer_desc* desc);
SOKOL_API_DECL sg_image_desc sg_query_image_defaults(const sg_image_desc* desc);
SOKOL_API_DECL sg_shader_desc sg_query_shader_defaults(const sg_shader_desc* desc);
SOKOL_API_DECL sg_pipeline_desc sg_query_pipeline_defaults(const sg_pipeline_desc* desc);
SOKOL_API_DECL sg_pass_desc sg_query_pass_defaults(const sg_pass_desc* desc);

/* separate resource allocation and initialization (for async setup) */
SOKOL_API_DECL sg_buffer sg_alloc_buffer(void);
SOKOL_API_DECL sg_image sg_alloc_image(void);
SOKOL_API_DECL sg_shader sg_alloc_shader(void);
SOKOL_API_DECL sg_pipeline sg_alloc_pipeline(void);
SOKOL_API_DECL sg_pass sg_alloc_pass(void);
SOKOL_API_DECL void sg_init_buffer(sg_buffer buf_id, const sg_buffer_desc* desc);
SOKOL_API_DECL void sg_init_image(sg_image img_id, const sg_image_desc* desc);
SOKOL_API_DECL void sg_init_shader(sg_shader shd_id, const sg_shader_desc* desc);
SOKOL_API_DECL void sg_init_pipeline(sg_pipeline pip_id, const sg_pipeline_desc* desc);
SOKOL_API_DECL void sg_init_pass(sg_pass pass_id, const sg_pass_desc* desc);
SOKOL_API_DECL void sg_fail_buffer(sg_buffer buf_id);
SOKOL_API_DECL void sg_fail_image(sg_image img_id);
SOKOL_API_DECL void sg_fail_shader(sg_shader shd_id);
SOKOL_API_DECL void sg_fail_pipeline(sg_pipeline pip_id);
SOKOL_API_DECL void sg_fail_pass(sg_pass pass_id);

/* rendering contexts (optional) */
SOKOL_API_DECL sg_context sg_setup_context(void);
SOKOL_API_DECL void sg_activate_context(sg_context ctx_id);
SOKOL_API_DECL void sg_discard_context(sg_context ctx_id);


/* bqq extensions */
typedef struct sg_bqq_subimage_rect {
    int src_x;
    int src_y;
    int dst_x;
    int dst_y;
    int dst_z;
    int width;
    int height;
} sg_bqq_subimage_rect;

typedef struct sg_bqq_subimage_copy_desc {
    sg_image dst_image;
    sg_image src_image;
    const sg_bqq_subimage_rect *rects;
    int num_rects;
    int num_mips;
} sg_bqq_subimage_copy_desc;

SOKOL_API_DECL void sg_bqq_update_subimage(sg_image img, const sg_image_desc *desc, int dst_x, int dst_y);
SOKOL_API_DECL void sg_bqq_copy_subimages(const sg_bqq_subimage_copy_desc *desc);
SOKOL_API_DECL void sg_bqq_generate_mipmaps(sg_image image);

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif // SOKOL_GFX_INCLUDED

