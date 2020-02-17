#pragma once

#include "Asset.h"
#include "sf/String.h"

namespace sp {

struct Sprite : Asset
{
	static const AssetType AssetType;

	virtual void startLoadingImp() final;
};

using SpriteRef = Ref<Sprite>;

}
