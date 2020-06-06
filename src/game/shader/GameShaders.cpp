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
#include "TestMesh.h"
#include "TestSkin.h"
#include "ShadowGrid.h"
#include "Line.h"
#include "Sphere.h"

#include "GameShaders.h"

GameShaders gameShaders;

void GameShaders::load()
{
	if (isLoaded) return;
	isLoaded = true;

	mapTile = sg_make_shader(MapTile_MapTile_shader_desc());
	mapShadow = sg_make_shader(MapShadow_MapShadow_shader_desc());
	lightGrid = sg_make_shader(LightGrid_LightGrid_shader_desc());
	postprocess = sg_make_shader(Postprocess_Postprocess_shader_desc());
	fxaa = sg_make_shader(Fxaa_Fxaa_shader_desc());
	upscale = sg_make_shader(Upscale_Upscale_shader_desc());
	upscaleFast = sg_make_shader(Upscale_UpscaleFast_shader_desc());
	testMesh = sg_make_shader(TestMesh_TestMesh_shader_desc());
	skinnedMesh = sg_make_shader(TestSkin_TestSkin_shader_desc());
	shadowGrid = sg_make_shader(ShadowGrid_ShadowGrid_shader_desc());
	line = sg_make_shader(Line_Line_shader_desc());
	sphere = sg_make_shader(Sphere_Sphere_shader_desc());

	{
		sf::Vec2 verts[] = {
			{ 0.0f, 0.0f },
			{ 2.0f, 0.0f },
			{ 0.0f, 2.0f },
		};

		sg_buffer_desc d = { };
		d.type = SG_BUFFERTYPE_VERTEXBUFFER;
		d.content = verts;
		d.size = sizeof(verts);
		d.label = "fullscreenTriangle";
		fullscreenTriangleBuffer = sg_make_buffer(&d);
	}

	for (int largeIndices = 0; largeIndices <= 1; largeIndices++) {
		uint32_t flags = sp::PipeDepthWrite | sp::PipeCullAuto | (largeIndices ? sp::PipeIndex32 : sp::PipeIndex16);
		sg_pipeline_desc &d = mapChunkShadowPipe[largeIndices].init(mapShadow, flags);
		d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
	}

	shadowGridPipe.init(shadowGrid, sp::PipeVertexFloat2);
}
