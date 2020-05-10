#pragma once

#include "Asset.h"
#include "sf/Vector.h"
#include "sf/Array.h"
#include "sf/Matrix.h"
#include "sf/Quaternion.h"
#include "sf/HashMap.h"
#include "ext/sokol/sokol_defs.h"
#include "sf/Symbol.h"

namespace sp {

constexpr uint32_t MaxBones = 64;

struct Vertex
{
	sf::Vec3 position;
	sf::Vec3 normal;
	sf::Vec2 uv;
};

struct SkinVertex
{
	sf::Vec3 position;
	sf::Vec3 normal;
	sf::Vec2 uv;
	uint8_t index[4];
	uint8_t weight[4];
};

struct SkinWeights
{
	uint8_t index[4];
	uint8_t weight[4];
};

bool operator==(const Vertex &a, const Vertex &b);
sf_forceinline static bool operator!=(const Vertex &a, const Vertex &b) { return !(a == b); }
uint32_t hash(const Vertex &v);

bool operator==(const SkinVertex &a, const SkinVertex &b);
sf_forceinline static bool operator!=(const SkinVertex &a, const SkinVertex &b) { return !(a == b); }
uint32_t hash(const SkinVertex &v);

struct MeshBone
{
	uint32_t boneIndex;
	sf::Mat34 meshToBone;
};

struct Mesh
{
	sf::Symbol materialName;

	sf::Array<Vertex> vertexData;
	sf::Array<uint16_t> indexData;

	uint32_t numIndices;
	uint32_t numVertices;
	uint32_t vertexBufferOffset;
	uint32_t indexBufferOffset;
};

struct SkinMesh
{
	sf::Symbol materialName;

	sf::Array<Vertex> vertexData;
	sf::Array<uint16_t> indexData;
	sf::Array<MeshBone> bones;

	uint32_t numIndices;
	uint32_t numVertices;
	uint32_t vertexBufferOffset;
	uint32_t indexBufferOffset;
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
	bool ignoreGeometry = false;
	bool ignoreAnimations = false;
	sf::Array<sf::CString> retainBones;

	virtual uint32_t hash() const final;
	virtual bool equal(const AssetProps &rhs) const final;
	virtual void copyTo(AssetProps *rhs) const final;
};

struct AnimationCurve
{
	sf::Symbol boneName;
	sf::Array<float> translationTime;
	sf::Array<sf::Vec3> translationValue;
	sf::Array<float> rotationTime;
	sf::Array<sf::Quat> rotationValue;
	sf::Array<float> scaleTime;
	sf::Array<sf::Vec3> scaleValue;
};

struct Animation
{
	sf::Symbol name;
	sf::Array<AnimationCurve> curves;
};

struct Model : Asset
{
	static AssetType SelfType;
	using PropType = ModelProps;

	sf::Array<Mesh> meshes;
	sf::Array<SkinMesh> skins;
	sf::Array<Bone> bones;
	sf::HashMap<sf::Symbol, uint32_t> boneNames;
	sf::Array<Animation> animations;

	sg_buffer vertexBuffer;
	sg_buffer indexBuffer;

	sg_buffer skinVertexBuffer;
	sg_buffer skinIndexBuffer;
};

using ModelRef = Ref<Model>;

sf::Mat34 boneTransformToMatrix(const BoneTransform &t);

void evaluateAnimation(Model *model, sf::Slice<BoneTransform> dst, const Animation &animation, float time);
void boneTransformToWorld(Model *model, sf::Slice<sf::Mat34> dst, const sf::Slice<BoneTransform> src, const sf::Mat34 &toWorld);

}
