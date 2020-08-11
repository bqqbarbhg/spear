#pragma once

#include "Asset.h"

#include "Model.h"

#include "sf/Array.h"

namespace sp {

struct AnimationBone
{
	uint32_t parentIx = ~0u;
	sf::Symbol name;
};

struct Animation : Asset
{
	static AssetType SelfType;
	using PropType = NoAssetProps;

	sf::Array<AnimationBone> bones;
	float duration = 0.0f;

	void generateBoneMapping(Model *model, sf::Slice<uint32_t> dst);
	void evaluate(float time, sf::Slice<const uint32_t> boneMapping, sf::Slice<BoneTransform> transforms);
};

using AnimationRef = Ref<Animation>;

void boneTransformToWorld(Model *model, sf::Slice<sf::Mat34> dst, sf::Slice<const BoneTransform> src, const sf::Mat34 &toWorld);

}
