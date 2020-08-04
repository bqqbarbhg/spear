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
	bool noBreak = false;
	sf::Vec4 color = sf::Vec4(0.0f);
	float relativeHeight = 1.0f;
	float depth = 0.0f;
};

struct RichSprite
{
	sp::SpriteRef sprite;
	sf::Vec4 color = sf::Vec4(1.0f);
	bool glueLeft = false;
	bool glueRight = false;
	bool useFontColor = false;
	float depth = 0.0f;
	sf::Vec2 anchor;
	sf::Mat23 transform;
	float layoutWidth = 0.0f;
};

struct RichTagSprite
{
	RichSprite open;
	RichSprite close;
};

struct RichTextStyle
{
	RichFont font;
	sf::HashMap<RichTagString, RichFont> fontTags;
	sf::HashMap<RichTagString, RichTagSprite> spriteTags;
};

struct RichTextDesc
{
	RichTextStyle *style = nullptr;
	bool allowBold = true;
	bool allowItalic = true;
	bool allowColorHex = true;
	bool allowNoBreak = true;
	bool allowSoftHyphen = true;
	bool allowSoftBreak = true;
	bool allowParagraphs = true;
	bool defaultBold = false;
	bool defaultItalic = false;
	bool overrideBaseFontColor = false;
	bool overflow = false;
	sf::Vec2 offset;
	sf::Vec4 baseColor;
	float wrapWidth = 10000.0f;
	float fontHeight = 0.0f;
	float lineSpacing = 1.0f;
	float paragraphSpacing = 1.5f;
};

sf::Vec2 drawRichText(sp::Canvas &canvas, const RichTextDesc &desc, sf::String paragraphs);

}
