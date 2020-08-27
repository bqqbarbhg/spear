#pragma once

#include "Asset.h"
#include "sf/Box.h"
#include "Audio.h"

namespace sp {

struct Sound : Asset
{
	static AssetType SelfType;
	using PropType = NoAssetProps;

	sf::Box<AudioSource> getSource() const;
};

using SoundRef = Ref<Sound>;

}
