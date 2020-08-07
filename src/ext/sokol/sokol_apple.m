#include "sokol_config.h"

#define SOKOL_IMPL

#define SOKOL_MALLOC(s) sf_malloc(s)
#define SOKOL_CALLOC(n, s) sf_calloc(n, s)
#define SOKOL_FREE(p) sf_free(p)

#include "sokol_fetch.h"
#include "sokol_app.h"
#include "sokol_gfx.h"

#if SF_OS_APPLE
	#if !defined(SP_NO_APP)
		#include "sokol_app_impl.h"
	#endif
    #include "sokol_fetch_impl.h"
    #include "sokol_gfx_impl.h"
    #include "sg_ext_metal_timing_impl.h"
#endif

