#pragma once

#include "ext/sokol/sokol_defs.h"
#include "sp/Renderer.h"
#include "sf/Vector.h"

struct GameShaders
{
	sg_shader mapTile;
	sg_shader mapShadow;
	sg_shader lightGrid;
	sg_shader postprocess;
	sg_shader fxaa;
	sg_shader upscale;
	sg_shader upscaleFast;

	void load();
};

extern GameShaders gameShaders;
