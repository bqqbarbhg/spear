#include "MapMesh.h"

#include "sf/Float4.h"

namespace cl {

struct MapGeometryBuilder
{
	uint32_t numVertices = 0, numIndices = 0;
	sf::Array<uint16_t> indices16;
	sf::Array<uint32_t> indices32;
	uint16_t *indicesDst16 = nullptr;
	uint32_t *indicesDst32 = nullptr;
	sf::Vec3 aabbMin = sf::Vec3(+HUGE_VALF), aabbMax = sf::Vec3(-HUGE_VALF);
	bool loading = false;

	void count(sp::Model *model) {
		if (!model) return;
		if (!model->shouldBeLoaded()) {
			if (!model->isFailed()) loading = true;
			return;
		}

		for (sp::Mesh &mesh : model->meshes) {
			numVertices += mesh.numVertices;
			numIndices += mesh.numIndices;
		}
	}

	void finishCount() {
		if (numIndices == 0) return;
		if (numVertices > UINT16_MAX) {
			indices32.resizeUninit(numIndices);
			indicesDst32 = indices32.data;
		} else {
			indices16.resizeUninit(numIndices);
			indicesDst16 = indices16.data;
		}
	}

	void appendIndices(sf::Slice<uint16_t> indices, uint32_t vertexOffset) {
		if (indicesDst16) {
			uint16_t *dst = indicesDst16;
			for (uint16_t index : indices) {
				*dst++ = (uint16_t)(vertexOffset + index);
			}
			indicesDst16 = dst;
		} else {
			uint32_t *dst = indicesDst32;
			for (uint16_t index : indices) {
				*dst++ = vertexOffset + index;
			}
			indicesDst32 = dst;
		}
	}

	sf_forceinline void updateBounds(const sf::Vec3 &pos) {
		aabbMin = sf::min(aabbMin, pos);
		aabbMax = sf::max(aabbMax, pos);
	}

	void finish(const char *indexName, MapGeometry &dst) {
		if (indicesDst16) {
			dst.indexBuffer.initIndex(indexName, indices16.slice());
			dst.largeIndices = false;
		} else if (indicesDst32) {
			dst.indexBuffer.initIndex(indexName, indices32.slice());
			dst.largeIndices = true;
		} else {
			dst.largeIndices = false;
		}
		dst.numInidces = numIndices;
		dst.bounds = sf::Bounds3::minMax(aabbMin, aabbMax);
	}
};

void MapGeometry::reset()
{
	vertexBuffer.reset();
	indexBuffer.reset();
}

void MapChunkGeometry::reset()
{
	main.reset();
	shadow.reset();
}

bool MapChunkGeometry::build(sf::Slice<MapMesh> meshes, const sf::Vec2i &chunkPos)
{
	MapGeometryBuilder mainBuilder, shadowBuilder;
	for (MapMesh &mapMesh : meshes) {
		mainBuilder.count(mapMesh.model);
		shadowBuilder.count(mapMesh.shadowModel);
		if (mapMesh.material.isLoading()) return false;
	}

	if (mainBuilder.loading || shadowBuilder.loading) return false;

	reset();
	if (meshes.size == 0) return true;

	mainBuilder.finishCount();
	shadowBuilder.finishCount();

	sf::Array<MapVertex> vertices;
	sf::Array<sf::Vec3> shadowVertices;
	vertices.resizeUninit(mainBuilder.numVertices);
	shadowVertices.resizeUninit(shadowBuilder.numVertices);
	MapVertex *vertexDst = vertices.data;
	sf::Vec3 *shadowVertexDst = shadowVertices.data;

	for (MapMesh &mapMesh : meshes) {
		// TODO: Fix inverse tranpsose
		sf::Mat34 transform = mapMesh.transform;
		sf::Mat33 tangentTransform = transform.get33();
		sf::Mat33 normalTransform = transform.get33();

		sf::Float4 col0 = sf::Float4::loadu(transform.cols[0].v);
		sf::Float4 col1 = sf::Float4::loadu(transform.cols[1].v);
		sf::Float4 col2 = sf::Float4::loadu(transform.cols[2].v);
		sf::Float4 col3 = sf::Float4::loadu(transform.cols[3].v - 1).rotateLeft();

		const constexpr float roundScale = 1000.0f;
		const constexpr float rcpRoundScale = 1.0f / roundScale;

		if (mapMesh.model.isLoaded() && mapMesh.material.isLoaded()) {
			cl::TileMaterial *material = mapMesh.material;
			for (sp::Mesh &mesh : mapMesh.model->meshes) {
				sf_assert(mesh.streams[0].stride == sizeof(MapSrcVertex));
				mainBuilder.appendIndices(sf::slice(mesh.cpuIndexData16, mesh.numIndices), (uint32_t)(vertexDst - vertices.data));
				for (MapSrcVertex &vertex : sf::slice((MapSrcVertex*)mesh.streams[0].cpuData, mesh.numVertices)) {
					MapVertex &dst = *vertexDst++;
					const sf::Vec3 &vp = vertex.position;
					const sf::Vec3 &vn = vertex.normal;
					const sf::Vec4 &vt = vertex.tangent;

					sf::Float4 tp = col0*vp.x + col1*vp.y + col2*vp.z + col3;
					sf::Float4 tn = col0*vn.x + col1*vn.y + col2*vn.z;
					sf::Float4 tt = col0*vt.x + col1*vt.y + col2*vt.z;

					tp = (tp * roundScale).round() * rcpRoundScale;
					tn *= sf::broadcastRcpLengthXYZ(tn);
					tt *= sf::broadcastRcpLengthXYZ(tt);

					dst.position = tp.asVec3();
					dst.normal = tn.asVec3();
					dst.tangent = sf::Vec4(tt.asVec3(), vertex.tangent.w);
					dst.uv = vertex.uv * material->uvScale + material->uvBase;
					dst.tint = mapMesh.tint;
					mainBuilder.updateBounds(dst.position);
				}
			}
		}

		if (mapMesh.shadowModel.isLoaded()) {
			for (sp::Mesh &mesh : mapMesh.shadowModel->meshes) {
				shadowBuilder.appendIndices(sf::slice(mesh.cpuIndexData16, mesh.numIndices), (uint32_t)(shadowVertexDst - shadowVertices.data));
				for (MapSrcVertex &vertex : sf::slice((MapSrcVertex*)mesh.streams[0].cpuData, mesh.numVertices)) {
					sf::Vec3 &dst = *shadowVertexDst++;

					const sf::Vec3 &vp = vertex.position;

					sf::Float4 tp = col0*vp.x + col1*vp.y + col2*vp.z + col3;

					tp = (tp * roundScale).round() * rcpRoundScale;

					dst = tp.asVec3();
					shadowBuilder.updateBounds(dst);
				}
			}
		}
	}

	sf::SmallStringBuf<256> name;
	name.format("MapChunk(%d,%d) ", chunkPos.x, chunkPos.y);
	uint32_t prefixLen = name.size;

	if (vertices.size) {
		name.resize(prefixLen); name.append(" vertices");
		main.vertexBuffer.initVertex(name.data, vertices.slice());
	}

	name.resize(prefixLen); name.append(" indices");
	mainBuilder.finish(name.data, main);

	if (shadowVertices.size) {
		name.resize(prefixLen); name.append(" shadow vertices");
		shadow.vertexBuffer.initVertex(name.data, shadowVertices.slice());
	}

	name.resize(prefixLen); name.append(" shadow indices");
	shadowBuilder.finish(name.data, shadow);

	return true;
}

}
