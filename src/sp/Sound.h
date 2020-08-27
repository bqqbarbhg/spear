#pragma once

#include "Asset.h"
#include "sf/Box.h"
#include "Audio.h"

#include "ext/sp_tools_common.h"

namespace sp {

struct Sound : Asset
{
	static AssetType SelfType;
	using PropType = NoAssetProps;

	spsound_info info = { };

	sf::Box<AudioSource> getSource() const;
};

using SoundRef = Ref<Sound>;

}
