#pragma once

#if defined(TRACY_ENABLE)

#include "ext/tracy/Tracy.hpp"

#define SP_ZONE(name) ZoneScopedN(name)
#define SP_ZONE_FUNC() ZoneScoped

#endif
