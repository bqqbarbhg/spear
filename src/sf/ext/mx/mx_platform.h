#pragma once

#include "mx_config.h"

#if defined(_WIN32)
	#include "mx_platform_win32.h"
#elif defined(__EMSCRIPTEN_PTHREADS__)
	#include "mx_platform_emscripten.h"
#elif defined(__wasm__)
	#include "mx_platform_singlethreaded.h"
#else
	#include "mx_platform_posix.h"
#endif
