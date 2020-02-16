#ifndef SOKOL_TIME_INCLUDED
/*
    sokol_time.h    -- simple cross-platform time measurement

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
#define SOKOL_TIME_INCLUDED (1)
#include <stdint.h>

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

SOKOL_API_DECL void stm_setup(void);
SOKOL_API_DECL uint64_t stm_now(void);
SOKOL_API_DECL uint64_t stm_diff(uint64_t new_ticks, uint64_t old_ticks);
SOKOL_API_DECL uint64_t stm_since(uint64_t start_ticks);
SOKOL_API_DECL uint64_t stm_laptime(uint64_t* last_time);
SOKOL_API_DECL double stm_sec(uint64_t ticks);
SOKOL_API_DECL double stm_ms(uint64_t ticks);
SOKOL_API_DECL double stm_us(uint64_t ticks);
SOKOL_API_DECL double stm_ns(uint64_t ticks);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif // SOKOL_TIME_INCLUDED

