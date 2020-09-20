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

	sp::SpriteRef characterSelect;
	sp::SpriteRef characterActive;
	sp::SpriteRef characterMove;

	sp::SpriteRef statusIconOutline;
	sp::SpriteRef statusHealthOutline;
	sp::SpriteRef statusHealthFilling;
	sp::FontRef statusFont;

	sp::SpriteRef buttonSprite;
	sp::FontRef buttonFont;

	sp::FontRef damageFont;

	GuiResources();
};

} }
