#pragma once

#include "server/ServerState.h"

#include "sp/Canvas.h"
#include "sp/Sprite.h"
#include "sp/Font.h"
#include "sp/RichText.h"

namespace cl {

struct GuiCard
{
	static const constexpr float canvasWidth = 500.0f;
	static const constexpr float canvasHeight = 800.0f;
	static const constexpr float canvasXByY = canvasWidth / canvasHeight;
	static const constexpr float canvasYByX = canvasHeight / canvasWidth;

	void init(const sv::Prefab &prefab);

	sp::SpriteRef background;
	sp::SpriteRef frame;
	sp::SpriteRef image;
	sp::SpriteRef dice;

	sp::FontRef nameFont;
	sp::RichTextStyle descriptionStyle;

	bool melee = false;

	sf::Symbol name;
	sf::Symbol description;
};

// Card virtual grid: 500x800
void renderCard(sp::Canvas &canvas, const GuiCard &card);

}
