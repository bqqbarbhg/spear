#define _CRT_SECURE_NO_WARNINGS

#include "sp/GameMain.h"

#include "sf/Sort.h"
#include "sf/String.h"
#include "sf/File.h"

#include "sp/Canvas.h"
#include "sp/Sprite.h"
#include "sp/Font.h"
#include "sp/Model.h"
#include "sp/Renderer.h"

#include "ext/sokol/sokol_gfx.h"

#include "game/shader/TestMesh.h"
#include "game/shader/TestSkin.h"
#include "game/shader/Postprocess.h"
#include "game/shader/Fxaa.h"
#include "game/shader/Upscale.h"

#include "sf/Reflection.h"

#include "sp/Json.h"

#include "game/shader/GameShaders.h"

#include "game/server/GameState.h"

#include "State.h"

#include <time.h>

#include "game/Game.h"

#include "ext/bq_websocket.h"
#include "ext/bq_websocket_platform.h"

#include "game/server/Message.h"
#include "sf/Box.h"

#if 0

static void appendUtf8(sf::StringBuf &buf, uint32_t code)
{
	if (code < 0x7f) {
		buf.append((char)code);
	} else if (code <= 0x7ff) {
		buf.append((char)(0xc0 | (code >> 6)));
		buf.append((char)(0x80 | (code >> 0 & 0x3f)));
	} else if (code <= 0xffff) {
		buf.append((char)(0xe0 | (code >> 12)));
		buf.append((char)(0x80 | (code >>  6 & 0x3f)));
		buf.append((char)(0x80 | (code >>  0 & 0x3f)));
	} else if (code <= 0x10ffff) {
		buf.append((char)(0xf0 | (code >> 18)));
		buf.append((char)(0x80 | (code >> 12 & 0x3f)));
		buf.append((char)(0x80 | (code >>  6 & 0x3f)));
		buf.append((char)(0x80 | (code >>  0 & 0x3f)));
	}
}

struct Game
{
	sp::ModelProps charProps;

	sp::Canvas canvas;
	sp::Canvas canvas2;
	sp::Canvas canvas3;
	sp::FontRef font{"sp://OpenSans-Ascii.ttf"};
	sp::FontRef jpFont{"data/kochi-mincho-subst.ttf"};
	sp::ModelRef models[2];
	sp::SpriteRef shirt;
	sp::SpriteRef skin;
	sp::SpriteRef nextSkin{"data/skin1.png"};
	sp::SpriteRef nextShirt{"data/shirt1.png"};
	sp::ModelRef shield{"data/round_shield.fbx"};
	sp::ModelRef sword{"data/sword.fbx"};
	sp::ModelRef staff{"data/staff.fbx"};
	uint32_t jpFrame = 0;
	float skinTimer = 0.0f;
	sf::Vec3 shirtTint;

	sg_pipeline testPipeline[2];
	sg_pipeline skinPipeline[2];

	Game()
	{
		srand(time(NULL));

		charProps.retainBones.push("Item.L");
		charProps.retainBones.push("Item.R");

		models[0].load("data/human.fbx", charProps);
		models[1].load("data/dwarf.fbx", charProps);

		sp::ContentFile::addCacheDownloadRoot("KittenCache",
		[](const sf::CString &name, sf::StringBuf &url, sf::StringBuf &path, void *user) -> bool {
			int w, h;
			if (!sscanf(name.data, "kitten_%d_%d.png", &w, &h)) return false;

			url.format("https://placekitten.com/%d/%d", w, h);
			path.format("cache/kitten_%d_%d.png", w, h);

			return true;
		}, nullptr);

		sp::ContentFile::addRelativeFileRoot("");

		canvas.clear();

		sp::SpriteProps props;
		props.tileX = true;
		props.tileY = true;
		sp::SpriteRef grid{"data/hue.png", props};
		sp::SpriteRef badGrid{"data/hue.png"};
		sp::SpriteRef ghost{"data/ghost.png"};

		canvas.draw(grid, sf::Vec2(0.0f, 0.0f), sf::Vec2(200.0f, 200.0f), sf::Vec4(0.5f));
		canvas.draw(grid, sf::Vec2(200.0f, 0.0f), sf::Vec2(200.0f, 200.0f), sf::Vec4(0.5f));
		canvas.draw(badGrid, sf::Vec2(0.0f, 200.0f), sf::Vec2(200.0f, 200.0f), sf::Vec4(0.5f));
		canvas.draw(badGrid, sf::Vec2(200.0f, 200.0f), sf::Vec2(200.0f, 200.0f), sf::Vec4(0.5f));
		canvas.draw(ghost, sf::Vec2(400.0f, 100.0f), sf::Vec2(400.0f, 400.0f));

		sp::TextDraw td;
		td.font = font;
		td.string = "Typo, WOrld!";
		td.transform.m02 = 100.0f;
		td.transform.m12 = 500.0f;
		td.height = 60.0f;
		canvas.drawText(td);

		{
			sg_pipeline_desc desc = { };
			desc.shader = sg_make_shader(TestMesh_TestMesh_shader_desc());

			desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
			desc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
			desc.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;

			desc.depth_stencil.depth_write_enabled = true;
			desc.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;

			desc.rasterizer.cull_mode = SG_CULLMODE_BACK;
			desc.rasterizer.face_winding = SG_FACEWINDING_CCW;
			desc.rasterizer.alpha_to_coverage_enabled = true;

			desc.index_type = SG_INDEXTYPE_UINT16;

			testPipeline[0] = sg_make_pipeline(&desc);

			desc.rasterizer.depth_bias = 200.0f;
			desc.rasterizer.depth_bias_slope_scale = 2.0f;
			testPipeline[1] = sg_make_pipeline(&desc);
		}

		{
			sg_pipeline_desc desc = { };
			desc.shader = sg_make_shader(TestSkin_TestSkin_shader_desc());

			desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
			desc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
			desc.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
			desc.layout.attrs[3].format = SG_VERTEXFORMAT_UBYTE4;
			desc.layout.attrs[4].format = SG_VERTEXFORMAT_UBYTE4N;

			desc.depth_stencil.depth_write_enabled = true;
			desc.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;

			desc.rasterizer.cull_mode = SG_CULLMODE_BACK;
			desc.rasterizer.face_winding = SG_FACEWINDING_CCW;
			desc.rasterizer.alpha_to_coverage_enabled = true;

			desc.index_type = SG_INDEXTYPE_UINT16;

			skinPipeline[0] = sg_make_pipeline(&desc);

			desc.rasterizer.depth_bias = 200.0f;
			desc.rasterizer.depth_bias_slope_scale = 2.0f;
			skinPipeline[1] = sg_make_pipeline(&desc);
		}


	}

	void debugRenderAtlases()
	{
		sgl_defaults();

		sgl_enable_texture();

		sf::SmallArray<sg_image, 32> images;

		images.push(sp::Font::getFontAtlasImage());

		{
			sf::SmallArray<sp::Atlas*, 32> atlases;
			sp::Atlas::getAtlases(atlases);
			for (sp::Atlas *atlas : atlases) {
				images.push(atlas->image);
			}
		}

		float xs = 0.2f * (float)sapp_height() / (float)sapp_width();
		float ys = -0.2f;
		float y = -1.0f - ys;
		float x = -1.0f;
		int n = 0;
		for (sg_image image : images) {
			sgl_texture(image);

			sgl_begin_quads();
			sgl_v2f_t2f(x, y, 0.0f, 0.0f);
			sgl_v2f_t2f(x+xs, y, 1.0f, 0.0f);
			sgl_v2f_t2f(x+xs, y+ys, 1.0f, 1.0f);
			sgl_v2f_t2f(x, y+ys, 0.0f, 1.0f);
			sgl_end();

			if (++n % 2 == 0) {
				x += xs;
				y = -1.0f - ys;
			} else {
				y -= ys;
			}
		}
	}

	void update(float dt)
	{
		debugRenderAtlases();

		if (jpFont->isLoaded()) {
			jpFrame++;
		}

		skinTimer -= dt;
		if (skinTimer <= 0.0f && nextShirt->isLoaded() && nextSkin->isLoaded()) {
			skinTimer = 1.6f;
			skin = nextSkin;
			shirt = nextShirt;

			sf::StringBuf skinName, shirtName;
			skinName.format("data/skin%u.png", (uint32_t)rand() % 5 + 1);
			shirtName.format("data/shirt%u.png", (uint32_t)rand() % 5 + 1);
			nextSkin = sp::SpriteRef(skinName);
			nextShirt = sp::SpriteRef(shirtName);
			shirtTint.x = ((float)rand() / (float)RAND_MAX) * 0.5f + 0.5f;
			shirtTint.y = ((float)rand() / (float)RAND_MAX) * 0.5f + 0.5f;
			shirtTint.z = ((float)rand() / (float)RAND_MAX) * 0.5f + 0.5f;
		}


		canvas2.clear();
		sf::StringBuf str;
		for (uint32_t n = 0; n < 32; n++) {
			appendUtf8(str, 0x4e00 + (jpFrame + n) % 2000);
		}
		sp::TextDraw td;
		td.font = jpFont;
		td.string = str;
		td.transform.m02 = 100.0f;
		td.transform.m12 = 700.0f;
		td.height = 200.0f;
		canvas2.drawText(td);

		canvas3.clear();

		float time = (float)stm_sec(stm_now()) * 100.0f;
		uint32_t begin = (uint32_t)(time / 400);
		for (uint32_t y = 0; y < 2; y++)
		for (uint32_t x = begin; x < begin + 12; x++) {
			sf::StringBuf url;
			url.format("kitten_%d_%d.png", 200+x, 200+y);
			sp::SpriteRef sprite{url};
			canvas3.draw(sprite, sf::Vec2(x * 400.0f - time, y * 400.0f), sf::Vec2(400.0f, 400.0f));
		}

	}

	void render()
	{
		sg_pass_action pass_action = { };
		pass_action.colors[0].action = SG_ACTION_CLEAR;
#if 1
		pass_action.colors[0].val[0] = (float)0x64 / 255.0f;
		pass_action.colors[0].val[1] = (float)0x95 / 255.0f;
		pass_action.colors[0].val[2] = (float)0xed / 255.0f;
		pass_action.colors[0].val[3] = (float)0xff / 255.0f;
#else
		pass_action.colors[0].val[0] = 0.0f;
		pass_action.colors[0].val[1] = 0.0f;
		pass_action.colors[0].val[2] = 0.0f;
		pass_action.colors[0].val[3] = 0.0f;
#endif

		sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());

		canvas3.render(sp::CanvasRenderOpts::windowPixels());
		canvas.render(sp::CanvasRenderOpts::windowPixels());
		canvas2.render(sp::CanvasRenderOpts::windowPixels());

		sp::CanvasRenderOpts opts = sp::CanvasRenderOpts::windowPixels();
		opts.transform = sf::mat::scale(0.2f) * opts.transform;

		canvas2.render(opts);

		sf::Mat34 world = sf::mat::rotateY(sinf((float)stm_sec(stm_now()*0.07f))*0.3f + 2.7f) * sf::mat::scale(1.0f) * sf::mat::translateY(-2.0f);
		sf::Mat34 look = sf::mat::look(sf::Vec3(0.0f, 0.0f, -10.0f), sf::Vec3(0.0f, 0.0f, 1.0f));

		sf::Mat44 proj;
		if (sg_query_backend() == SG_BACKEND_D3D11) {
			proj = sf::mat::perspectiveD3D(1.2f, (float)sapp_width()/(float)sapp_height(), 0.01f, 100.0f);
		} else {
			proj = sf::mat::perspectiveGL(1.2f, (float)sapp_width()/(float)sapp_height(), 0.01f, 100.0f);
		}

		sf::SmallArray<sf::Mat34, sp::MaxBones> modelBoneWorld[2];

		bool useStaff = fmodf(stm_sec(stm_now()) / 8.0f, 1.0f) > 0.5f;

		int modelIx = 0;
		for (sp::ModelRef model : models) {
			if (!model->isLoaded()) {
				modelIx++;
				continue;
			}

			sf::SmallArray<sp::BoneTransform, sp::MaxBones> boneTransforms;
			sf::SmallArray<sf::Mat34, sp::MaxBones> boneWorld;
			boneTransforms.resizeUninit(model->bones.size);
			boneWorld.resizeUninit(model->bones.size);

			float animTime = fmodf((float)stm_sec(stm_now()), 80.0/24.0);
			sf::CString animName = useStaff ? sf::CString("Armature|StaffIdle") : sf::CString("Armature|ShieldIdle");
			for (sp::Animation &anim : model->animations) {
				if (anim.name == animName) {
					sp::evaluateAnimation(model, boneTransforms, anim, animTime);
					sp::boneTransformToWorld(model, boneWorld, boneTransforms, sf::mat::translateX((modelIx * 2 - 1) * 2.5f) * world);
					break;
				}
			}

			modelBoneWorld[modelIx].push(boneWorld);

			for (sp::Mesh &mesh : model->meshes) {
				if (mesh.materialName == "Prop") continue;

				int pipeIx = mesh.materialName == "Skin" ? 1 : 0;
				sg_apply_pipeline(testPipeline[pipeIx]);

				sf::Mat44 wvp = proj * look * sf::mat::translateX((modelIx * 2 - 1) * 2.5f) * world;

				sp::Sprite *sprite;

				TestMesh_Transform_t transform;
				wvp.writeColMajor44(transform.transform);
				world.writeColMajor44(transform.normalTrasnform);
				if (mesh.materialName == "Skin") {
					transform.color[0] = 1.0f;
					transform.color[1] = 1.0f;
					transform.color[2] = 1.0f;
					sprite = skin;
				} else {
					transform.color[0] = shirtTint.x;
					transform.color[1] = shirtTint.y;
					transform.color[2] = shirtTint.z;
					sprite = shirt;
				}

				if (!sprite || !sprite->isLoaded()) continue;

				sp::Atlas *atlas = sprite->atlas;
				sf::Vec2 atlasSize = sf::Vec2((float)atlas->width, (float)atlas->height);
				sf::Vec2 uvSize = sprite->maxVert - sprite->minVert;
				sf::Vec2 size = sf::Vec2((float)sprite->width, (float)sprite->height);
				sf::Vec2 pos = sf::Vec2((float)sprite->x, (float)sprite->y) / atlasSize;

				sf::Vec2 scale = size / atlasSize / uvSize;
				sf::Vec2 offset = pos - sprite->minVert * scale;

				sf::Vec2 minUv = pos;
				sf::Vec2 maxUv = pos + size;

				transform.texScaleOffset[0] = scale.x;
				transform.texScaleOffset[1] = scale.y;
				transform.texScaleOffset[2] = offset.x;
				transform.texScaleOffset[3] = offset.y;

				TestMesh_FragUniform_t fragUniform;
				fragUniform.texMin[0] = minUv.x;
				fragUniform.texMin[1] = minUv.y;
				fragUniform.texMax[0] = maxUv.x;
				fragUniform.texMax[1] = maxUv.y;

				sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_TestMesh_Transform, &transform, sizeof(transform));
				sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_TestMesh_FragUniform, &fragUniform, sizeof(fragUniform));

				sg_bindings binds = { };
				binds.vertex_buffers[0] = model->vertexBuffer;
				binds.index_buffer = model->indexBuffer;
				binds.index_buffer_offset = mesh.indexBufferOffset * sizeof(uint16_t);
				binds.vertex_buffer_offsets[0] = mesh.vertexBufferOffset * sizeof(sp::Vertex);
				binds.fs_images[0] = atlas->image;
				sg_apply_bindings(&binds);

				sg_draw(0, mesh.numIndices, 1);
			}

			for (sp::SkinMesh &mesh : model->skins) {
				if (mesh.materialName == "Prop") continue;

				int pipeIx = mesh.materialName == "Skin" ? 1 : 0;
				sg_apply_pipeline(skinPipeline[pipeIx]);

				sf::Mat44 wvp = proj * look;

				sp::Sprite *sprite;

				TestSkin_VertexUniform_t vu;
				wvp.writeColMajor44(vu.viewProj);
				if (mesh.materialName == "Skin") {
					vu.color[0] = 1.0f;
					vu.color[1] = 1.0f;
					vu.color[2] = 1.0f;
					sprite = skin;
				} else {
					vu.color[0] = shirtTint.x;
					vu.color[1] = shirtTint.y;
					vu.color[2] = shirtTint.z;
					sprite = shirt;
				}

				if (!sprite || !sprite->isLoaded()) continue;

				sp::Atlas *atlas = sprite->atlas;
				sf::Vec2 atlasSize = sf::Vec2((float)atlas->width, (float)atlas->height);
				sf::Vec2 uvSize = sprite->maxVert - sprite->minVert;
				sf::Vec2 size = sf::Vec2((float)sprite->width, (float)sprite->height);
				sf::Vec2 pos = sf::Vec2((float)sprite->x, (float)sprite->y) / atlasSize;

				sf::Vec2 scale = size / atlasSize / uvSize;
				sf::Vec2 offset = pos - sprite->minVert * scale;

				sf::Vec2 minUv = pos;
				sf::Vec2 maxUv = pos + size;

				vu.texScaleOffset[0] = scale.x;
				vu.texScaleOffset[1] = scale.y;
				vu.texScaleOffset[2] = offset.x;
				vu.texScaleOffset[3] = offset.y;

				TestSkin_Bones_t bones;
				for (uint32_t i = 0; i < mesh.bones.size; i++) {
					sp::MeshBone &meshBone = mesh.bones[i];
					sf::Mat34 transform = boneWorld[meshBone.boneIndex] * meshBone.meshToBone;
					memcpy(bones.bones[i * 3 + 0], transform.getRow(0).v, sizeof(sf::Vec4));
					memcpy(bones.bones[i * 3 + 1], transform.getRow(1).v, sizeof(sf::Vec4));
					memcpy(bones.bones[i * 3 + 2], transform.getRow(2).v, sizeof(sf::Vec4));
				}

				sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_TestSkin_VertexUniform, &vu, sizeof(vu));
				sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_TestSkin_Bones, &bones, sizeof(bones));

				TestMesh_FragUniform_t fragUniform;
				fragUniform.texMin[0] = minUv.x;
				fragUniform.texMin[1] = minUv.y;
				fragUniform.texMax[0] = maxUv.x;
				fragUniform.texMax[1] = maxUv.y;

				sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_TestSkin_FragUniform, &fragUniform, sizeof(fragUniform));

				sg_bindings binds = { };
				binds.vertex_buffers[0] = model->skinVertexBuffer;
				binds.index_buffer = model->skinIndexBuffer;
				binds.index_buffer_offset = mesh.indexBufferOffset * sizeof(uint16_t);
				binds.vertex_buffer_offsets[0] = mesh.vertexBufferOffset * sizeof(sp::SkinVertex);
				binds.fs_images[0] = atlas->image;
				sg_apply_bindings(&binds);

				sg_draw(0, mesh.numIndices, 1);
			}

			modelIx++;
		}

		sf::Array<sp::Model*> items;
		if (useStaff) {
			items.push(staff);
		} else {
			items.push(shield);
			items.push(sword);
		}

		for (uint32_t modelI = 0; modelI < 2; modelI++) {
			for (sp::Model *model : items) {
				if (!model->isLoaded()) {
					continue;
				}

				sf::Mat34 world;
				if (models[modelI]->isLoaded()) {
					const char *bone = model == shield ? "Item.L" : "Item.R";
					auto pair = models[modelI]->boneNames.find(sf::CString(bone));
					if (pair) {
						world = modelBoneWorld[modelI][pair->val];
					}
				}

				for (sp::Mesh &mesh : model->meshes) {
					if (mesh.materialName == "Prop") continue;

					int pipeIx = mesh.materialName == "Shirt" ? 1 : 0;
					sg_apply_pipeline(testPipeline[pipeIx]);

					sf::Mat44 wvp = proj * look * world * sf::mat::rotateX(sf::F_PI*-0.5f);

					sp::Sprite *sprite;

					TestMesh_Transform_t transform;
					wvp.writeColMajor44(transform.transform);
					world.writeColMajor44(transform.normalTrasnform);
					transform.color[0] = 1.0f;
					transform.color[1] = 1.0f;
					transform.color[2] = 1.0f;
					sprite = skin;

					if (!sprite || !sprite->isLoaded()) continue;

					sp::Atlas *atlas = sprite->atlas;
					sf::Vec2 atlasSize = sf::Vec2((float)atlas->width, (float)atlas->height);
					sf::Vec2 uvSize = sprite->maxVert - sprite->minVert;
					sf::Vec2 size = sf::Vec2((float)sprite->width, (float)sprite->height);
					sf::Vec2 pos = sf::Vec2((float)sprite->x, (float)sprite->y) / atlasSize;

					sf::Vec2 scale = size / atlasSize / uvSize;
					sf::Vec2 offset = pos - sprite->minVert * scale;

					sf::Vec2 minUv = pos;
					sf::Vec2 maxUv = pos + size;

					transform.texScaleOffset[0] = scale.x;
					transform.texScaleOffset[1] = scale.y;
					transform.texScaleOffset[2] = offset.x;
					transform.texScaleOffset[3] = offset.y;

					TestMesh_FragUniform_t fragUniform;
					fragUniform.texMin[0] = minUv.x;
					fragUniform.texMin[1] = minUv.y;
					fragUniform.texMax[0] = maxUv.x;
					fragUniform.texMax[1] = maxUv.y;

					sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_TestMesh_Transform, &transform, sizeof(transform));
					sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_TestMesh_FragUniform, &fragUniform, sizeof(fragUniform));

					sg_bindings binds = { };
					binds.vertex_buffers[0] = model->vertexBuffer;
					binds.index_buffer = model->indexBuffer;
					binds.index_buffer_offset = mesh.indexBufferOffset * sizeof(uint16_t);
					binds.vertex_buffer_offsets[0] = mesh.vertexBufferOffset * sizeof(sp::Vertex);
					binds.fs_images[0] = atlas->image;
					sg_apply_bindings(&binds);

					sg_draw(0, mesh.numIndices, 1);
				}
			}
		}

		sgl_draw();

		sg_end_pass();
		sg_commit();
	}
};

Game *game;

struct OtherStruct;

struct TestStruct
{
	sf::Array<sf::Vec2i> arr;
	sf::StringBuf str;
	sf::HashMap<sf::StringBuf, uint32_t> nameMap;
	sf::HashMap<sf::StringBuf, OtherStruct> structMap;
};

struct OtherStruct
{
	TestStruct a;
	TestStruct b;
};

struct Card
{
	sf::StringBuf name;
	sf::StringBuf image;
	sf::StringBuf description;
};

struct AttachItem
{
	sf::StringBuf bone;
	sf::StringBuf model;
};

namespace sf {

template<> void initType<TestStruct>(sf::Type *t)
{
	static Field fields[] = {
		sf_field(TestStruct, arr),
		sf_field(TestStruct, str),
		sf_field(TestStruct, nameMap),
		sf_field(TestStruct, structMap),
	};
	sf_struct(t, TestStruct, fields);
}

template<> void initType<OtherStruct>(sf::Type *t)
{
	static Field fields[] = {
		sf_field(OtherStruct, a),
		sf_field(OtherStruct, b),
	};
	sf_struct(t, OtherStruct, fields);
}

template<> void initType<Card>(sf::Type *t)
{
	static Field fields[] = {
		sf_field(Card, name),
		sf_field(Card, image),
		sf_field(Card, description),
	};
	sf_struct(t, Card, fields);
}

template<> void initType<AttachItem>(sf::Type *t)
{
	static Field fields[] = {
		sf_field(AttachItem, bone),
		sf_field(AttachItem, model),
	};
	sf_struct(t, AttachItem, fields);
}

}

void spConfig(sp::MainConfig &config)
{
	config.sappDesc->window_title = "Spear";
	config.sappDesc->sample_count = 4;

	config.sappDesc->width = 1200;
	config.sappDesc->height = 1080;


	jsi_args args = { };
	args.dialect.allow_bare_keys = true;
	args.dialect.allow_comments = true;
	args.dialect.allow_trailing_comma = true;
	args.dialect.allow_missing_comma = true;
	args.dialect.allow_control_in_string = true;
	args.implicit_root_object = true;
	jsi_value *val = jsi_parse_file("data/testdata.js", &args);


	for (jsi_prop &c : val->object) {
		if (jsi_equal(c.key, "Character")) {
			Character ch;
			sp::readJson(&c.value, ch);
		}
	}

#if 0
	sf::Type *type = sf::typeOf<TestStruct>();

	TestStruct ts1;
	ts1.arr.push(sf::Vec2i(1, 2));
	ts1.arr.push(sf::Vec2i(3, 4));
	ts1.str = "Hello world!";
	ts1.nameMap[sf::String("First")] = 1;
	ts1.nameMap[sf::String("Second")] = 2;
	ts1.structMap[sf::String("Pair")].a.str = "Pair A";
	ts1.structMap[sf::String("Pair")].b.str = "Pair B";

	sf::Array<char> data;
	sf::writeBinary(data, ts1);

	sf::Slice<char> src = data;
	TestStruct ts2;
	bool ret = sf::readBinary(src, ts2);
	sf_assert(src.size == 0);
	sf_assert(ts2.arr[0] == sf::Vec2i(1, 2));
	sf_assert(ts2.arr[1] == sf::Vec2i(3, 4));
	sf_assert(ts2.str == "Hello world!");
	sf_assert(ts2.nameMap.find(sf::String("First"))->val == 1);
	sf_assert(ts2.nameMap.find(sf::String("Second"))->val == 2);

	jso_stream s;
	jso_init_growable(&s);
	s.pretty = true;

	sp::writeJson(s, ts1);

	sf::debugPrint("%.*s\n", (int)s.pos, s.data);

	jsi_value *json = jsi_parse_memory(s.data, s.pos, NULL);

	jso_close(&s);

	TestStruct ts3;
	sp::readJson(json, ts3);
	sf_assert(ts3.arr[0] == sf::Vec2i(1, 2));
	sf_assert(ts3.arr[1] == sf::Vec2i(3, 4));
	sf_assert(ts3.str == "Hello world!");
	sf_assert(ts3.nameMap.find(sf::String("First"))->val == 1);
	sf_assert(ts3.nameMap.find(sf::String("Second"))->val == 2);

	jsi_args args = { };
	args.dialect.allow_bare_keys = true;
	args.dialect.allow_comments = true;
	args.dialect.allow_trailing_comma = true;
	args.dialect.allow_missing_comma = true;
	args.dialect.allow_control_in_string = true;
	args.implicit_root_object = true;
	jsi_value *val = jsi_parse_file("data/card/Sword.js", &args);

	for (jsi_prop &c : val->object) {
		if (jsi_equal(c.key, "Card")) {
			Card card;
			sp::readJson(&c.value, card);
		} else if (jsi_equal(c.key, "AttachItem")) {
			AttachItem attach;
			sp::readJson(&c.value, attach);
		}
	}

	return;
#endif
}

#endif

thread_local Game *t_game;

void spConfig(sp::MainConfig &config)
{
	config.sappDesc.window_title = "Spear";
	config.sappDesc.sample_count = 1;

	config.sappDesc.width = 1200;
	config.sappDesc.height = 1080;

	config.sgDesc.buffer_pool_size = 10*1024;
	config.sgDesc.image_pool_size = 10*1024;
}

sp::RenderTarget mainTarget;
sp::RenderTarget mainDepthTarget;
sp::RenderPass mainPass;

sp::RenderTarget tonemapTarget;
sp::RenderPass tonemapPass;

sp::RenderTarget fxaaTarget;
sp::RenderPass fxaaPass;

sp::Pipeline postPipe;
sp::Pipeline fxaaPipe;
sp::Pipeline upscalePipe;
sp::Pipeline upscaleFastPipe;

int renderRes = 20;
int mainSamples = 1;

void recreateTargets()
{
	int systemWidth = sapp_width();
	int systemHeight = sapp_height();

	float time = (float)stm_sec(stm_now());
	float res = sf::clamp(sqrtf((float)renderRes / 20.0f), 0.1f, 1.0f);
	sf::Vec2i mainRes = sf::Vec2i(sf::Vec2(sf::Vec2i(systemWidth, systemHeight)) * res);

	sg_pixel_format mainFormat = SG_PIXELFORMAT_RG11B10F;
	sg_pixel_format mainDepthFormat = SG_PIXELFORMAT_DEPTH_STENCIL;

	mainTarget.init("mainTarget", mainRes, mainFormat, mainSamples);
	mainDepthTarget.init("mainDepth", mainRes, mainDepthFormat, mainSamples);
	tonemapTarget.init("tonemapTarget", mainRes, SG_PIXELFORMAT_RGBA8);
	fxaaTarget.init("fxaaTarget", mainRes, SG_PIXELFORMAT_RGBA8);

	mainPass.init("main", mainTarget, mainDepthTarget);
	tonemapPass.init("tonemap", tonemapTarget);
	fxaaPass.init("fxaa", fxaaTarget);
}

sp::FontRef font;
sp::Canvas canvas;

bqws_socket *ws;

void spInit()
{
	srand(0);

	{
		sf::typeOf<sv::Message>();
		sf::Array<sf::RcBox<sv::Message>> messages;

		{
			sf::RcBox<sv::MessageAction> msg = sf::rcBox<sv::MessageAction>();
			msg->test = 10;
			messages.push(msg);
		}

		{
			sf::RcBox<sv::MessageActionSuccess> msg = sf::rcBox<sv::MessageActionSuccess>();
			msg->testSuccessFlag = true;
			messages.push(msg);
		}

		{
			sf::RcBox<sv::MessageActionFailure> msg = sf::rcBox<sv::MessageActionFailure>();
			msg->testDescription = "Path blocked";
			messages.push(msg);
		}

		{
			sf::RcBox<sv::MessageUpdate> msg = sf::rcBox<sv::MessageUpdate>();
			msg->testMessages = messages;
			messages.push(msg);
		}

		jso_stream s;
		jso_init_growable(&s);
		s.pretty = true;
		s.pretty_wrap = 70;

		sf::Array<char> binary;
		sf::writeBinary(binary, messages);

		sp::writeJson(s, messages);

		sf::debugPrint("%.*s", (int)s.pos, s.data);

		jsi_value *val = jsi_parse_memory(s.data, (size_t)s.pos, NULL);

		sf::Array<sf::RcBox<sv::Message>> messages2, messages3;
		sp::readJson(val, messages2);
		sf::readBinary(binary.slice(), messages3);

		messages2.clear();
	}

	sv::State state;
	sv::Map &map = state.map;

	{
		sv::TileType &t = map.tileTypes.push();
		t.name = sf::Symbol("None");
	}

	{
		sv::TileType &t = map.tileTypes.push();
		t.name = sf::Symbol("Floor");
		t.floor = true;
	}

	{
		sv::TileType &t = map.tileTypes.push();
		t.name = sf::Symbol("Wall");
		t.wall = true;
	}

	for (uint32_t i = 0; i < 100*100/4; i++) {
		sf::Vec2i pos = { rand() % 100 - 50, rand() % 100 - 50 };
		map.setTile(pos, 1);
	}

	{
		sv::Entity &entity = state.entities.push();
		entity.type = sv::Entity::None;
	}

	{
		sv::Entity &entity = state.entities.push();
		entity.position = { 10, 5 };
		entity.type = sv::Entity::Character;
		entity.index = 0;
	}

	{
		sv::Character &chr = state.characters.push();
		chr.name = sf::Symbol("Player");
	}

	jso_stream s;
	jso_init_growable(&s);
	s.pretty = true;
	s.pretty_wrap = 70;

	sp::writeJson(s, state);

	sf::Array<char> binaryData;
	sf::writeBinary(binaryData, state);
	sf::writeFile("state.bin", binaryData.slice());

	sf::writeFile("state.json", sf::slice(s.data, s.pos));

	gameShaders.load();

	sp::ContentFile::addRelativeFileRoot("");

#if SF_OS_WASM
	sp::ContentFile::addRelativeFileRoot("/");
#endif

	sp::ContentFile::addCacheDownloadRoot("CDN",
	[](const sf::CString &name, sf::StringBuf &url, sf::StringBuf &path, void *user) -> bool {
		if (strncmp(name.data, "data/", 5)) return false;

		url.append("http://51.158.185.160/demo/", name);
		path.append("cache/", name);

		return true;
	}, nullptr);

	ws = bqws_pt_connect("ws://51.158.185.160:4004", NULL, NULL, NULL);
	bqws_send_text(ws, "Hello world!");

	font.load("sp://OpenSans-Ascii.ttf");

	recreateTargets();

	postPipe.init(gameShaders.postprocess, sp::PipeVertexFloat2);
	fxaaPipe.init(gameShaders.fxaa, sp::PipeVertexFloat2);
	upscalePipe.init(gameShaders.upscale, sp::PipeVertexFloat2);
	upscaleFastPipe.init(gameShaders.upscaleFast, sp::PipeVertexFloat2);

	t_game = new Game();
	Game &game = *t_game;

	sf::Symbol tileNames[] = {
		sf::Symbol("data/tile.fbx"),
		sf::Symbol("data/tile.fbx"),
		sf::Symbol("data/tile.fbx"),
		sf::Symbol("data/tile.fbx"),
		sf::Symbol("data/tile.fbx"),
		sf::Symbol("data/tile.fbx"),
		sf::Symbol("data/tile_slanted.fbx"),
		sf::Symbol("data/tile_slanted.fbx"),
		sf::Symbol("data/tile_slanted.fbx"),
		sf::Symbol("data/tile_broken.fbx"),
	};

	srand(1);

	for (int32_t x = -80; x < 80; x++)
	for (int32_t y = -80; y < 80; y++) {

		sf::Vec2i tile { x, y };
		Entity e = { (uint32_t)(x + 100) * 1000 + y };
		game.map.setTile(e, tile);

		MapModel model;
		model.modelName = tileNames[rand() % sf_arraysize(tileNames)];
		model.transform = sf::mat::scale(0.5f) * sf::mat::rotateY((rand() % 4) * sf::F_PI * 0.5f);
		if (rand() % 3 == 0) {
			// model.transform = sf::mat::translateY(((float)rand()/(float)RAND_MAX)*6.0f-3.0f) * model.transform;
			model.transform = sf::mat::translateY(((float)rand()/(float)RAND_MAX)*0.1f-0.05f) * model.transform;
		}
		game.mapRenderer.addMapModel(e, model);
	}

	for (int32_t radius = 4; radius < 80; radius *= 3) {

		for (int32_t x = -radius; x <= radius; x++) {
			if (rand() % 3 == 0) continue;

			int32_t y = -radius;
			sf::Vec2i tile { x, y };
			Entity e = { (uint32_t)(x + 100) * 1000 + y + 1000000 };
			game.map.setTile(e, tile);

			MapModel model;
			model.modelName = sf::Symbol("data/wall.fbx");
			model.shadowModelName = sf::Symbol("data/wall_shadow.fbx");
			model.transform = sf::mat::scale(0.5f) * sf::mat::rotateY(sf::F_2PI * 0.5f);
			game.mapRenderer.addMapModel(e, model);
		}

		for (int32_t x = -radius; x <= radius; x++) {
			if (rand() % 4 < 3) continue;

			int32_t y = radius;
			sf::Vec2i tile { x, y };
			Entity e = { (uint32_t)(x + 100) * 1000 + y + 2000000 };
			game.map.setTile(e, tile);

			MapModel model;
			model.modelName = sf::Symbol("data/wall.fbx");
			model.shadowModelName = sf::Symbol("data/wall_shadow.fbx");
			model.transform = sf::mat::scale(0.5f) * sf::mat::rotateY(sf::F_2PI * 0.0f);
			game.mapRenderer.addMapModel(e, model);
		}

		for (int32_t y = -radius; y <= radius; y++) {
			if (rand() % 3 == 0) continue;

			int32_t x = -radius;
			sf::Vec2i tile { x, y };
			Entity e = { (uint32_t)(x + 100) * 1000 + y + 3000000 };
			game.map.setTile(e, tile);

			MapModel model;
			model.modelName = sf::Symbol("data/wall.fbx");
			model.shadowModelName = sf::Symbol("data/wall_shadow.fbx");
			model.transform = sf::mat::scale(0.5f) * sf::mat::rotateY(sf::F_2PI * 0.75f);
			game.mapRenderer.addMapModel(e, model);
		}

		for (int32_t y = -radius; y <= radius; y++) {
			if (rand() % 5 == 0) continue;

			int32_t x = radius;
			sf::Vec2i tile { x, y };
			Entity e = { (uint32_t)(x + 100) * 1000 + y + 4000000 };
			game.map.setTile(e, tile);

			MapModel model;
			model.modelName = sf::Symbol("data/wall.fbx");
			model.shadowModelName = sf::Symbol("data/wall_shadow.fbx");
			model.transform = sf::mat::scale(0.5f) * sf::mat::rotateY(sf::F_2PI * 0.25f);
			game.mapRenderer.addMapModel(e, model);
		}

	}

}

void spCleanup()
{
	delete t_game;
}

bool fxaa = true;
bool catmullRom = false;

void spEvent(const sapp_event *e)
{
	if (e->type == SAPP_EVENTTYPE_KEY_DOWN) {
		if (e->key_code == SAPP_KEYCODE_UP) {
			if (renderRes < 20) renderRes++;
			recreateTargets();
		} else if (e->key_code == SAPP_KEYCODE_DOWN) {
			if (renderRes > 0) renderRes--;
			recreateTargets();
		} else if (e->key_code == SAPP_KEYCODE_RIGHT) {
			if (mainSamples < 8) mainSamples *= 2;
			recreateTargets();
		} else if (e->key_code == SAPP_KEYCODE_LEFT) {
			if (mainSamples > 1) mainSamples /= 2;
			recreateTargets();
		} else if (e->key_code == SAPP_KEYCODE_F) {
			fxaa = !fxaa;
		} else if (e->key_code == SAPP_KEYCODE_C) {
			catmullRom = !catmullRom;
		}
	}
}

extern sg_buffer g_hackPostVertexBuffer;
double cpuMs;

void spFrame(float dt)
{
	Game &game = *t_game;

	bqws_update(ws);

	sp::beginFrame();

	uint64_t begin = stm_now();

	float anim = (float)sin(stm_sec(stm_now()) * 0.1f)*0.0f;

	float fov = 1.2f;
	float aspect = (float)sapp_width()/(float)sapp_height();
	float near = 0.1f, far = 1000.0f;
	sf::Vec3 cameraPos = sf::Vec3(0.0f, 14.0f + anim * 10.0f, 6.0f);
	game.camera.position = cameraPos;
	game.camera.worldToView = sf::mat::look(cameraPos, sf::Vec3(0.0f, -10.0f - anim * 5.0f, -4.0f));
	if (sg_query_backend() == SG_BACKEND_D3D11) {
		game.camera.viewToClip = sf::mat::perspectiveD3D(fov, aspect, near, far);
	} else {
		game.camera.viewToClip = sf::mat::perspectiveGL(fov, (float)sapp_width()/(float)sapp_height(), near, far);
	}

	game.camera.worldToClip = game.camera.viewToClip * game.camera.worldToView;

	game.mapRenderer.update();

	game.mapRenderer.testRenderLight();

	{
		sg_pass_action action = { };
		action.colors[0].action = SG_ACTION_CLEAR;
		action.colors[0].val[0] = 0.0f;
		action.colors[0].val[1] = 0.0f;
		action.colors[0].val[2] = 0.0f;
		action.colors[0].val[3] = 1.0f;
		sp::beginPass(mainPass, &action);
	}

	game.mapRenderer.render();

	sp::endPass();

	sp::beginPass(tonemapPass, nullptr);

	{
		postPipe.bind();

		sg_bindings bindings = { };
		bindings.fs_images[SLOT_Postprocess_mainImage] = mainTarget.image;
		bindings.vertex_buffers[0] = g_hackPostVertexBuffer;
		sg_apply_bindings(&bindings);

		sg_draw(0, 3, 1);
	}

	sp::endPass();

	if (fxaa) {
		sp::beginPass(fxaaPass, nullptr);

		{
			fxaaPipe.bind();

			sg_bindings bindings = { };
			bindings.fs_images[SLOT_Fxaa_tonemapImage] = tonemapTarget.image;
			bindings.vertex_buffers[0] = g_hackPostVertexBuffer;
			sg_apply_bindings(&bindings);

			Fxaa_Pixel_t pixel;
			pixel.rcpTexSize = sf::Vec2(1.0f) / sf::Vec2(tonemapPass.resolution);
			sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_Fxaa_Pixel, &pixel, sizeof(pixel));

			sg_draw(0, 3, 1);
		}

		sp::endPass();
	}

	sp::beginDefaultPass(sapp_width(), sapp_height(), nullptr);

	if (catmullRom) {
		upscalePipe.bind();

		sg_bindings bindings = { };
		bindings.fs_images[SLOT_Upscale_tonemapImage] = fxaa ? fxaaTarget.image : tonemapTarget.image;
		bindings.vertex_buffers[0] = g_hackPostVertexBuffer;
		sg_apply_bindings(&bindings);

		Upscale_Pixel_t pixel;
		pixel.texSize = sf::Vec2(tonemapPass.resolution);
		pixel.rcpTexSize = sf::Vec2(1.0f) / pixel.texSize;
		sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_Upscale_Pixel, &pixel, sizeof(pixel));

		sg_draw(0, 3, 1);
	} else {
		upscaleFastPipe.bind();

		sg_bindings bindings = { };
		bindings.fs_images[SLOT_Upscale_tonemapImage] = fxaa ? fxaaTarget.image : tonemapTarget.image;
		bindings.vertex_buffers[0] = g_hackPostVertexBuffer;
		sg_apply_bindings(&bindings);

		sg_draw(0, 3, 1);
	}

	canvas.clear();

	float y = 100.0f;
	{
		static float smoothFps;
		if (dt > 1.0f / 1000.0f) smoothFps = sf::lerp(smoothFps, 1.0f / dt, 0.1f);
		sf::SmallStringBuf<128> text;
		text.format("FPS: %.1f", smoothFps);
		sp::TextDraw td;
		td.font = font;
		td.string = text;
		td.transform.m02 = 100.0f;
		td.transform.m12 = y; y += 60.0f;
		td.height = 60.0f;
		canvas.drawText(td);
	}

	{
		sf::SmallStringBuf<128> text;
		text.format("CPU time: %.2fms", cpuMs);
		sp::TextDraw td;
		td.font = font;
		td.string = text;
		td.transform.m02 = 100.0f;
		td.transform.m12 = y; y += 60.0f;
		td.height = 60.0f;
		canvas.drawText(td);
	}

	{
		sf::SmallStringBuf<128> text;
		text.format("Resolution: %d%% (up/down)", renderRes * 100 / 20);
		sp::TextDraw td;
		td.font = font;
		td.string = text;
		td.transform.m02 = 100.0f;
		td.transform.m12 = y; y += 60.0f;
		td.height = 60.0f;
		canvas.drawText(td);
	}

	{
		sf::SmallStringBuf<128> text;
		text.format("MSAA: %d (left/right)", mainSamples);
		sp::TextDraw td;
		td.font = font;
		td.string = text;
		td.transform.m02 = 100.0f;
		td.transform.m12 = y; y += 60.0f;
		td.height = 60.0f;
		canvas.drawText(td);
	}

	{
		sf::SmallStringBuf<128> text;
		text.format("FXAA: %s (F)", fxaa ? "true" : "false");
		sp::TextDraw td;
		td.font = font;
		td.string = text;
		td.transform.m02 = 100.0f;
		td.transform.m12 = y; y += 60.0f;
		td.height = 60.0f;
		canvas.drawText(td);
	}

	{
		sf::SmallStringBuf<128> text;
		text.format("Catmull-Rom: %s (C)", catmullRom ? "true" : "false");
		sp::TextDraw td;
		td.font = font;
		td.string = text;
		td.transform.m02 = 100.0f;
		td.transform.m12 = y; y += 60.0f;
		td.height = 60.0f;
		canvas.drawText(td);
	}

	for (const sp::PassTime &time : sp::getPassTimes()) {
		sf::SmallStringBuf<128> text;
		text.format("%s: %.2fms", time.name.data, time.time * 1000.0);
		sp::TextDraw td;
		td.font = font;
		td.string = text;
		td.transform.m02 = 100.0f;
		td.transform.m12 = y; y += 60.0f;
		td.height = 60.0f;
		canvas.drawText(td);
	}

	canvas.render(sp::CanvasRenderOpts::windowPixels());

	sp::endPass();

	sp::endFrame();

	sg_commit();

	cpuMs = stm_ms(stm_since(begin));
}
