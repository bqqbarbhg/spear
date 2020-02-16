#pragma once

#include "sf/Platform.h"

#define SOKOL_ASSERT(cond) sf_assert(cond)
#define SOKOL_LOG(msg) sf_debug_log(msg)
#define SOKOL_D3D11
