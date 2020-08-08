#include "Model.h"

#include "ContentFile.h"
#include "sf/HashMap.h"

#include "ext/sokol/sokol_gfx.h"

namespace sp {

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
	new (dst) ModelProps(*this);
}

struct ModelImp : Model
{
	virtual void assetStartLoading() final;
	virtual void assetUnload() final;
};

AssetType Model::SelfType = { "Model", sizeof(ModelImp), sizeof(Model::PropType),
	[](Asset *a) { new ((ModelImp*)a) ModelImp(); }
};

static sf::Symbol toSymbol(char *strings, spfile_string str)
{
	return sf::Symbol(strings + str.offset, str.length);
}

static sf::Vec3 toSF(spmdl_vec3 v) { return { v.x, v.y, v.z }; }
static sf::Quat toSFQuat(spmdl_vec4 v) { return { v.x, v.y, v.z, v.w }; }
static sf::Mat34 toSF(const spmdl_matrix &m) {
	sf::Mat34 r = sf::Uninit;
	r.cols[0] = toSF(m.columns[0]);
	r.cols[1] = toSF(m.columns[1]);
	r.cols[2] = toSF(m.columns[2]);
	r.cols[3] = toSF(m.columns[3]);
	return r;
};

static void loadImp(void *user, const ContentFile &file)
{
	ModelImp *imp = (ModelImp*)user;
	ModelProps &props = *(ModelProps*)imp->props;

	spmdl_util su;
	spmdl_util_init(&su, file.data, file.size);
	spmdl_header header = spmdl_decode_header(&su);
	char *strings = spmdl_decode_strings(&su);
	spmdl_node *nodes = spmdl_decode_nodes(&su);
	spmdl_bone *bones = spmdl_decode_bones(&su);
	spmdl_material *materials = spmdl_decode_materials(&su);
	spmdl_mesh *meshes = spmdl_decode_meshes(&su);
	char *vertex, *index;

	if (props.cpuData && !spfile_util_failed(&su.file)) {
		imp->cpuVertexData.resizeUninit(header.s_vertex.uncompressed_size);
		imp->cpuIndexData.resizeUninit(header.s_index.uncompressed_size);
		vertex = imp->cpuVertexData.data;
		index = imp->cpuIndexData.data;
		spmdl_decode_vertex_to(&su, vertex);
		spmdl_decode_index_to(&su, index);
	} else {
		vertex = spmdl_decode_vertex(&su);
		index = spmdl_decode_index(&su);
	}

	if (header.info.num_bvh_nodes > 0) {
		imp->bvhNodes.resizeUninit(header.info.num_bvh_nodes);
		spmdl_decode_bvh_nodes_to(&su, imp->bvhNodes.data);
	}

	if (header.info.num_bvh_tris > 0) {
		imp->bvhTriangles.resizeUninit(header.info.num_bvh_tris * 3);
		spmdl_decode_bvh_tris_to(&su, imp->bvhTriangles.data);
	}

	if (spfile_util_failed(&su.file)) {
		spfile_util_free(&su.file);
		imp->assetFailLoading();
		return;
	}

	imp->bones.reserve(header.info.num_nodes);
	for (uint32_t i = 0; i < header.info.num_nodes; i++) {
		spmdl_node &sp_node = nodes[i];
		Bone &bone = imp->bones.push();
		bone.parentIx = sp_node.parent;
		bone.name = toSymbol(strings, sp_node.name);
		bone.bindTransform.translation = toSF(sp_node.translation);
		bone.bindTransform.rotation = toSFQuat(sp_node.rotation);
		bone.bindTransform.scale = toSF(sp_node.scale);
		bone.toRoot = toSF(sp_node.self_to_root);

		imp->boneNames.insert(bone.name, i);
	}

	imp->meshes.reserve(header.info.num_meshes);
	for (uint32_t i = 0; i < header.info.num_meshes; i++) {
		spmdl_mesh &sp_mesh = meshes[i];
		spmdl_material &sp_material = materials[sp_mesh.material];

		sp::Mesh &mesh = imp->meshes.push();

		mesh.materialName = toSymbol(strings, sp_material.name);
		mesh.numIndices = sp_mesh.num_indices;
		mesh.numVertices = sp_mesh.num_vertices;
		mesh.indexBufferOffset = sp_mesh.index_buffer.offset;
		mesh.bvhRootNodeIndex = sp_mesh.bvh_index;
		mesh.bounds = sf::Bounds3::minMax(toSF(sp_mesh.aabb_min), toSF(sp_mesh.aabb_max));

		mesh.attribs.reserve(sp_mesh.num_attribs);
		for (uint32_t attrI = 0; attrI < sp_mesh.num_attribs; attrI++) {
			if (sp_mesh.attribs[attrI].attrib == SP_VERTEX_ATTRIB_PADDING) continue;
			mesh.attribs.push(sp_mesh.attribs[attrI]);
		}

		mesh.bones.reserve(sp_mesh.num_bones);
		for (uint32_t boneI = 0; boneI < sp_mesh.num_bones; boneI++) {
			spmdl_bone &sp_bone = bones[sp_mesh.bone_offset + boneI];
			sp::MeshBone &bone = mesh.bones.push();
			bone.boneIndex = sp_bone.node;
			bone.meshToBone = toSF(sp_bone.mesh_to_bone);
		}

		mesh.streams.reserve(sp_mesh.num_vertex_buffers);
		for (uint32_t bufI = 0; bufI < sp_mesh.num_vertex_buffers; bufI++) {
			spmdl_buffer &sp_buffer = sp_mesh.vertex_buffers[bufI];
			sp::VertexStream &stream = mesh.streams.push();
			stream.offset = sp_buffer.offset;
			stream.stride = sp_buffer.stride;
			if (props.cpuData) {
				stream.cpuData = vertex + sp_buffer.offset;
			}
		}

		if (props.cpuData) {
			if (sp_mesh.index_buffer.stride == 2) {
				mesh.cpuIndexData16 = (uint16_t*)(index + sp_mesh.index_buffer.offset);
			} else {
				mesh.cpuIndexData32 = (uint32_t*)(index + sp_mesh.index_buffer.offset);
			}
		}
	}

	if (!props.cpuData) {
		{
			sf::SmallStringBuf<256> name;
			name.append(imp->name, " vertexBuffer");
			imp->vertexBuffer.initVertex(name.data, vertex, header.s_vertex.uncompressed_size);
		}

		{
			sf::SmallStringBuf<256> name;
			name.append(imp->name, " indexBuffer");
			imp->indexBuffer.initIndex(name.data, index, header.s_index.uncompressed_size);
		}
	}

	spfile_util_free(&su.file);
	imp->assetFinishLoading();
}

void ModelImp::assetStartLoading()
{
	sf::SmallStringBuf<256> assetName;
	assetName.append(name, ".spmdl");

	// TODO: Create buffers on main thread
	ContentFile::loadMainThread(assetName, &loadImp, this);
}

void ModelImp::assetUnload()
{
}

struct BvhTraverseFrame
{
	spmdl_bvh_split *split;
	float t;
};

sf_inline float intesersectRayAabb(const sf::Vec3 &origin, const sf::Vec3 &rcpDir, const sf::Vec3 &min, const sf::Vec3 &max, float tMin)
{
	sf::Vec3 loT = (min - origin) * rcpDir;
	sf::Vec3 hiT = (max - origin) * rcpDir;
	sf::Vec3 minT = sf::min(loT, hiT);
	sf::Vec3 maxT = sf::max(loT, hiT);
	float t0 = sf::max(minT.x, minT.y, minT.z);
	float t1 = sf::min(maxT.x, maxT.y, maxT.z);
	if (t0 >= t1 || t1 < tMin) return HUGE_VALF;
	return t0;
}

sf_inline sf::Vec3 decodeBvhVertex(const sf::Vec3 &bias, const sf::Vec3 &scale, uint32_t v)
{
	uint32_t ix = v & 0x3ff;
	uint32_t iy = (v >> 10) & 0x3ff;
	uint32_t iz = v >> 20;
	float x = ((float)ix * (1.0f/1023.0f)) * scale.x + bias.x;
	float y = ((float)iy * (1.0f/1023.0f)) * scale.y + bias.y;
	float z = ((float)iz * (1.0f/1023.0f)) * scale.z + bias.z;
	return sf::Vec3(x, y, z);
}

static float intersectTriangles(const uint32_t *tris, const spmdl_bvh_split &split, const sf::Ray &ray, float tMin)
{
	const uint32_t *tri = tris + split.data_index * 3;
	const uint32_t *triEnd = tri + split.num_triangles * 3;

	sf::Vec3 bias = (const sf::Vec3&)split.aabb_min;
	sf::Vec3 scale = (const sf::Vec3&)split.aabb_max - (const sf::Vec3&)split.aabb_min;

	const float eps = 0.0000001f;
	float tHit = HUGE_VALF;

	while (tri != triEnd) {
		sf::Vec3 v0 = decodeBvhVertex(bias, scale, tri[0]);
		sf::Vec3 v1 = decodeBvhVertex(bias, scale, tri[1]);
		sf::Vec3 v2 = decodeBvhVertex(bias, scale, tri[2]);

		sf::Vec3 e1 = v1 - v0;
		sf::Vec3 e2 = v2 - v0;

		sf::Vec3 h = sf::cross(ray.direction, e2);
		float a = sf::dot(e1, h);

		if (a < -eps || a > eps) {
			float f = 1.0f / a;
			sf::Vec3 s = ray.origin - v0;
			float u = f * sf::dot(s, h);
			sf::Vec3 q = sf::cross(s, e1);
			float v = f * sf::dot(ray.direction, q);
			float w = 1.0f - u - v;
			// HACK: Allow areas outside of the triangles to combat
			// potential quantization mismatch issues
			if (sf::min(sf::min(u, v), w) >= -0.001f) {
				float t = f * sf::dot(e2, q);
				if (t >= tMin) {
					tHit = t;
				}
			}
		}

		tri += 3;
	}

	return tHit;
}

float Model::castMeshRay(uint32_t rootNode, const sf::Ray &ray, float tMin) const
{
	sf::SmallArray<BvhTraverseFrame, 64> stack;

	sf::Vec3 origin = ray.origin;
	sf::Vec3 rcpDir = sf::Vec3(1.0f) / ray.direction;

	uint32_t *tris = bvhTriangles.data;
	spmdl_bvh_node *nodes = bvhNodes.data;
	sf_assert(rootNode < bvhNodes.size);

	float tNext = HUGE_VALF;
	float tHit = HUGE_VALF;

	uint32_t nodeIx = rootNode;
	for (;;) {
		spmdl_bvh_node &node = nodes[nodeIx];

		float t0 = intesersectRayAabb(origin, rcpDir, (sf::Vec3&)node.splits[0].aabb_min, (sf::Vec3&)node.splits[0].aabb_max, tMin);
		float t1 = intesersectRayAabb(origin, rcpDir, (sf::Vec3&)node.splits[1].aabb_min, (sf::Vec3&)node.splits[1].aabb_max, tMin);

		float tNear = sf::min(t0, t1);
		float tFar = sf::max(t0, t1);
		int ixNear = t0 != tNear ? 1 : 0;
		int ixFar = ixNear ^ 1;

		spmdl_bvh_split &splitNear = node.splits[ixNear];
		spmdl_bvh_split &splitFar = node.splits[ixFar];

		if (tNear < tHit) {
			if (splitNear.num_triangles >= 0) {
				tHit = sf::min(tHit, intersectTriangles(tris, splitNear, ray, tMin));
				if (tFar < tHit) {
					if (splitFar.num_triangles >= 0) {
						tHit = sf::min(tHit, intersectTriangles(tris, splitFar, ray, tMin));
					} else {
						nodeIx = splitFar.data_index;
						continue;
					}
				}
			} else {
				stack.push({ &splitFar, tFar });
				nodeIx = splitNear.data_index;
				continue;
			}
		}

		for (;;) {
			if (stack.size == 0) return tHit;
			BvhTraverseFrame frame = stack.popValue();
			if (frame.t < tHit) {
				if (frame.split->num_triangles >= 0) {
					tHit = sf::min(tHit, intersectTriangles(tris, *frame.split, ray, tMin));
				} else {
					nodeIx = frame.split->data_index;
					break;
				}
			}
		}
	}
}

float Model::castMeshRay(uint32_t rootNode, const sf::Ray &ray, const sf::Mat34 &transform, float tMin) const
{
	sf::Ray localRay = sf::transformRay(sf::inverse(transform), ray);
	return castMeshRay(rootNode, localRay, tMin);
}

float Model::castModelRay(const sf::Ray &ray, float tMin) const
{
	float tHit = HUGE_VALF;

	sf::Vec3 rcpDir = sf::Vec3(1.0f) / ray.direction;

	for (const Mesh &mesh : meshes) {
		if (mesh.bvhRootNodeIndex == ~0u) continue;

		sf::Vec3 aabbMin = mesh.bounds.origin - mesh.bounds.extent;
		sf::Vec3 aabbMax = mesh.bounds.origin + mesh.bounds.extent;
		float tAabb = intesersectRayAabb(ray.origin, rcpDir, aabbMin, aabbMax, tMin);

		if (tAabb < tHit) {
			tHit = sf::min(tHit, castMeshRay(mesh.bvhRootNodeIndex, ray, tMin));
		}
	}

	return tHit;
}

float Model::castModelRay(const sf::Ray &ray, const sf::Mat34 &transform, float tMin) const
{
	sf::Ray localRay = sf::transformRay(sf::inverse(transform), ray);
	return castModelRay(localRay, tMin);
}

}
