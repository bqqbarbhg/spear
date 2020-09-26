#include "GuiResources.h"

namespace cl { namespace gui {

GuiResources::GuiResources()
{
	inventory.load("Assets/Gui/Card/Inventory.png");
	inventoryOpen.load("Assets/Gui/Card/Inventory_Open.png");
	inventorySlot.load("Assets/Gui/Card/Inventory_Slot.png");
	cardSilhouette.load("Assets/Gui/Card/Card_Silhouette.png");
	cardOutline.load("Assets/Gui/Card/Card_Outline.png");
	characterSelect.load("Assets/Billboards/Character_Select.png");
	characterMove.load("Assets/Billboards/Character_Move.png");
	characterActive.load("Assets/Billboards/Character_Active.png");

	statusIconOutline.load("Assets/Gui/Status/Icon_Outline.png");
	statusHealthOutline.load("Assets/Gui/Status/Health_Outline.png");
	statusHealthFilling.load("Assets/Gui/Status/Health_Filling.png");
	statusFont.load("Assets/Gui/Font/Overlock-Bold.ttf");

	slotPlaceholders[(uint32_t)GuiCardSlot::Melee].load("Assets/Gui/Card/Slot_Melee.png");
	slotPlaceholders[(uint32_t)GuiCardSlot::Skill].load("Assets/Gui/Card/Slot_Skill.png");
	slotPlaceholders[(uint32_t)GuiCardSlot::Spell].load("Assets/Gui/Card/Slot_Spell.png");
	slotPlaceholders[(uint32_t)GuiCardSlot::Item].load("Assets/Gui/Card/Slot_Item.png");

	buttonSprite.load("Assets/Gui/Misc/Button_Base.png");
	buttonFont.load("Assets/Gui/Font/Alegreya-Regular.ttf");
	damageFont.load("Assets/Gui/Font/Overlock-Regular.ttf");

	tutorialRichStyle.font.regular.load(sf::Symbol("Assets/Gui/Font/Overlock-Regular.ttf"));
	tutorialRichStyle.font.bold.load(sf::Symbol("Assets/Gui/Font/Overlock-Bold.ttf"));
	tutorialRichStyle.font.italic.load(sf::Symbol("Assets/Gui/Font/Overlock-Italic.ttf"));
	tutorialRichStyle.font.boldItalic.load(sf::Symbol("Assets/Gui/Font/Overlock-BoldItalic.ttf"));
}

} }
