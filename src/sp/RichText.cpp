#include "RichText.h"

namespace sp {

struct RichDynamicStyle
{
	sp::Font *font;
	sf::Vec4 color;
	float fontHeight;
};

struct RichTextDraw
{
	bool newLine;
	bool newParagraph;
	RichDynamicStyle style;
	sf::String text;
};

static uint32_t parseHexDigit(char c) {
	if (c >= '0' && c <= '9') return (uint32_t)c - '0';
	else if (c >= 'a' && c <= 'f') return (uint32_t)c - 'a' + 10;
	else if (c >= 'A' && c <= 'F') return (uint32_t)c - 'A' + 10;
	else return 0;
}

static sf::Vec4 parseColor(const char *data, size_t length)
{
	uint32_t r, g, b, a = 255;
	if (length == 1) {
		r = g = b = parseHexDigit(data[0]) * 0x11;
	} else if (length == 3 || length == 4) {
		r = parseHexDigit(data[0]) * 0x11;
		g = parseHexDigit(data[1]) * 0x11;
		b = parseHexDigit(data[2]) * 0x11;
		if (length == 4) a = parseHexDigit(data[3]) * 0x11;
	} else if (length == 6 || length == 8) {
		r = parseHexDigit(data[0]) << 4 | parseHexDigit(data[1]);
		g = parseHexDigit(data[2]) << 4 | parseHexDigit(data[3]);
		b = parseHexDigit(data[4]) << 4 | parseHexDigit(data[5]);
		if (length == 8) a = parseHexDigit(data[6]) << 4 | parseHexDigit(data[7]);
	} else {
		r = g = b = a = 0;
	}
	return sf::Vec4((float)r, (float)g, (float)b, (float)a) * (1.0f / 255.0f);
}

static sp::Font *selectFont(const RichFont &font, sp::Font *fallbackFont, int32_t boldCount, int32_t italicCount)
{
	if (boldCount > 0 && italicCount > 0 && font.boldItalic) {
		return font.boldItalic;
	} else if (boldCount > 0 && font.bold) {
		return font.bold;
	} else if (italicCount > 0 && font.italic) {
		return font.italic;
	} else if (font.regular) {
		return font.regular;
	} else {
		return fallbackFont;
	}
}

static RichDynamicStyle evaluateStyle(float fontHeight, const RichFont &baseFont, const sf::Vec4 &baseColor, sf::Slice<const sf::KeyVal<RichTagString, RichFont>*> fonts, sf::Slice<sf::Vec4> colors, int32_t boldCount, int32_t italicCount)
{
	RichDynamicStyle style;
	style.color = baseColor;
	style.font = selectFont(baseFont, nullptr, boldCount, italicCount);
	style.fontHeight = fontHeight;
	for (const auto *pair : fonts) {
		style.font = selectFont(baseFont, style.font, boldCount, italicCount);
		style.fontHeight = fontHeight * pair->val.relativeHeight;
		if (pair->val.hasColor) {
			style.color = pair->val.color;
		}
	}
	if (colors.size > 0) {
		style.color = colors[colors.size - 1];
	}
	return style;
}

sf::Vec2 drawRichText(sp::Canvas &canvas, const RichTextDesc &desc, sf::Slice<const sf::String> paragraphs)
{
	sf::SmallArray<sf::Vec4, 8> colorStack;
	sf::SmallArray<const sf::KeyVal<RichTagString, RichFont>*, 8> fontStack;
	RichTextStyle &style = *desc.style;

	sf::Vec4 baseColor = desc.overrideBaseFontColor || !style.font.hasColor ? desc.baseColor : style.font.color;

	int32_t boldCount = desc.defaultBold ? 1 : 0;
	int32_t italicCount = desc.defaultItalic ? 1 : 0;
	RichDynamicStyle dynamicStyle = evaluateStyle(desc.fontHeight, style.font, baseColor, fontStack, colorStack, boldCount, italicCount);

	bool firstParagraph = true;
	sf::SmallArray<RichTextDraw, 64> draws;
	for (sf::String text : paragraphs) {

		bool newParagraph = !firstParagraph;
		bool newLine = !firstParagraph;
		firstParagraph = false;

		size_t begin = 0, end = 0;
		bool reachedEnd = false;
		while (!reachedEnd) {
			bool flush = false;
			bool nextNewLine = false;
			bool styleDirty = false;

			size_t flushEnd = end;
			if (end >= text.size) {
				reachedEnd = true;
				flush = true;
			} else if (text.data[end] == '\n') {
				nextNewLine = true;
				flush = true;
				end++;
			} else if (text.data[end] == '[') {
				flush = true;
				if (end + 1 < text.size && text.data[end + 1] == '[') {
					flushEnd = end + 1;
					end += 2;
				} else {
					uint32_t tagBegin = end + 1;
					bool closing = false;
					if (tagBegin < text.size && text.data[tagBegin] == '/') {
						closing = true;
						tagBegin++;
					}
					uint32_t tagEnd = tagBegin;
					for (; tagEnd < text.size; tagEnd++) {
						if (text.data[tagEnd] == ']') break;
					}

					sf::String tag = sf::String(text.data + tagBegin, tagEnd - tagBegin);
					if (const sf::KeyVal<RichTagString, RichFont> *pair = style.fontTags.find(tag)) {
						if (closing) {
							for (int32_t i = (int32_t)fontStack.size - 1; i >= 0; i--) {
								if (fontStack[i] == pair) {
									fontStack.removeOrdered(i);
									break;
								}
							}
						} else {
							fontStack.push(pair);
						}
					} else if (const sf::KeyVal<RichTagString, RichSprite> *sprite = style.spriteTags.find(tag)) {
					} else if (desc.allowBold && tag.size == 1 && tag.data[0] == 'b') {
						boldCount += closing ? -1 : +1;
					} else if (desc.allowItalic && tag.size == 1 && tag.data[0] == 'i') {
						italicCount += closing ? -1 : +1;
					} else if (desc.allowColorHex && tag.size >= 1 && tag.data[0] == '#') {
						if (closing) {
							if (colorStack.size > 0) colorStack.pop();
						} else {
							colorStack.push(parseColor(tag.data + 1, tag.size - 1));
						}
					}
					styleDirty = true;
					end = tagEnd + 1;
				}
			} else {
				end++;
			}

			if (flush) {
				if (dynamicStyle.font) {
					RichTextDraw &draw = draws.push();
					draw.text = sf::String(text.data + begin, flushEnd - begin);
					draw.newLine = newLine;
					draw.newParagraph = newParagraph;
					draw.style = dynamicStyle;
				}

				newParagraph = 0;
				newLine = nextNewLine;
				begin = end;
			}

			if (styleDirty) {
				dynamicStyle = evaluateStyle(desc.fontHeight, style.font, baseColor, fontStack, colorStack, boldCount, italicCount);
			}
		}
	}


	sf::Vec2 pos = sf::Vec2(0.0f);
	float currentWidth = 0.0f;
	uint32_t spanBegin = 0;
	uint32_t drawBegin = 0;
	for (;;) {

		uint32_t spanLength = 0;
		uint32_t drawEnd = drawBegin;
		uint32_t spanEnd = spanBegin;
		for (; drawEnd < draws.size; drawEnd++) {
			uint32_t prevEnd = drawEnd == drawBegin ? spanBegin : 0;
			RichTextDraw &draw = draws[drawEnd];
			if (spanEnd >= spanBegin && (spanEnd == draw.text.size || draw.text.data[spanEnd] == ' ')) {
				sf::String piece { draw.text.data + prevEnd, spanEnd - prevEnd };
				float width = draw.style.font->measureText(piece, draw.style.fontHeight).x;
				if (pos.x + currentWidth + width < desc.wrapWidth) {
					currentWidth += width;
				} else {
				}
			}
			
		}

	}

	for (const RichTextDraw &draw : draws) {
		if (!draw.style.font) continue;

		if (draw.newParagraph) {
			pos.x = 0.0f;
			pos.y += desc.fontHeight * desc.paragraphSpacing;
		} else if (draw.newLine) {
			pos.x = 0.0f;
			pos.y += desc.fontHeight;
		}

		size_t begin = 0;
		size_t prevEnd = 0;
		size_t end = 0;

		float currentWidth = 0.0f;
		for (; end <= draw.text.size; end++) {

			if (end >= begin && (end == draw.text.size || draw.text.data[end] == ' ')) {
				sf::String piece { draw.text.data + prevEnd, end - prevEnd };
				float width = draw.style.font->measureText(piece, draw.style.fontHeight).x;

				if (pos.x + currentWidth + width < desc.wrapWidth) {
					currentWidth += width;
				} else {
					sf::String prevPiece { draw.text.data + begin, prevEnd - begin };

					sp::TextDraw td;
					td.string = prevPiece;
					td.transform.m02 = desc.offset.x + pos.x;
					td.transform.m12 = desc.offset.y + pos.y;
					td.font = draw.style.font;
					td.height = draw.style.fontHeight;
					td.color = draw.style.color;
					canvas.drawText(td);

					begin = prevEnd;
					pos.x = 0.0f;
					pos.y += desc.fontHeight;

					while (begin < draw.text.size && draw.text.data[begin] == ' ') {
						begin++;
					}
					sf::String nextPiece { draw.text.data + begin, end - begin };
					currentWidth = draw.style.font->measureText(nextPiece, draw.style.fontHeight).x;
				}
				prevEnd = end;

				if (end == draw.text.size && begin < end) {
					sf::String lastPiece { draw.text.data + begin, end - begin };

					sp::TextDraw td;
					td.string = lastPiece;
					td.transform.m02 = desc.offset.x + pos.x;
					td.transform.m12 = desc.offset.y + pos.y;
					td.font = draw.style.font;
					td.height = draw.style.fontHeight;
					td.color = draw.style.color;
					canvas.drawText(td);

					pos.x += currentWidth;
				}
			}
		}
	}
	pos.y += desc.fontHeight;
	return pos;
}

}
