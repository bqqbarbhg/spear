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

uint32_t ModelProps::hash() const
{
	uint32_t h = 0;
	h = sf::hashCombine(h, sf::hash(cpuData));
	return h;
}

bool ModelProps::equal(const AssetProps &rhs) const
{
	const ModelProps &r = (const ModelProps&)rhs;
	if (cpuData != r.cpuData) return false;
	return true;
}

void ModelProps::copyTo(AssetProps *uninitDst) const
{
	ModelProps *dst = (ModelProps*)uninitDst;
	new (dst) ModelProps();
	dst->cpuData = cpuData;
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
sf_forceinline static sf::String toSF(ufbx_string v) { return sf::String(v.data, v.length); }

struct MeshBuilder
{
	sf::StringBuf materialName;
	sf::HashMap<Vertex, uint16_t> map;
	sf::Array<Vertex> vertices;
	sf::Array<uint16_t> indices;
};

static void addVertex(MeshBuilder &mb, ufbx_mesh &mesh, uint32_t index)
{
	Vertex vert;

	{
		ufbx_vec3 v = ufbx_get_vertex_vec3(&mesh.vertex_position, index);
		v = ufbx_transform_position(&mesh.node.to_root, v);
		vert.position = toSF(v);
	}

	if (mesh.vertex_normal.data) {
		ufbx_vec3 v = ufbx_get_vertex_vec3(&mesh.vertex_normal, index);
		v = ufbx_transform_normal(&mesh.node.to_root, v);
		vert.normal = toSF(v);
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

static void loadImp(void *user, const ContentFile &file)
{
	ModelImp *imp = (ModelImp*)user;
	ModelProps *props = (ModelProps*)imp->props;

	ufbx_scene *scene = ufbx_load_memory(file.data, file.size, NULL, NULL);
	if (!scene) {
		imp->assetFailLoading();
		return;
	}

	sf::Array<MeshBuilder> builders;

	builders.resize(scene->materials.size + 1);
	for (uint32_t i = 0; i < scene->materials.size; i++) {
		builders[i].materialName = toSF(scene->materials.data[i].name);
	}

	for (ufbx_mesh &mesh : scene->meshes) {
		for (size_t fi = 0; fi < mesh.num_faces; fi++) {
			ufbx_face face = mesh.faces[fi];
			uint32_t material = mesh.face_material ? mesh.face_material[fi] : builders.size - 1;
			auto &mb = builders[material];

			for (uint32_t bi = 1; bi + 2 <= face.num_indices; bi++) {
				addVertex(mb, mesh, face.index_begin + 0);
				addVertex(mb, mesh, face.index_begin + bi + 0);
				addVertex(mb, mesh, face.index_begin + bi + 1);
			}

		}

	}

	if (props->cpuData) {
		for (MeshBuilder &mb : builders) {
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

		for (MeshBuilder &mb : builders) {
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

		{
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

		{
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

	}

	ufbx_free_scene(scene);

	imp->assetFinishLoading();
}

void ModelImp::assetStartLoading()
{
	ContentFile::loadAsync(name, &loadImp, this);
}

void ModelImp::assetUnload()
{
}

}
