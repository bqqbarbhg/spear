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
#include "Particle.h"
#include "FakeShadow.h"
#include "DebugMesh.h"
#include "DebugSkinnedMesh.h"
#include "Billboard.h"
#include "MapGBuffer.h"
#include "EnvmapBlend.h"

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
	particle = sg_make_shader(Particle_Particle_shader_desc());
	fakeShadow = sg_make_shader(FakeShadow_FakeShadow_shader_desc());
	debugMesh = sg_make_shader(DebugMesh_DebugMesh_shader_desc());
	debugSkinnedMesh = sg_make_shader(DebugSkinnedMesh_DebugSkinnedMesh_shader_desc());
	billboard = sg_make_shader(Billboard_Billboard_shader_desc());
	mapGBuffer = sg_make_shader(MapGBuffer_MapGBuffer_shader_desc());
	envmapBlend = sg_make_shader(EnvmapBlend_EnvmapBlend_shader_desc());

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

	for (int largeIndices = 0; largeIndices <= 1; largeIndices++) {
		uint32_t flags = sp::PipeDepthWrite | (largeIndices ? sp::PipeIndex32 : sp::PipeIndex16);
		sg_pipeline_desc &d = mapChunkEnvmapPipe[largeIndices].init(mapGBuffer, flags);
		d.blend.color_attachment_count = 2;
		d.rasterizer.cull_mode = SG_CULLMODE_NONE;
		d.rasterizer.face_winding = SG_FACEWINDING_CCW;
		d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
		d.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
		d.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT4;
		d.layout.attrs[3].format = SG_VERTEXFORMAT_FLOAT2;
		d.layout.attrs[4].format = SG_VERTEXFORMAT_UBYTE4N;
	}

	shadowGridPipe.init(shadowGrid, sp::PipeVertexFloat2);

	{
		sg_pipeline_desc &d = fakeShadowPipe.init(fakeShadow, sp::PipeBlendOver | sp::PipeIndex16 | sp::PipeDepthTest);
		d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
		d.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
	}

	{
		sg_pipeline_desc &d = debugMeshPipe.init(debugMesh, sp::PipeIndex16 | sp::PipeDepthWrite);
		d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
		d.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
		d.layout.buffers[0].stride = 12 * sizeof(float);
	}

	{
		sg_pipeline_desc &d = debugSkinnedMeshPipe.init(debugSkinnedMesh, sp::PipeIndex16 | sp::PipeDepthWrite);
		d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
		d.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;
		d.layout.attrs[2].format = SG_VERTEXFORMAT_SHORT4N;
		d.layout.attrs[3].format = SG_VERTEXFORMAT_SHORT4N;
		d.layout.attrs[4].format = SG_VERTEXFORMAT_UBYTE4;
		d.layout.attrs[5].format = SG_VERTEXFORMAT_UBYTE4N;
	}

	{
		sg_pipeline_desc &d = billboardPipe.init(billboard, sp::PipeIndex16 | sp::PipeDepthTest | sp::PipeBlendPremultiply);
		d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
		d.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;
		d.layout.attrs[2].format = SG_VERTEXFORMAT_UBYTE4N;
	}
}
