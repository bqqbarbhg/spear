#pragma once

#include "sp/Canvas.h"
#include "sp/Font.h"
#include "sp/Sprite.h"
#include "sf/Vector.h"
#include "sf/HashMap.h"

namespace sp {

using RichTagString = sf::SmallStringBuf<8>;

struct RichFont
{
	sp::FontRef regular;
	sp::FontRef bold;
	sp::FontRef italic;
	sp::FontRef boldItalic;
	bool hasColor = false;
	sf::Vec4 color = sf::Vec4(0.0f);
	float relativeHeight = 1.0f;
};

struct RichSprite
{
	sp::SpriteRef sprite;
	sf::Vec4 color = sf::Vec4(1.0f);
};

struct RichTextStyle
{
	RichFont font;
	sf::HashMap<RichTagString, RichFont> fontTags;
	sf::HashMap<RichTagString, RichSprite> spriteTags;
};

struct RichTextDesc
{
	RichTextStyle *style = nullptr;
	bool allowBold = true;
	bool allowItalic = true;
	bool allowColorHex = true;
	bool defaultBold = false;
	bool defaultItalic = false;
	bool overrideBaseFontColor = false;
	sf::Vec2 offset;
	sf::Vec4 baseColor;
	float wrapWidth = 10000.0f;
	float fontHeight = 0.0f;
	float paragraphSpacing = 1.5f;
};

sf::Vec2 drawRichText(sp::Canvas &canvas, const RichTextDesc &desc, sf::Slice<const sf::String> paragraphs);

}
