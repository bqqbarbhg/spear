#pragma once

#include "Asset.h"
#include "sf/Vector.h"
#include "sf/Array.h"
#include "ext/sokol/sokol_defs.h"

namespace sp {

struct Vertex
{
	sf::Vec3 position;
	sf::Vec3 normal;
	sf::Vec2 uv;
};

bool operator==(const Vertex &a, const Vertex &b);
sf_forceinline bool operator!=(const Vertex &a, const Vertex &b) { return !(a == b); }
uint32_t hash(const Vertex &v);

struct Mesh
{
	sf::StringBuf materialName;

	sf::Array<Vertex> vertexData;
	sf::Array<uint16_t> indexData;

	uint32_t numIndices;
	uint32_t numVertices;
	uint32_t vertexBufferOffset;
	uint32_t indexBufferOffset;
};

struct ModelProps : AssetProps
{
	bool cpuData = false;

	virtual uint32_t hash() const final;
	virtual bool equal(const AssetProps &rhs) const final;
	virtual void copyTo(AssetProps *rhs) const final;
};

struct Model : Asset
{
	static AssetType AssetType;
	using PropType = ModelProps;

	sf::Array<Mesh> meshes;

	sg_buffer vertexBuffer;
	sg_buffer indexBuffer;
};

using ModelRef = Ref<Model>;

}
