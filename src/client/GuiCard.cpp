#include "GuiCard.h"

#include "ext/imgui/imgui.h"

namespace cl {

void GuiCard::init(const sv::Prefab &prefab, uint32_t svId)
{
	const sv::CardComponent *cardComp = prefab.findComponent<sv::CardComponent>();
	if (!cardComp) return;

	this->svId = svId;

	nameFont.load(sf::Symbol("Assets/Gui/Font/Alegreya-Regular.ttf"));
	descriptionStyle.font.regular.load(sf::Symbol("Assets/Gui/Font/Overlock-Regular.ttf"));
	descriptionStyle.font.bold.load(sf::Symbol("Assets/Gui/Font/Overlock-Bold.ttf"));
	descriptionStyle.font.italic.load(sf::Symbol("Assets/Gui/Font/Overlock-Italic.ttf"));
	descriptionStyle.font.boldItalic.load(sf::Symbol("Assets/Gui/Font/Overlock-BoldItalic.ttf"));

	image.load(cardComp->image);

	if (cardComp->melee) {
		slot = GuiCardSlot::Melee;
	} else if (cardComp->skill) {
		slot = GuiCardSlot::Skill;
	} else if (cardComp->spell) {
		slot = GuiCardSlot::Spell;
	} else if (cardComp->item) {
		slot = GuiCardSlot::Item;
	} else {
		slot = GuiCardSlot::Count;
	}

	name = cardComp->name;
	description = cardComp->description;

	if (cardComp->melee) {
		frame.load(sf::Symbol("Assets/Gui/Card/Melee_Frame.png"));
		background.load(sf::Symbol("Assets/Gui/Card/Melee_Background.png"));
		const sv::CardMeleeComponent *meleeComp = prefab.findComponent<sv::CardMeleeComponent>();

		melee = true;
		if (meleeComp) {
			if (meleeComp->hitRoll.die == 4) {
				dice.load(sf::Symbol("Assets/Gui/Card/Dice_4.png"));
			}
		}
	} else {
		switch (slot) {
		case GuiCardSlot::Spell: frame.load(sf::Symbol("Assets/Gui/Card/Spell_Frame.png")); break;
		case GuiCardSlot::Skill: frame.load(sf::Symbol("Assets/Gui/Card/Skill_Frame.png")); break;
		case GuiCardSlot::Item: frame.load(sf::Symbol("Assets/Gui/Card/Item_Frame.png")); break;
		}
		
	}
}

void renderCard(sp::Canvas &canvas, const GuiCard &card)
{
#if 0
	if (card.background) {
		canvas.draw(card.background, sf::Vec2(0.0f, 0.0f), sf::Vec2(500.0f, 800.0f));
	}
#endif

	if (card.melee) {
		canvas.draw(card.image, sf::Vec2(25.0f, 25.0f), sf::Vec2(450.0f, 450.0f));

		if (card.dice) {
			canvas.draw(card.dice, sf::Vec2(370.0f, 370.0f), sf::Vec2(100.0f, 100.0f));
		}

	} else {
		canvas.draw(card.image, sf::Vec2(25.0f, 25.0f), sf::Vec2(450.0f, 333.0f));
	}

	static float nameOffset = 446.0f;
	static float nameMeleeOffset = 542.0f;
	static float nameFontHeight = 72.0f;
	static float bodyOffset = 515.0f;
	static float bodyMeleeOffset = 610.0f;
	static float bodyFontHeight = 43.0f;
	static float bodyPad = 60.0f;
	static sf::Vec3 titleColor = { 0.965686262f, 0.923917055f, 0.847342372f };
	static sf::Vec3 bodyColor = { 0.0686274767f, 0.0671391934f, 0.0649269745f };

#if 0
	static int prevFrameCount = 0;
	int frameCount = ImGui::GetFrameCount();
	if (prevFrameCount != frameCount) {
		prevFrameCount = frameCount;
		if (ImGui::Begin("Card tweak")) {
			ImGui::InputFloat("Name offset", &nameOffset);
			ImGui::InputFloat("Name melee offset", &nameMeleeOffset);
			ImGui::InputFloat("Name font height", &nameFontHeight);
			ImGui::InputFloat("Body offset", &bodyOffset);
			ImGui::InputFloat("Body melee offset", &bodyMeleeOffset);
			ImGui::InputFloat("Body font height", &bodyFontHeight);
			ImGui::InputFloat("Body pad", &bodyPad);
			ImGui::ColorEdit3("Title color", titleColor.v);
			ImGui::ColorEdit3("Body color", bodyColor.v);
		}
		ImGui::End();
	}
#endif

	float nameY, descY;
	if (!card.melee) {
		nameY = nameOffset;
		descY = bodyOffset;
	} else {
		nameY = nameMeleeOffset;
		descY = bodyMeleeOffset;
	}

	if (card.frame) {
		canvas.draw(card.frame, sf::Vec2(0.0f, 0.0f), sf::Vec2(500.0f, 800.0f));
	}

	if (card.nameFont) {
		float nameHeight = nameFontHeight;
		sf::Vec2 size = card.nameFont->measureText(card.name, nameHeight);
		float width = size.x;

		sp::TextDraw draw;
		draw.font = card.nameFont;
		draw.string = card.name;
		draw.transform = sf::mat2D::translate(sf::Vec2(250.0f - width * 0.5f, nameY));
		draw.height = nameHeight;
		draw.color = sf::Vec4(titleColor, 1.0f);
		canvas.drawText(draw);
	}

	{
		float pad = bodyPad;
		sp::RichTextDesc desc;
		desc.style = &card.descriptionStyle;
		desc.wrapWidth = 500.0f - 2.0f * pad;
		desc.fontHeight = bodyFontHeight;
		desc.offset.x = pad;
		desc.offset.y = descY;
		desc.baseColor = sf::Vec4(bodyColor, 1.0f);
		sp::drawRichText(canvas, desc, card.description);
	}
}

}
