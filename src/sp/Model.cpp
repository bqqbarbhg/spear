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
		sp::Mesh &mesh = imp->meshes.push();

		mesh.numIndices = sp_mesh.num_indices;
		mesh.numVertices = sp_mesh.num_vertices;
		mesh.indexBufferOffset = sp_mesh.index_buffer.offset;

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

}
