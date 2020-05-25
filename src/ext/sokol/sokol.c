#include "sokol_config.h"

#pragma warning(disable: 4703)

#define SOKOL_IMPL
#define SOKOL_GL_IMPL

#define SOKOL_FETCH_THREAD_START() sf_set_debug_thread_name("Fetch Thread")

#if !SF_OS_WASM && !SF_OS_APPLE && !defined(SP_NO_APP)
	#include <curl/curl.h>
	#include <curl/easy.h>
	#include <curl/multi.h>
	#define SOKOL_CURL_STATIC
    #define SOKOL_FETCH_USE_CURL
#endif

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_gl.h"
#include "sokol_time.h"
#include "sokol_fetch.h"

#include "sokol_gl_impl.h"
#include "sokol_time_impl.h"

#if !SF_OS_APPLE
    #include "sokol_fetch_impl.h"
    #include "sokol_gfx_impl.h"
	#if !defined(SP_NO_APP)
		#include "sokol_app_impl.h"
	#endif
#endif

