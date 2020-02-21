#ifndef SOKOL_FETCH_INCLUDED
/*
    sokol_fetch.h -- asynchronous data loading/streaming

    Project URL: https://github.com/floooh/sokol

    LICENSE
    =======
    zlib/libpng license

    Copyright (c) 2019 Andre Weissflog

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
#define SOKOL_FETCH_INCLUDED (1)
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

/* configuration values for sfetch_setup() */
typedef struct sfetch_desc_t {
    uint32_t _start_canary;
    uint32_t max_requests;          /* max number of active requests across all channels, default is 128 */
    uint32_t num_channels;          /* number of channels to fetch requests in parallel, default is 1 */
    uint32_t num_lanes;             /* max number of requests active on the same channel, default is 1 */
    uint32_t _end_canary;
} sfetch_desc_t;

/* a request handle to identify an active fetch request, returned by sfetch_send() */
typedef struct sfetch_handle_t { uint32_t id; } sfetch_handle_t;

/* error codes */
typedef enum {
    SFETCH_ERROR_NO_ERROR,
    SFETCH_ERROR_FILE_NOT_FOUND,
    SFETCH_ERROR_NO_BUFFER,
    SFETCH_ERROR_BUFFER_TOO_SMALL,
    SFETCH_ERROR_UNEXPECTED_EOF,
    SFETCH_ERROR_INVALID_HTTP_STATUS,
    SFETCH_ERROR_CANCELLED,
    SFETCH_ERROR_CURL_FAILED,
} sfetch_error_t;

/* the response struct passed to the response callback */
typedef struct sfetch_response_t {
    sfetch_handle_t handle;         /* request handle this response belongs to */
    bool dispatched;                /* true when request is in DISPATCHED state (lane has been assigned) */
    bool fetched;                   /* true when request is in FETCHED state (fetched data is available) */
    bool paused;                    /* request is currently in paused state */
    bool finished;                  /* this is the last response for this request */
    bool failed;                    /* request has failed (always set together with 'finished') */
    bool cancelled;                 /* request was cancelled (always set together with 'finished') */
    sfetch_error_t error_code;      /* more detailed error code when failed is true */
    uint32_t channel;               /* the channel which processes this request */
    uint32_t lane;                  /* the lane this request occupies on its channel */
    const char* path;               /* the original filesystem path of the request (FIXME: this is unsafe, wrap in API call?) */
    void* user_data;                /* pointer to read/write user-data area (FIXME: this is unsafe, wrap in API call?) */
    uint32_t fetched_offset;        /* current offset of fetched data chunk in file data */
    uint32_t fetched_size;          /* size of fetched data chunk in number of bytes */
    void* buffer_ptr;               /* pointer to buffer with fetched data */
    uint32_t buffer_size;           /* overall buffer size (may be >= than fetched_size!) */
} sfetch_response_t;

/* response callback function signature */
typedef void(*sfetch_callback_t)(const sfetch_response_t*);

/* request parameters passed to sfetch_send() */
typedef struct sfetch_request_t {
    uint32_t _start_canary;
    uint32_t channel;               /* index of channel this request is assigned to (default: 0) */
    const char* path;               /* filesystem path or HTTP URL (required) */
    sfetch_callback_t callback;     /* response callback function pointer (required) */
    void* buffer_ptr;               /* buffer pointer where data will be loaded into (optional) */
    uint32_t buffer_size;           /* buffer size in number of bytes (optional) */
    uint32_t chunk_size;            /* number of bytes to load per stream-block (optional) */
    const void* user_data_ptr;      /* pointer to a POD user-data block which will be memcpy'd(!) (optional) */
    uint32_t user_data_size;        /* size of user-data block (optional) */
    uint32_t _end_canary;
} sfetch_request_t;

/* setup sokol-fetch (can be called on multiple threads) */
SOKOL_API_DECL void sfetch_setup(const sfetch_desc_t* desc);
/* discard a sokol-fetch context */
SOKOL_API_DECL void sfetch_shutdown(void);
/* return true if sokol-fetch has been setup */
SOKOL_API_DECL bool sfetch_valid(void);
/* get the desc struct that was passed to sfetch_setup() */
SOKOL_API_DECL sfetch_desc_t sfetch_desc(void);
/* return the max userdata size in number of bytes (SFETCH_MAX_USERDATA_UINT64 * sizeof(uint64_t)) */
SOKOL_API_DECL int sfetch_max_userdata_bytes(void);
/* return the value of the SFETCH_MAX_PATH implementation config value */
SOKOL_API_DECL int sfetch_max_path(void);

/* send a fetch-request, get handle to request back */
SOKOL_API_DECL sfetch_handle_t sfetch_send(const sfetch_request_t* request);
/* return true if a handle is valid *and* the request is alive */
SOKOL_API_DECL bool sfetch_handle_valid(sfetch_handle_t h);
/* do per-frame work, moves requests into and out of IO threads, and invokes response-callbacks */
SOKOL_API_DECL void sfetch_dowork(void);

/* bind a data buffer to a request (request must not currently have a buffer bound, must be called from response callback */
SOKOL_API_DECL void sfetch_bind_buffer(sfetch_handle_t h, void* buffer_ptr, uint32_t buffer_size);
/* clear the 'buffer binding' of a request, returns previous buffer pointer (can be 0), must be called from response callback */
SOKOL_API_DECL void* sfetch_unbind_buffer(sfetch_handle_t h);
/* cancel a request that's in flight (will call response callback with .cancelled + .finished) */
SOKOL_API_DECL void sfetch_cancel(sfetch_handle_t h);
/* pause a request (will call response callback each frame with .paused) */
SOKOL_API_DECL void sfetch_pause(sfetch_handle_t h);
/* continue a paused request */
SOKOL_API_DECL void sfetch_continue(sfetch_handle_t h);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif // SOKOL_FETCH_INCLUDED

