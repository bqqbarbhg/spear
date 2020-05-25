#pragma once

#include "sf/Platform.h"

#define SOKOL_ASSERT(cond) sf_assert(cond)
#define SOKOL_LOG(msg) sf_debug_log(msg)

#if defined(SP_NO_APP)
	#define SOKOL_DUMMY_BACKEND
#elif SF_OS_WINDOWS
	#if defined(SP_USE_OPENGL)
		#define SOKOL_GLCORE33
	#else
		#define SOKOL_D3D11
	#endif
#elif SF_OS_WASM
	#ifdef SP_USE_WEBGL2
		#define SOKOL_GLES3
	#else
		#define SOKOL_GLES2
	#endif
#elif SF_OS_APPLE
    #define SOKOL_METAL
#else
	#define SOKOL_GLCORE33
#endif
