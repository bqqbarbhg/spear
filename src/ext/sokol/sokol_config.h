#pragma once

#include "sf/Platform.h"

#define SOKOL_ASSERT(cond) sf_assert(cond)
#define SOKOL_LOG(msg) sf_debug_log(msg)
#define SOKOL_FETCH_USE_CURL

#if SF_OS_WINDOWS
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
#else
	#define SOKOL_GLCORE33
#endif
