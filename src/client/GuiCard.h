#pragma once

#include "server/ServerState.h"

#include "sp/Canvas.h"
#include "sp/Sprite.h"
#include "sp/Font.h"
#include "sp/RichText.h"

namespace cl {

enum class GuiCardSlot
{
	Melee,
	Skill,
	Spell,
	Item,
	Count,
};

struct GuiCardResources
{
	sp::SpriteRef slotPlaceholders[(uint32_t)GuiCardSlot::Count];
	sp::SpriteRef inventory { "Assets/Gui/Card/Inventory.png" };

	GuiCardResources()
	{
		slotPlaceholders[(uint32_t)GuiCardSlot::Melee].load("Assets/Gui/Card/Slot_Melee.png");
		slotPlaceholders[(uint32_t)GuiCardSlot::Skill].load("Assets/Gui/Card/Slot_Skill.png");
		slotPlaceholders[(uint32_t)GuiCardSlot::Spell].load("Assets/Gui/Card/Slot_Spell.png");
		slotPlaceholders[(uint32_t)GuiCardSlot::Item].load("Assets/Gui/Card/Slot_Item.png");
	}
};

struct GuiCard
{
	static const constexpr float canvasWidth = 500.0f;
	static const constexpr float canvasHeight = 800.0f;
	static const constexpr float canvasXByY = canvasWidth / canvasHeight;
	static const constexpr float canvasYByX = canvasHeight / canvasWidth;

	struct RenderOpts
	{
		bool showCooldown = true;
	};

	void init(const sv::Prefab &prefab, uint32_t svId);

	uint32_t svId;

	uint32_t prevSlotIndex = ~0u;
	uint64_t prevSlotFrame = 0;

	uint32_t cooldownLeft = 0;

	sp::SpriteRef background;
	sp::SpriteRef frame;
	sp::SpriteRef image;
	sp::SpriteRef dice;
	sp::SpriteRef cooldownIcon;
	sp::SpriteRef cooldownOverlay;

	sp::FontRef nameFont;
	sp::FontRef cooldownFont;
	sp::RichTextStyle descriptionStyle;

	bool melee = false;
	GuiCardSlot slot = GuiCardSlot::Count;

	sf::Symbol name;
	sf::Symbol description;
	sf::SmallStringBuf<8> cooldownText;
};

// Card virtual grid: 500x800
void renderCard(sp::Canvas &canvas, const GuiCard &card, const GuiCard::RenderOpts &opts);

}
