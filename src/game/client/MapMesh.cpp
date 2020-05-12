#include "MapMesh.h"

namespace cl {

struct MapGeometryBuilder
{
	uint32_t numVertices = 0, numIndices = 0;
	sf::Array<uint16_t> indices16;
	sf::Array<uint32_t> indices32;
	uint16_t *indicesDst16 = nullptr;
	uint32_t *indicesDst32 = nullptr;
	sf::Vec3 aabbMin = sf::Vec3(+HUGE_VALF), aabbMax = sf::Vec3(-HUGE_VALF);

	void count(sp::Model *model) {
		if (!model) return;
		sf_assert(model->isLoaded());
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

void MapChunkGeometry::build(sf::Slice<MapMesh> meshes, const sf::Vec2i &chunkPos)
{
	reset();
	if (meshes.size == 0) return;

	MapGeometryBuilder mainBuilder, shadowBuilder;

	for (MapMesh &mapMesh : meshes) {
		mainBuilder.count(mapMesh.model);
		shadowBuilder.count(mapMesh.shadowModel);
	}

	mainBuilder.finishCount();
	shadowBuilder.finishCount();

	sf::Array<sp::Vertex> vertices;
	sf::Array<sf::Vec3> shadowVertices;
	vertices.resizeUninit(mainBuilder.numVertices);
	shadowVertices.resizeUninit(shadowBuilder.numVertices);
	sp::Vertex *vertexDst = vertices.data;
	sf::Vec3 *shadowVertexDst = shadowVertices.data;

	for (MapMesh &mapMesh : meshes) {
		// TODO: Fix inverse tranpsose
		sf::Mat34 transform = mapMesh.transform;
		sf::Mat33 normalTransform = transform.get33();

		if (mapMesh.model) {
			for (sp::Mesh &mesh : mapMesh.model->meshes) {
				mainBuilder.appendIndices(mesh.indexData, (uint32_t)(vertexDst - vertices.data));
				for (sp::Vertex &vertex : mesh.vertexData) {
					sp::Vertex &dst = *vertexDst++;
					dst.position = sf::transformPoint(transform, vertex.position);
					dst.normal = sf::normalizeOrZero(sf::transformPoint(normalTransform, vertex.normal));
					dst.uv = vertex.uv;
					mainBuilder.updateBounds(dst.position);
				}
			}
		}

		if (mapMesh.shadowModel) {
			for (sp::Mesh &mesh : mapMesh.shadowModel->meshes) {
				shadowBuilder.appendIndices(mesh.indexData, (uint32_t)(shadowVertexDst - shadowVertices.data));
				for (sp::Vertex &vertex : mesh.vertexData) {
					sf::Vec3 &dst = *shadowVertexDst++;
					dst = sf::transformPoint(transform, vertex.position);
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
}

}