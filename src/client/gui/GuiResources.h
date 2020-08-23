#pragma once

#include "sp/Sprite.h"
#include "sp/Font.h"

#include "client/GuiCard.h"

namespace cl { namespace gui {

struct GuiResources
{
	sp::SpriteRef slotPlaceholders[(uint32_t)GuiCardSlot::Count];
	sp::SpriteRef inventorySlot;
	sp::SpriteRef inventory;
	sp::SpriteRef inventoryOpen;
	sp::SpriteRef cardSilhouette;
	sp::SpriteRef cardOutline;

	GuiResources();
};

} }
