#pragma once

#include "sf/Base.h"
#include "sf/Symbol.h"
#include "sp/Model.h"
#include "sp/Animation.h"
#include "sp/Sprite.h"
#include "sp/Asset.h"
#include "TileMaterial.h"
#include "MeshMaterial.h"

namespace sp { struct ContentFile; }

namespace cl {

struct AnimationInfo
{
	sp::AnimationRef animationRef;

	sf::Symbol name;
	sf::Array<sf::Symbol> tags;
	sf::Symbol file;

	void refresh();
};

struct ModelInfo
{
	sp::ModelRef modelRef;
	sf::HashMap<sf::Symbol, MeshMaterialRef> materialRefs;

	sf::Symbol model;
	sf::HashMap<sf::Symbol, sf::Symbol> materials;
	float scale = 1.0f;
	sf::Array<AnimationInfo> animations;

	void refresh();
};

struct TileVariantInfo
{
	sp::ModelRef modelRef;
	sp::ModelRef shadowModelRef;
	cl::TileMaterialRef materialRef;

	sf::Symbol model;
	sf::Symbol shadowModel;
	sf::Symbol material;

	float scale = 1.0f;
	float probability = 1.0f;

	void refresh();
};

struct TileInfo
{
	sf::Array<TileVariantInfo> variants;
	float scale = 1.0f;
	float totalProbability = 0.0f;

	TileVariantInfo &getVariant(float x);

	void refresh();
};

struct AssetInfoBase : sp::Asset
{
	sf::Type *type;

	AssetInfoBase(sf::Type *type) : type(type) { }

	static void loadCallback(void *user, const sp::ContentFile &file);
	virtual void assetStartLoading() final;
};

template <typename T>
struct AssetInfo : AssetInfoBase
{
	static sp::AssetType SelfType;
	using PropType = sp::NoAssetProps;

	AssetInfo() : AssetInfoBase(sf::typeOf<T>()) { }

	virtual void assetUnload() final {
		data =  T();
	}

	T data;
};

template <typename T>
sp::AssetType AssetInfo<T>::SelfType = {
	"AssetInfo", sizeof(AssetInfo<T>), sizeof(sp::NoAssetProps),
	[](Asset *asset) { new (asset) AssetInfo<T>(); },
};

using AnimationInfoRef = sp::Ref<AssetInfo<AnimationInfo>>;
using ModelInfoRef = sp::Ref<AssetInfo<ModelInfo>>;
using TileInfoRef = sp::Ref<AssetInfo<TileInfo>>;

}
