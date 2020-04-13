#include "Model.h"

#include "ContentFile.h"
#include "sf/HashMap.h"

#include "ext/ufbx.h"
#include "ext/sokol/sokol_gfx.h"

namespace sp {

bool operator==(const Vertex &a, const Vertex &b)
{
	return !memcmp(&a, &b, sizeof(a));
}

uint32_t hash(const Vertex &v)
{
	return sf::hashBuffer(&v, sizeof(v));
}

bool operator==(const SkinVertex &a, const SkinVertex &b)
{
	return !memcmp(&a, &b, sizeof(a));
}

uint32_t hash(const SkinVertex &v)
{
	return sf::hashBuffer(&v, sizeof(v));
}

uint32_t ModelProps::hash() const
{
	uint32_t h = 0;
	h = sf::hashCombine(h, sf::hash(cpuData));
	for (const sf::CString &s : retainBones) {
		h = sf::hashCombine(h, sf::hash(s));
	}
	return h;
}

bool ModelProps::equal(const AssetProps &rhs) const
{
	const ModelProps &r = (const ModelProps&)rhs;
	if (cpuData != r.cpuData) return false;
	if (retainBones.size != r.retainBones.size) return false;
	for (uint32_t i = 0; i < retainBones.size; i++) {
		if (retainBones[i] != r.retainBones[i]) return false;
	}
	return true;
}

void ModelProps::copyTo(AssetProps *uninitDst) const
{
	ModelProps *dst = (ModelProps*)uninitDst;
	new (dst) ModelProps();
	dst->cpuData = cpuData;
	dst->retainBones = retainBones;
}

struct ModelImp : Model
{
	virtual void assetStartLoading() final;
	virtual void assetUnload() final;
};

AssetType Model::AssetType = { "Model", sizeof(ModelImp), sizeof(Model::PropType),
	[](Asset *a) { new ((ModelImp*)a) ModelImp(); }
};

sf_forceinline static sf::Vec2 toSF(ufbx_vec2 v) { return sf::Vec2((float)v.x, (float)v.y); }
sf_forceinline static sf::Vec3 toSF(ufbx_vec3 v) { return sf::Vec3((float)v.x, (float)v.y, (float)v.z); }
sf_forceinline static sf::Quat toSFQuat(ufbx_vec4 v) { return sf::Quat((float)v.x, (float)v.y, (float)v.z, (float)v.w); }
sf_forceinline static sf::String toSF(ufbx_string v) { return sf::String(v.data, v.length); }
static sf::Mat34 toSF(const ufbx_matrix &m) {
	sf::Mat34 r = sf::Uninit;
	r.cols[0] = toSF(m.cols[0]);
	r.cols[1] = toSF(m.cols[1]);
	r.cols[2] = toSF(m.cols[2]);
	r.cols[3] = toSF(m.cols[3]);
	return r;
}

template <typename VertT>
struct MeshBuilder
{
	sf::StringBuf materialName;
	sf::HashMap<VertT, uint16_t> map;
	sf::Array<VertT> vertices;
	sf::Array<uint16_t> indices;
	sf::Array<MeshBone> bones;
};

static void addVertex(MeshBuilder<Vertex> &mb, ufbx_mesh &mesh, ufbx_matrix normalMat, uint32_t index)
{
	Vertex vert;

	{
		ufbx_vec3 v = ufbx_get_vertex_vec3(&mesh.vertex_position, index);
		v = ufbx_transform_position(&mesh.node.to_root, v);
		vert.position = toSF(v);
	}

	if (mesh.vertex_normal.data) {
		ufbx_vec3 v = ufbx_get_vertex_vec3(&mesh.vertex_normal, index);
		v = ufbx_transform_direction(&normalMat, v);
		vert.normal = normalize(toSF(v));
	} else {
		vert.normal = sf::Vec3();
	}

	if (mesh.vertex_uv.data) {
		vert.uv = toSF(ufbx_get_vertex_vec2(&mesh.vertex_uv, index));
		vert.uv.y = 1.0f - vert.uv.y;
	} else {
		vert.uv = sf::Vec2();
	}

	auto res = mb.map.insert(vert, (uint16_t)mb.vertices.size);
	mb.indices.push(res.entry.val);
	if (res.inserted) {
		mb.vertices.push(vert);
	}
}

static void addSkinVertex(MeshBuilder<SkinVertex> &mb, ufbx_mesh &mesh, sf::Array<SkinWeights> &weights, uint32_t boneOffset, uint32_t index)
{
	SkinVertex vert;

	{
		vert.position = toSF(ufbx_get_vertex_vec3(&mesh.vertex_position, index));
	}

	if (mesh.vertex_normal.data) {
		vert.normal = normalize(toSF(ufbx_get_vertex_vec3(&mesh.vertex_normal, index)));
	} else {
		vert.normal = sf::Vec3();
	}

	if (mesh.vertex_uv.data) {
		vert.uv = toSF(ufbx_get_vertex_vec2(&mesh.vertex_uv, index));
		vert.uv.y = 1.0f - vert.uv.y;
	} else {
		vert.uv = sf::Vec2();
	}

	SkinWeights w = weights[mesh.vertex_position.indices[index]];
	for (uint32_t i = 0; i < 4; i++) {
		vert.index[i] = (uint8_t)(w.index[i] + boneOffset);
		vert.weight[i] = w.weight[i];
	}

	auto res = mb.map.insert(vert, (uint16_t)mb.vertices.size);
	mb.indices.push(res.entry.val);
	if (res.inserted) {
		mb.vertices.push(vert);
	}
}

struct BoneRef
{
	uint32_t index;
	uint8_t boneOffset;
};

static uint32_t addBone(Model &model, sf::Array<ufbx_node*> &bonePtrs, ufbx_node *node)
{
	uint32_t parentIx = ~0u;
	if (node->parent) {
		parentIx = addBone(model, bonePtrs, node->parent);
	}

	sf_assert(model.bones.size == bonePtrs.size);
	for (uint32_t i = 0; i < bonePtrs.size; i++) {
		if (bonePtrs[i] == node) return i;
	}

	uint32_t boneIndex = model.bones.size;
	bonePtrs.push(node);
	Bone &bone = model.bones.push();
	bone.name = toSF(node->name);
	bone.toRoot = toSF(node->to_root);
	bone.parentIx = parentIx;
	bone.bindTransform.translation = toSF(node->transform.translation);
	bone.bindTransform.rotation = toSFQuat(node->transform.rotation);
	bone.bindTransform.scale = toSF(node->transform.scale);
	model.boneNames[bone.name] = boneIndex;
	return boneIndex;
}

static void loadImp(void *user, const ContentFile &file)
{
	ModelImp *imp = (ModelImp*)user;
	ModelProps *props = (ModelProps*)imp->props;

	ufbx_scene *scene = ufbx_load_memory(file.data, file.size, NULL, NULL);
	if (!scene) {
		imp->assetFailLoading();
		return;
	}

	sf::Array<MeshBuilder<Vertex>> meshBuilders;
	sf::Array<MeshBuilder<SkinVertex>> skinBuilders;

	meshBuilders.resize(scene->materials.size + 1);
	for (uint32_t i = 0; i < scene->materials.size; i++) {
		meshBuilders[i].materialName = toSF(scene->materials.data[i].name);
	}

	sf::Array<SkinWeights> weights;
	sf::Array<ufbx_node*> bonePtrs;

	for (sf::CString name : props->retainBones) {
		ufbx_node *node = ufbx_find_node_len(scene, name.data, name.size);
		if (node) addBone(*imp, bonePtrs, node);
	}

	for (ufbx_mesh &mesh : scene->meshes) {

		if (mesh.skins.size > 0) {
			weights.clear();
			weights.resize(mesh.num_vertices);

			sf::SmallArray<MeshBone, MaxBones> meshBones;

			sf::SmallArray<BoneRef, 64> skinBuilderIndices;
			skinBuilderIndices.resizeUninit(scene->materials.size + 1);
			for (BoneRef &ref : skinBuilderIndices) ref.index = ~0u;

			for (ufbx_skin &skin : mesh.skins) {
				uint8_t meshBoneIndex = (uint8_t)meshBones.size;
				MeshBone &meshBone = meshBones.push();
				uint32_t boneIndex = addBone(*imp, bonePtrs, skin.bone);
				meshBone.boneIndex = boneIndex;
				meshBone.meshToBone = toSF(skin.mesh_to_bind);

				for (size_t i = 0; i < skin.num_weights; i++) {
					uint8_t weight = (uint8_t)(sf::clamp(skin.weights[i], 0.0, 1.0) * 255.0);
					uint8_t index = meshBoneIndex;
					SkinWeights &vw = weights[skin.indices[i]];
					for (size_t j = 0; j < 4; j++) {
						if (vw.weight[j] < weight) {
							sf::impSwap(vw.weight[j], weight);
							sf::impSwap(vw.index[j], index);
						}
					}
				}
			}

			// Normalize weights
			for (SkinWeights &w : weights) {
				if (w.weight[0] == 0) continue;

				double total = 0;
				int32_t residue = 255;
				for (uint8_t ww : w.weight) total += (double)ww;
				for (uint8_t &ww : w.weight) {
					ww = (uint8_t)((double)ww / (double)total * 255.0);
					residue -= ww;
				}

				// Make sure the integer total sums to 255
				sf_assert((int32_t)w.weight[0] + residue <= 255 && (int32_t)w.weight[0] + residue >= 0);
				w.weight[0] += residue;
			}

			// Add vertices
			for (size_t fi = 0; fi < mesh.num_faces; fi++) {
				ufbx_face face = mesh.faces[fi];
				sf::String materialName;
				uint32_t materialIx = (uint32_t)scene->materials.size;
				if (mesh.face_material) {
					materialName = toSF(mesh.materials.data[mesh.face_material[fi]]->name);
					materialIx = (uint32_t)(mesh.materials.data[mesh.face_material[fi]] - scene->materials.data);
				}

				BoneRef &ref = skinBuilderIndices[materialIx];
				if (ref.index == ~0u) {
					for (uint32_t ix = 0; ix < skinBuilders.size; ix++) {
						if (skinBuilders[ix].materialName == materialName && skinBuilders[ix].bones.size + meshBones.size <= MaxBones) {
							ref.index = ix;
							ref.boneOffset = (uint8_t)skinBuilders[ix].bones.size;
							break;
						}
					}

					if (ref.index == ~0u) {
						ref.index = skinBuilders.size;
						MeshBuilder<SkinVertex> &mb = skinBuilders.push();
						mb.materialName = materialName;
						ref.boneOffset = 0;
					}

					MeshBuilder<SkinVertex> &mb = skinBuilders[ref.index];
					mb.bones.push(meshBones);
				}

				MeshBuilder<SkinVertex> &mb = skinBuilders[ref.index];
				for (uint32_t bi = 1; bi + 2 <= face.num_indices; bi++) {
					addSkinVertex(mb, mesh, weights, ref.boneOffset, face.index_begin + 0);
					addSkinVertex(mb, mesh, weights, ref.boneOffset, face.index_begin + bi + 0);
					addSkinVertex(mb, mesh, weights, ref.boneOffset, face.index_begin + bi + 1);
				}

			}

		} else {

			ufbx_matrix normalMat = ufbx_get_normal_matrix(&mesh.node.to_root);

			// Add vertices
			for (size_t fi = 0; fi < mesh.num_faces; fi++) {
				ufbx_face face = mesh.faces[fi];
				uint32_t materialIx = (uint32_t)scene->materials.size;
				if (mesh.face_material) {
					materialIx = (uint32_t)(mesh.materials.data[mesh.face_material[fi]] - scene->materials.data);
				}
				MeshBuilder<Vertex> &mb = meshBuilders[materialIx];

				for (uint32_t bi = 1; bi + 2 <= face.num_indices; bi++) {
					addVertex(mb, mesh, normalMat, face.index_begin + 0);
					addVertex(mb, mesh, normalMat, face.index_begin + bi + 0);
					addVertex(mb, mesh, normalMat, face.index_begin + bi + 1);
				}
			}
		}
	}

	weights = sf::Array<SkinWeights>();

	if (props->cpuData) {
		for (MeshBuilder<Vertex> &mb : meshBuilders) {
			if (mb.indices.size == 0) continue;

			Mesh &dst = imp->meshes.push();
			dst.materialName = mb.materialName;
			dst.vertexData = std::move(mb.vertices);
			dst.indexData = std::move(mb.indices);
			dst.numVertices = mb.vertices.size;
			dst.numIndices = mb.indices.size;
		}
	} else {
		sf::Array<Vertex> vertices;
		sf::Array<uint16_t> indices;

		sf::Array<SkinVertex> skinVertices;
		sf::Array<uint16_t> skinIndices;

		for (MeshBuilder<Vertex> &mb : meshBuilders) {
			if (mb.indices.size == 0) continue;

			Mesh &dst = imp->meshes.push();
			dst.materialName = mb.materialName;
			dst.vertexBufferOffset = vertices.size;
			dst.indexBufferOffset = indices.size;
			dst.numVertices = mb.vertices.size;
			dst.numIndices = mb.indices.size;
			vertices.push(mb.vertices);
			indices.push(mb.indices);
		}

		for (MeshBuilder<SkinVertex> &mb : skinBuilders) {
			if (mb.indices.size == 0) continue;

			SkinMesh &dst = imp->skins.push();
			dst.materialName = mb.materialName;
			dst.vertexBufferOffset = skinVertices.size;
			dst.indexBufferOffset = skinIndices.size;
			dst.numVertices = mb.vertices.size;
			dst.numIndices = mb.indices.size;
			dst.bones = std::move(mb.bones);
			skinVertices.push(mb.vertices);
			skinIndices.push(mb.indices);
		}

		if (vertices.size > 0) {
			sf::SmallStringBuf<128> label;
			label.append(imp->name, " vertices");

			sg_buffer_desc desc = { };
			desc.type = SG_BUFFERTYPE_VERTEXBUFFER;
			desc.size = (int)vertices.byteSize();
			desc.content = vertices.data;
			desc.usage = SG_USAGE_IMMUTABLE;
			desc.label = label.data;
			imp->vertexBuffer = sg_make_buffer(&desc);
		}

		if (indices.size > 0) {
			sf::SmallStringBuf<128> label;
			label.append(imp->name, " indices");

			sg_buffer_desc desc = { };
			desc.type = SG_BUFFERTYPE_INDEXBUFFER;
			desc.size = (int)indices.byteSize();
			desc.content = indices.data;
			desc.usage = SG_USAGE_IMMUTABLE;
			desc.label = label.data;
			imp->indexBuffer = sg_make_buffer(&desc);
		}

		if (skinVertices.size > 0) {
			sf::SmallStringBuf<128> label;
			label.append(imp->name, " skinVertices");

			sg_buffer_desc desc = { };
			desc.type = SG_BUFFERTYPE_VERTEXBUFFER;
			desc.size = (int)skinVertices.byteSize();
			desc.content = skinVertices.data;
			desc.usage = SG_USAGE_IMMUTABLE;
			desc.label = label.data;
			imp->skinVertexBuffer = sg_make_buffer(&desc);
		}

		if (skinIndices.size > 0) {
			sf::SmallStringBuf<128> label;
			label.append(imp->name, " skinIndices");

			sg_buffer_desc desc = { };
			desc.type = SG_BUFFERTYPE_INDEXBUFFER;
			desc.size = (int)skinIndices.byteSize();
			desc.content = skinIndices.data;
			desc.usage = SG_USAGE_IMMUTABLE;
			desc.label = label.data;
			imp->skinIndexBuffer = sg_make_buffer(&desc);
		}

	}

	for (ufbx_anim_layer &layer : scene->anim_layers) {
		Animation &anim = imp->animations.push();
		anim.name = toSF(layer.name);

		for (uint32_t boneI = 0; boneI < bonePtrs.size; boneI++) {
			Bone &bone = imp->bones[boneI];
			ufbx_node *node = bonePtrs[boneI];
			if (!ufbx_find_node_anim_prop_begin(scene, &layer, node)) continue;

			AnimationCurve &curve = anim.curves.push();
			curve.boneName = bone.name;

			for (uint32_t timeI = 0; timeI < 100; timeI++) {
				double time = (double)timeI / 24.0;
				ufbx_evaluate_opts opts = { };
				opts.layer = &layer;
				ufbx_transform transform = ufbx_evaluate_transform(scene, node, &opts, time);

				curve.translationTime.push((float)time);
				curve.rotationTime.push((float)time);
				curve.scaleTime.push((float)time);
				curve.translationValue.push(toSF(transform.translation));
				curve.rotationValue.push(toSFQuat(transform.rotation));
				curve.scaleValue.push(toSF(transform.scale));
			}
		}
	}

	ufbx_free_scene(scene);

	imp->assetFinishLoading();
}

void ModelImp::assetStartLoading()
{
	// TODO: Create buffers on main thread
	ContentFile::loadMainThread(name, &loadImp, this);
}

void ModelImp::assetUnload()
{
}

static void findKey(uint32_t &aIx, uint32_t &bIx, float &t, const sf::Array<float> &times, float time)
{
	uint32_t begin = 0;
	uint32_t end = times.size;
	float *ts = times.data;
	while (end - begin >= 16) {
		uint32_t mid = (begin + end) >> 1;
		if (ts[mid] < time) {
			begin = mid + 1;
		} else {
			end = mid;
		}
	}

	end = times.size;
	for (; begin < end; begin++) {
		if (ts[begin] < time) continue;

		bIx = begin;
		if (begin == 0) {
			aIx = begin;
			t = 0.0f;
			return;
		}

		aIx = begin - 1;
		t = (time - ts[begin - 1]) / (ts[begin] - ts[begin - 1]);
		return;
	}

	aIx = end - 1;
	bIx = end - 1;
	t = 0.0f;
}

sf::Mat34 boneTransformToMatrix(const BoneTransform &t)
{
	return sf::mat::world(t.translation, t.rotation, t.scale);
}

void evaluateAnimation(Model *model, sf::Slice<BoneTransform> dst, const Animation &animation, float time)
{
	if (!model) return;

	uint32_t ix = 0;
	for (Bone &bone : model->bones) {
		dst[ix] = bone.bindTransform;
		ix++;
	}

	for (const AnimationCurve &curve : animation.curves) {
		auto pair = model->boneNames.find(curve.boneName);
		if (!pair) continue;
		uint32_t index = pair->val;
		uint32_t a, b;
		float t;

		findKey(a, b, t, curve.translationTime, time);
		dst[index].translation = sf::lerp(curve.translationValue[a], curve.translationValue[b], t);
		findKey(a, b, t, curve.rotationTime, time);
		dst[index].rotation = sf::normalize(sf::lerp(curve.rotationValue[a], curve.rotationValue[b], t));
		findKey(a, b, t, curve.scaleTime, time);
		dst[index].scale = sf::lerp(curve.scaleValue[a], curve.scaleValue[b], t);
	}
}

void boneTransformToWorld(Model *model, sf::Slice<sf::Mat34> dst, const sf::Slice<BoneTransform> src, const sf::Mat34 &toWorld)
{
	if (!model) return;
	uint32_t numBones = model->bones.size;
	if (numBones == 0) return;

	dst[0] = toWorld * boneTransformToMatrix(src[0]);
	for (uint32_t boneI = 1; boneI < numBones; boneI++) {
		uint32_t parentI = model->bones[boneI].parentIx;
		dst[boneI] = dst[parentI] * boneTransformToMatrix(src[boneI]);
	}
}

}
