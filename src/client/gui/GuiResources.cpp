#include "GuiResources.h"

namespace cl { namespace gui {

GuiResources::GuiResources()
{
	inventory.load("Assets/Gui/Card/Inventory.png");
	inventoryOpen.load("Assets/Gui/Card/Inventory_Open.png");
	inventorySlot.load("Assets/Gui/Card/Inventory_Slot.png");
	cardSilhouette.load("Assets/Gui/Card/Card_Silhouette.png");
	slotPlaceholders[(uint32_t)GuiCardSlot::Melee].load("Assets/Gui/Card/Slot_Melee.png");
	slotPlaceholders[(uint32_t)GuiCardSlot::Skill].load("Assets/Gui/Card/Slot_Skill.png");
	slotPlaceholders[(uint32_t)GuiCardSlot::Spell].load("Assets/Gui/Card/Slot_Spell.png");
	slotPlaceholders[(uint32_t)GuiCardSlot::Item].load("Assets/Gui/Card/Slot_Item.png");
}

} }
