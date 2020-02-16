#pragma once

#if defined(_WIN32)
	#include "mx_platform_win32.h"
#elif defined(__wasm__)
	#include "mx_platform_singlethreaded.h"
#else
	#include "mx_platform_posix.h"
#endif
