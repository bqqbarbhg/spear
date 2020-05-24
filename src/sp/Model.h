#pragma once

#include "Asset.h"
#include "sf/Vector.h"
#include "sf/Array.h"
#include "sf/Matrix.h"
#include "sf/Quaternion.h"
#include "sf/HashMap.h"
#include "ext/sokol/sokol_defs.h"
#include "ext/sp_tools_common.h"
#include "sf/Symbol.h"
#include "sp/Renderer.h"

namespace sp {

constexpr uint32_t MaxBones = 64;

struct MeshBone
{
	uint32_t boneIndex;
	sf::Mat34 meshToBone;
};

struct VertexStream
{
	uint32_t stride = 0;
	uint32_t offset = 0;
	char *cpuData = nullptr;
};

struct Mesh
{
	sf::Symbol materialName;

	sf::SmallArray<VertexStream, 2> streams;
	sf::SmallArray<spmdl_attrib, 8> attribs;
	sf::Array<MeshBone> bones;

	uint32_t numIndices = 0;
	uint32_t numVertices = 0;
	uint32_t indexBufferOffset = 0;
	uint16_t *cpuIndexData16 = nullptr;
	uint32_t *cpuIndexData32 = nullptr;
};

struct BoneTransform
{
	sf::Vec3 translation;
	sf::Quat rotation;
	sf::Vec3 scale;
};

struct Bone
{
	uint32_t parentIx;
	sf::Symbol name;
	sf::Mat34 toRoot;
	BoneTransform bindTransform;
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
	static AssetType SelfType;
	using PropType = ModelProps;

	sf::Array<Mesh> meshes;
	sf::Array<Bone> bones;
	sf::HashMap<sf::Symbol, uint32_t> boneNames;

	sp::Buffer vertexBuffer;
	sp::Buffer indexBuffer;

	sf::Array<char> cpuVertexData;
	sf::Array<char> cpuIndexData;
};

using ModelRef = Ref<Model>;

}
