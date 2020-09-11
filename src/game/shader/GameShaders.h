#pragma once

#include "ext/sokol/sokol_defs.h"
#include "sp/Renderer.h"
#include "sf/Vector.h"

struct GameShaders
{
	bool isLoaded = false;

	sg_shader mapTile;
	sg_shader mapShadow;
	sg_shader lightGrid;
	sg_shader postprocess;
	sg_shader fxaa;
	sg_shader upscale;
	sg_shader upscaleFast;
	sg_shader testMesh;
	sg_shader skinnedMesh;
	sg_shader shadowGrid;
	sg_shader line;
	sg_shader sphere;
	sg_shader particle;
	sg_shader fakeShadow;
	sg_shader debugMesh;
	sg_shader debugSkinnedMesh;
	sg_shader billboard;
	sg_shader mapGBuffer;
	sg_shader envmapBlend;

	sg_buffer fullscreenTriangleBuffer;

	sp::Pipeline mapChunkShadowPipe[2];
	sp::Pipeline mapChunkEnvmapPipe[2];
	sp::Pipeline shadowGridPipe;
	sp::Pipeline fakeShadowPipe;
	sp::Pipeline debugMeshPipe;
	sp::Pipeline debugSkinnedMeshPipe;
	sp::Pipeline billboardPipe;

	void load();
};

extern GameShaders gameShaders;
