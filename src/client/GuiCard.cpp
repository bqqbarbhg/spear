#include "GuiCard.h"

#include "ext/imgui/imgui.h"

namespace cl {

void GuiCard::init(const sv::Prefab &prefab)
{
	const sv::CardComponent *cardComp = prefab.findComponent<sv::CardComponent>();
	if (!cardComp) return;

	nameFont.load(sf::Symbol("Assets/Gui/Font/Alegreya-Regular.ttf"));
	descriptionStyle.font.regular.load(sf::Symbol("Assets/Gui/Font/Overlock-Regular.ttf"));
	descriptionStyle.font.bold.load(sf::Symbol("Assets/Gui/Font/Overlock-Bold.ttf"));
	descriptionStyle.font.italic.load(sf::Symbol("Assets/Gui/Font/Overlock-Italic.ttf"));
	descriptionStyle.font.boldItalic.load(sf::Symbol("Assets/Gui/Font/Overlock-BoldItalic.ttf"));

	image.load(cardComp->image);

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
	}

	if (cardComp->spell) {
		frame.load(sf::Symbol("Assets/Gui/Card/Spell_Frame.png"));
	}
}

void renderCard(sp::Canvas &canvas, const GuiCard &card)
{
	if (card.background) {
		canvas.draw(card.background, sf::Vec2(0.0f, 0.0f), sf::Vec2(500.0f, 800.0f));
	}

	if (card.melee) {
		canvas.draw(card.image, sf::Vec2(0.0f, 0.0f), sf::Vec2(500.0f, 500.0f));

		if (card.dice) {
			canvas.draw(card.dice, sf::Vec2(370.0f, 370.0f), sf::Vec2(100.0f, 100.0f));
		}

	} else {
		canvas.draw(card.image, sf::Vec2(0.0f, 0.0f), sf::Vec2(500.0f, 500.0f));
	}

	static float nameOffset = 540.0f;
	static float nameFontHeight = 80.0f;
	static float bodyOffset = 610.0f;
	static float bodyFontHeight = 50.0f;
	static float bodyPad = 35.0f;

	if (card.melee) {
		if (ImGui::Begin("Card tweak")) {
			ImGui::InputFloat("Name offset", &nameOffset);
			ImGui::InputFloat("Name font height", &nameFontHeight);
			ImGui::InputFloat("Body offset", &bodyOffset);
			ImGui::InputFloat("Body font height", &bodyFontHeight);
			ImGui::InputFloat("Body pad", &bodyPad);
		}
		ImGui::End();
	}

	float nameY, descY;
	nameY = nameOffset;
	descY = bodyOffset;

	if (!card.melee) {
		nameY -= 110.0f;
		descY -= 110.0f;
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
		draw.color = sf::Vec4(0.0f, 0.0f, 0.0f, 1.0f);
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
		desc.baseColor = sf::Vec4(0.0f, 0.0f, 0.0f, 0.9f);
		sp::drawRichText(canvas, desc, card.description);
	}
}

}
