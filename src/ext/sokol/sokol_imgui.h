#ifndef SOKOL_IMGUI_INCLUDED
/*
    sokol_imgui.h -- drop-in Dear ImGui renderer/event-handler for sokol_gfx.h

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
#define SOKOL_IMGUI_INCLUDED (1)
#include <stdint.h>
#include <stdbool.h>

#if !defined(SOKOL_GFX_INCLUDED)
#error "Please include sokol_gfx.h before sokol_imgui.h"
#endif
#if !defined(SOKOL_IMGUI_NO_SOKOL_APP) && !defined(SOKOL_APP_INCLUDED)
#error "Please include sokol_app.h before sokol_imgui.h"
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

typedef struct simgui_desc_t {
    int max_vertices;
    sg_pixel_format color_format;
    sg_pixel_format depth_format;
    int sample_count;
    float dpi_scale;
    const char* ini_filename;
    bool no_default_font;
} simgui_desc_t;

SOKOL_API_DECL void simgui_setup(const simgui_desc_t* desc);
SOKOL_API_DECL void simgui_new_frame(int width, int height, double delta_time);
SOKOL_API_DECL void simgui_render(void);
#if !defined(SOKOL_IMGUI_NO_SOKOL_APP)
SOKOL_API_DECL bool simgui_handle_event(const sapp_event* ev);
#endif
SOKOL_API_DECL void simgui_shutdown(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* SOKOL_IMGUI_INCLUDED */

