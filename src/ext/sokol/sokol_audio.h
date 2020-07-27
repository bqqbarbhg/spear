#ifndef SOKOL_AUDIO_INCLUDED
/*
    sokol_audio.h -- cross-platform audio-streaming API

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
#define SOKOL_AUDIO_INCLUDED (1)
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

typedef struct saudio_desc {
    int sample_rate;        /* requested sample rate */
    int num_channels;       /* number of channels, default: 1 (mono) */
    int buffer_frames;      /* number of frames in streaming buffer */
    int packet_frames;      /* number of frames in a packet */
    int num_packets;        /* number of packets in packet queue */
    void (*stream_cb)(float* buffer, int num_frames, int num_channels);  /* optional streaming callback (no user data) */
    void (*stream_userdata_cb)(float* buffer, int num_frames, int num_channels, void* user_data); /*... and with user data */
    void* user_data;        /* optional user data argument for stream_userdata_cb */
} saudio_desc;

/* setup sokol-audio */
SOKOL_API_DECL void saudio_setup(const saudio_desc* desc);
/* shutdown sokol-audio */
SOKOL_API_DECL void saudio_shutdown(void);
/* true after setup if audio backend was successfully initialized */
SOKOL_API_DECL bool saudio_isvalid(void);
/* return the saudio_desc.user_data pointer */
SOKOL_API_DECL void* saudio_userdata(void);
/* return a copy of the original saudio_desc struct */
SOKOL_API_DECL saudio_desc saudio_query_desc(void);
/* actual sample rate */
SOKOL_API_DECL int saudio_sample_rate(void);
/* actual backend buffer size */
SOKOL_API_DECL int saudio_buffer_size(void);
/* actual number of channels */
SOKOL_API_DECL int saudio_channels(void);
/* get current number of frames to fill packet queue */
SOKOL_API_DECL int saudio_expect(void);
/* push sample frames from main thread, returns number of frames actually pushed */
SOKOL_API_DECL int saudio_push(const float* frames, int num_frames);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif // SOKOL_AUDIO_INCLUDED

