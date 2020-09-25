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
	sf::Array<spsound_take> takes;

	sf::Box<AudioSource> getSource(uint32_t takeIndex) const;
};

using SoundRef = Ref<Sound>;

}
