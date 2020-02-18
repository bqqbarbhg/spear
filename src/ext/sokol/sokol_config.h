#pragma once

#include "sf/Platform.h"

#define SOKOL_ASSERT(cond) sf_assert(cond)
#define SOKOL_LOG(msg) sf_debug_log(msg)

#if SF_OS_WINDOWS
	#define SOKOL_D3D11
	// #define SOKOL_GLCORE33
#elif SF_OS_WASM
	#ifdef SP_USE_WEBGL2
		#define SOKOL_GLES3
	#else
		#define SOKOL_GLES2
	#endif
#else
	#error "TODO"
#endif
