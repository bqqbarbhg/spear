#include "sokol_config.h"

#pragma warning(disable: 4703)

#define SOKOL_IMPL
#define SOKOL_IMGUI_IMPL

#include "ext/imgui/imgui.h"
#include "sokol_gfx.h"
#include "sokol_app.h"

#if !defined(SP_NO_APP)
	#include "sokol_imgui.h"
	#include "sokol_imgui_impl.h"
#endif
