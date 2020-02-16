#ifndef SOKOL_GL_INCLUDED
/*
    sokol_gl.h -- OpenGL 1.x style rendering on top of sokol_gfx.h

    Project URL: https://github.com/floooh/sokol

    LICENSE
    =======
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
#define SOKOL_GL_INCLUDED (1)
#include <stdint.h>
#include <stdbool.h>

#if !defined(SOKOL_GFX_INCLUDED)
#error "Please include sokol_gfx.h before sokol_gl.h"
#endif

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

/* sokol_gl pipeline handle (created with sgl_make_pipeline()) */
typedef struct sgl_pipeline { uint32_t id; } sgl_pipeline;

/*
    sgl_error_t

    Errors are reset each frame after calling sgl_draw(),
    get the last error code with sgl_error()
*/
typedef enum sgl_error_t {
    SGL_NO_ERROR = 0,
    SGL_ERROR_VERTICES_FULL,
    SGL_ERROR_UNIFORMS_FULL,
    SGL_ERROR_COMMANDS_FULL,
    SGL_ERROR_STACK_OVERFLOW,
    SGL_ERROR_STACK_UNDERFLOW,
} sgl_error_t;

typedef struct sgl_desc_t {
    int max_vertices;       /* size for vertex buffer */
    int max_commands;       /* size of uniform- and command-buffers */
    int pipeline_pool_size; /* size of the internal pipeline pool, default is 64 */
    sg_pixel_format color_format;
    sg_pixel_format depth_format;
    int sample_count;
    sg_face_winding face_winding; /* default front face winding is CCW */
} sgl_desc_t;

/* setup/shutdown/misc */
SOKOL_API_DECL void sgl_setup(const sgl_desc_t* desc);
SOKOL_API_DECL void sgl_shutdown(void);
SOKOL_API_DECL sgl_error_t sgl_error(void);
SOKOL_API_DECL void sgl_defaults(void);
SOKOL_API_DECL float sgl_rad(float deg);
SOKOL_API_DECL float sgl_deg(float rad);

/* create and destroy pipeline objects */
SOKOL_API_DECL sgl_pipeline sgl_make_pipeline(const sg_pipeline_desc* desc);
SOKOL_API_DECL void sgl_destroy_pipeline(sgl_pipeline pip);

/* render state functions */
SOKOL_API_DECL void sgl_viewport(int x, int y, int w, int h, bool origin_top_left);
SOKOL_API_DECL void sgl_scissor_rect(int x, int y, int w, int h, bool origin_top_left);
SOKOL_API_DECL void sgl_enable_texture(void);
SOKOL_API_DECL void sgl_disable_texture(void);
SOKOL_API_DECL void sgl_texture(sg_image img);

/* pipeline stack functions */
SOKOL_API_DECL void sgl_default_pipeline(void);
SOKOL_API_DECL void sgl_load_pipeline(sgl_pipeline pip);
SOKOL_API_DECL void sgl_push_pipeline(void);
SOKOL_API_DECL void sgl_pop_pipeline(void);

/* matrix stack functions */
SOKOL_API_DECL void sgl_matrix_mode_modelview(void);
SOKOL_API_DECL void sgl_matrix_mode_projection(void);
SOKOL_API_DECL void sgl_matrix_mode_texture(void);
SOKOL_API_DECL void sgl_load_identity(void);
SOKOL_API_DECL void sgl_load_matrix(const float m[16]);
SOKOL_API_DECL void sgl_load_transpose_matrix(const float m[16]);
SOKOL_API_DECL void sgl_mult_matrix(const float m[16]);
SOKOL_API_DECL void sgl_mult_transpose_matrix(const float m[16]);
SOKOL_API_DECL void sgl_rotate(float angle_rad, float x, float y, float z);
SOKOL_API_DECL void sgl_scale(float x, float y, float z);
SOKOL_API_DECL void sgl_translate(float x, float y, float z);
SOKOL_API_DECL void sgl_frustum(float l, float r, float b, float t, float n, float f);
SOKOL_API_DECL void sgl_ortho(float l, float r, float b, float t, float n, float f);
SOKOL_API_DECL void sgl_perspective(float fov_y, float aspect, float z_near, float z_far);
SOKOL_API_DECL void sgl_lookat(float eye_x, float eye_y, float eye_z, float center_x, float center_y, float center_z, float up_x, float up_y, float up_z);
SOKOL_API_DECL void sgl_push_matrix(void);
SOKOL_API_DECL void sgl_pop_matrix(void);

/* these functions only set the internal 'current texcoord / color' (valid inside or outside begin/end) */
SOKOL_API_DECL void sgl_t2f(float u, float v);
SOKOL_API_DECL void sgl_c3f(float r, float g, float b);
SOKOL_API_DECL void sgl_c4f(float r, float g, float b, float a);
SOKOL_API_DECL void sgl_c3b(uint8_t r, uint8_t g, uint8_t b);
SOKOL_API_DECL void sgl_c4b(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
SOKOL_API_DECL void sgl_c1i(uint32_t rgba);

/* define primitives, each begin/end is one draw command */
SOKOL_API_DECL void sgl_begin_points(void);
SOKOL_API_DECL void sgl_begin_lines(void);
SOKOL_API_DECL void sgl_begin_line_strip(void);
SOKOL_API_DECL void sgl_begin_triangles(void);
SOKOL_API_DECL void sgl_begin_triangle_strip(void);
SOKOL_API_DECL void sgl_begin_quads(void);
SOKOL_API_DECL void sgl_v2f(float x, float y);
SOKOL_API_DECL void sgl_v3f(float x, float y, float z);
SOKOL_API_DECL void sgl_v2f_t2f(float x, float y, float u, float v);
SOKOL_API_DECL void sgl_v3f_t2f(float x, float y, float z, float u, float v);
SOKOL_API_DECL void sgl_v2f_c3f(float x, float y, float r, float g, float b);
SOKOL_API_DECL void sgl_v2f_c3b(float x, float y, uint8_t r, uint8_t g, uint8_t b);
SOKOL_API_DECL void sgl_v2f_c4f(float x, float y, float r, float g, float b, float a);
SOKOL_API_DECL void sgl_v2f_c4b(float x, float y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
SOKOL_API_DECL void sgl_v2f_c1i(float x, float y, uint32_t rgba);
SOKOL_API_DECL void sgl_v3f_c3f(float x, float y, float z, float r, float g, float b);
SOKOL_API_DECL void sgl_v3f_c3b(float x, float y, float z, uint8_t r, uint8_t g, uint8_t b);
SOKOL_API_DECL void sgl_v3f_c4f(float x, float y, float z, float r, float g, float b, float a);
SOKOL_API_DECL void sgl_v3f_c4b(float x, float y, float z, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
SOKOL_API_DECL void sgl_v3f_c1i(float x, float y, float z, uint32_t rgba);
SOKOL_API_DECL void sgl_v2f_t2f_c3f(float x, float y, float u, float v, float r, float g, float b);
SOKOL_API_DECL void sgl_v2f_t2f_c3b(float x, float y, float u, float v, uint8_t r, uint8_t g, uint8_t b);
SOKOL_API_DECL void sgl_v2f_t2f_c4f(float x, float y, float u, float v, float r, float g, float b, float a);
SOKOL_API_DECL void sgl_v2f_t2f_c4b(float x, float y, float u, float v, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
SOKOL_API_DECL void sgl_v2f_t2f_c1i(float x, float y, float u, float v, uint32_t rgba);
SOKOL_API_DECL void sgl_v3f_t2f_c3f(float x, float y, float z, float u, float v, float r, float g, float b);
SOKOL_API_DECL void sgl_v3f_t2f_c3b(float x, float y, float z, float u, float v, uint8_t r, uint8_t g, uint8_t b);
SOKOL_API_DECL void sgl_v3f_t2f_c4f(float x, float y, float z, float u, float v, float r, float g, float b, float a);
SOKOL_API_DECL void sgl_v3f_t2f_c4b(float x, float y, float z, float u, float v, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
SOKOL_API_DECL void sgl_v3f_t2f_c1i(float x, float y, float z, float u, float v, uint32_t rgba);
SOKOL_API_DECL void sgl_end(void);

/* render everything */
SOKOL_API_DECL void sgl_draw(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* SOKOL_GL_INCLUDED */

