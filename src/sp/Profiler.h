#pragma once

#if defined(TRACY_ENABLE)

#include "ext/tracy/Tracy.hpp"

#define SP_ZONE(name) ZoneScopedN(name)
#define SP_ZONE_FUNC() ZoneScoped

#else

#define SP_ZONE(name) (void)0
#define SP_ZONE_FUNC() (void)0

#endif
