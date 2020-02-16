#pragma once

#include "sf/Platform.h"

#define SOKOL_ASSERT(cond) sf_assert(cond)
#define SOKOL_LOG(msg) sf_debug_log(msg)

#if SF_OS_WINDOWS
	#define SOKOL_D3D11
#elif SF_OS_WASM
	#define SOKOL_GLES2
#else
	#error "TODO"
#endif
