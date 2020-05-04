#include "ext/sokol/sokol_gfx.h"
#include "ext/sokol/sokol_config.h"
#include "sf/Vector.h"

#define SOKOL_SHDC_IMPL

#include "TestMesh.h"
#include "TestSkin.h"
#include "MapTile.h"
#include "MapShadow.h"
#include "LightGrid.h"
#include "Postprocess.h"
#include "Fxaa.h"
#include "Upscale.h"

#include "GameShaders.h"

GameShaders gameShaders;

void GameShaders::load()
{
	mapTile = sg_make_shader(MapTile_MapTile_shader_desc());
	mapShadow = sg_make_shader(MapShadow_MapShadow_shader_desc());
	lightGrid = sg_make_shader(LightGrid_LightGrid_shader_desc());
	postprocess = sg_make_shader(Postprocess_Postprocess_shader_desc());
	fxaa = sg_make_shader(Fxaa_Fxaa_shader_desc());
	upscale = sg_make_shader(Upscale_Upscale_shader_desc());
	upscaleFast = sg_make_shader(Upscale_UpscaleFast_shader_desc());
}
