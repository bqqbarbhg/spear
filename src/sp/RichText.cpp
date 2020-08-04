#include "RichText.h"

namespace sp {

struct RichDynamicStyle
{
	sp::Font *font;
	sf::Vec4 color;
	float fontHeight;
	float depth;
	bool noBreak = false;
};

struct RichTextDraw
{
	bool newLine;
	bool newParagraph;
	bool softBreak;
	bool softHyphen;
	RichDynamicStyle style;
	sf::String text;
	const RichSprite *sprite = nullptr;
};

struct RichOffset
{
	uint32_t draw, offset;

	bool operator==(const RichOffset &rhs) const {
		return draw == rhs.draw && offset == rhs.offset;
	}
	bool operator!=(const RichOffset &rhs) const {
		return !(draw == rhs.draw && offset == rhs.offset);
	}
};

struct RichSpan
{
	RichOffset begin, end;
};

static RichOffset findNextCharacter(sf::Slice<const RichTextDraw> draws, const RichOffset &previousEnd)
{
	RichOffset begin = previousEnd;
	begin.offset += 1;
	while (begin.draw < draws.size) {
		const RichTextDraw &draw = draws[begin.draw];
		if (begin.offset >= draw.text.size) {
			begin.offset = 0;
			begin.draw++;
			continue;
		}
		if (begin.offset < draw.text.size) {
			return begin;
		}
	}
	return begin;
}

static RichSpan findNextWord(sf::Slice<const RichTextDraw> draws, const RichOffset &previousEnd)
{
	// Skip leading whitespace
	RichOffset begin = previousEnd;
	while (begin.draw < draws.size) {
		const RichTextDraw &draw = draws[begin.draw];
		if (begin.offset >= draw.text.size) {
			begin.offset = 0;
			begin.draw++;
			continue;
		}
		if (draw.style.noBreak) break;
		while (begin.offset < draw.text.size) {
			if (draw.text.data[begin.offset] != ' ') {
				break;
			}
			begin.offset++;
		}
		if (begin.offset < draw.text.size) break;
	}

	// Find the next whitespace/end
	RichOffset end = begin;
	while (end.draw < draws.size) {
		const RichTextDraw &draw = draws[end.draw];
		if (begin.draw != end.draw && draw.newLine) {
			sf_assert(end.offset == 0);
			break;
		}
		if (end.offset >= draw.text.size || draw.style.noBreak) {
			if (draw.softBreak && (begin.draw < end.draw || begin.offset < draw.text.size)) {
				end.offset = draw.text.size;
				break;
			} else {
				end.offset = 0;
				end.draw++;
				continue;
			}
		}
		while (end.offset < draw.text.size) {
			if (draw.text.data[end.offset] == ' ') {
				break;
			}
			end.offset++;
		}
		if (end.offset < draw.text.size) break;
	}

	return { begin, end };
}

static float measureRichText(sf::Slice<const RichTextDraw> draws, const RichOffset &begin, const RichOffset &end, float *outExtraWidth=nullptr, bool forceHyphen=false) {
	float width = 0.0f;
	RichOffset pos = begin;

	if (outExtraWidth) *outExtraWidth = 0.0f;
	while (pos.draw < draws.size && pos.draw <= end.draw) {
		const RichTextDraw &draw = draws[pos.draw];
		uint32_t endOffset = end.draw == pos.draw ? end.offset : (uint32_t)draw.text.size;
		sf::String part = draw.text.substring(pos.offset, endOffset - pos.offset);
		if (draw.sprite) {
			if (sf::contains(part, '#')) {
				width += draw.sprite->layoutWidth * draw.style.fontHeight;
			}
		} else {
			width += draw.style.font->measureText(part, draw.style.fontHeight).x;
			if (pos.draw == end.draw && ((end.offset == draw.text.size && draw.softHyphen) || forceHyphen) && outExtraWidth) {
				*outExtraWidth = draw.style.font->measureText(sf::String("-"), draw.style.fontHeight).x;
			}
		}

		pos.draw++;
		pos.offset = 0;
	}

	return width;
}

static sf::Vec2 drawRichLine(sf::Slice<const RichTextDraw> draws, const RichOffset &begin, const RichOffset &end, sp::Canvas &canvas, const sf::Vec2 &startPosition, bool forceHyphen=false)
{
	sf::Vec2 origin = startPosition;
	float width = 0.0f;

	RichOffset pos = begin;
	while (pos.draw < draws.size && pos.draw <= end.draw) {
		const RichTextDraw &draw = draws[pos.draw];
		uint32_t endOffset = end.draw == pos.draw ? end.offset : (uint32_t)draw.text.size;
		sf::String part = draw.text.substring(pos.offset, endOffset - pos.offset);
		if (draw.sprite) {
			if (sf::contains(part, '#')) {

				sf::Mat23 space { sf::Uninit };
				space.m00 = draw.style.fontHeight;
				space.m10 = 0.0f;
				space.m01 = 0.0f;
				space.m11 = draw.style.fontHeight;
				space.m02 = origin.x;
				space.m12 = origin.y;

				SpriteDraw spriteDraw;
				spriteDraw.sprite = draw.sprite->sprite;
				spriteDraw.transform = space * draw.sprite->transform;
				spriteDraw.anchor = draw.sprite->anchor;
				spriteDraw.color = draw.sprite->color;
				if (draw.sprite->useFontColor) {
					spriteDraw.color *= draw.style.color;
				}
				spriteDraw.depth = draw.sprite->depth;
				canvas.draw(spriteDraw);

				origin.x += draw.sprite->layoutWidth * draw.style.fontHeight;
			}
		} else {
			TextDraw textDraw;
			textDraw.font = draw.style.font;
			textDraw.string = part;
			textDraw.transform.m02 = origin.x;
			textDraw.transform.m12 = origin.y;
			textDraw.height = draw.style.fontHeight;
			textDraw.color = draw.style.color;
			textDraw.depth = draw.style.depth;
			canvas.drawText(textDraw);

			origin.x += draw.style.font->measureText(part, draw.style.fontHeight).x;

			if (pos.draw == end.draw && ((end.offset == draw.text.size && draw.softHyphen) || forceHyphen)) {
				TextDraw textDraw;
				textDraw.font = draw.style.font;
				textDraw.string = sf::String("-");
				textDraw.transform.m02 = origin.x;
				textDraw.transform.m12 = origin.y;
				textDraw.height = draw.style.fontHeight;
				textDraw.color = draw.style.color;
				textDraw.depth = draw.style.depth;
				canvas.drawText(textDraw);

				origin.x += draw.style.font->measureText(part, draw.style.fontHeight).x;
			}
		}

		pos.draw++;
		pos.offset = 0;
	}

	return origin;
}

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

static RichDynamicStyle evaluateStyle(float fontHeight, const RichFont &baseFont, const sf::Vec4 &baseColor, sf::Slice<const sf::KeyVal<RichTagString, RichFont>*> fonts, sf::Slice<sf::Vec4> colors, int32_t boldCount, int32_t italicCount, int32_t noBreakCount)
{
	RichDynamicStyle style;
	style.color = baseColor;
	style.font = selectFont(baseFont, nullptr, boldCount, italicCount);
	style.fontHeight = fontHeight;
	style.depth = baseFont.depth;
	style.noBreak = noBreakCount > 0;
	for (const auto *pair : fonts) {
		style.font = selectFont(baseFont, style.font, boldCount, italicCount);
		style.fontHeight = fontHeight * pair->val.relativeHeight;
		style.noBreak |= pair->val.noBreak;
		style.depth = pair->val.depth;
		if (pair->val.hasColor) {
			style.color = pair->val.color;
		}
	}
	if (colors.size > 0) {
		style.color = colors[colors.size - 1];
	}
	return style;
}

static void setSpriteDraw(RichTextDraw &draw, const RichSprite *sprite, const RichDynamicStyle &style)
{
	if (sprite->glueLeft && sprite->glueRight) {
		draw.text = sf::String("#");
	} else if (sprite->glueLeft) {
		draw.text = sf::String("# ");
	} else if (sprite->glueRight) {
		draw.text = sf::String(" #");
	} else {
		draw.text = sf::String(" # ");
	}
	draw.style = style;
	draw.sprite = sprite;
	draw.newLine = false;
	draw.newParagraph = false;
}

sf::Vec2 drawRichText(sp::Canvas &canvas, const RichTextDesc &desc, sf::String text)
{
	sf::SmallArray<sf::Vec4, 8> colorStack;
	sf::SmallArray<const sf::KeyVal<RichTagString, RichFont>*, 8> fontStack;
	sf::SmallArray<RichTagString, 8> tagStack;
	RichTextStyle &style = *desc.style;

	sf::Vec4 baseColor = desc.overrideBaseFontColor || !style.font.hasColor ? desc.baseColor : style.font.color;

	int32_t boldCount = desc.defaultBold ? 1 : 0;
	int32_t italicCount = desc.defaultItalic ? 1 : 0;
	int32_t noBreakCount = 0;
	RichDynamicStyle dynamicStyle = evaluateStyle(desc.fontHeight, style.font, baseColor, fontStack, colorStack, boldCount, italicCount, noBreakCount);

	sf::SmallArray<RichTextDraw, 64> draws;

	{
		bool newLine = false;
		bool newParagraph = false;
		size_t begin = 0, end = 0;
		bool reachedEnd = false;
		while (!reachedEnd) {
			bool flush = false;
			bool nextNewLine = false;
			bool nextNewParagraph = false;
			bool softHyphen = false;
			bool softBreak = false;
			bool styleDirty = false;
			const RichSprite *preSprite = nullptr;
			const RichSprite *postSprite = nullptr;

			size_t flushEnd = end;
			if (end >= text.size) {
				reachedEnd = true;
				flush = true;
			} else if (text.data[end] == '\n') {
				flush = true;
				nextNewLine = true;
				end++;
				if (desc.allowParagraphs && end < text.size && text.data[end] == '\n') {
					end++;
					nextNewParagraph = true;
				}
			} else if (end + 1 < text.size && text.data[end] == '\xc2' && text.data[end + 1] == '\xad') {
				flush = true;
				softBreak = true;
				softHyphen = true;
				end += 2;
			} else if (text.data[end] == '[') {
				flush = true;
				if (end + 1 < text.size && text.data[end + 1] == '[') {
					flushEnd = end + 1;
					end += 2;
				} else {
					uint32_t tagBegin = (uint32_t)end + 1;
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

					RichTagString closingTag;
					if (closing) {
						closingTag = tagStack.popValue();
						if (tag.size == 0) {
							tag = closingTag;
						}
					} else {
						if (tag.size > 0 && tag.data[tag.size - 1] == '/') {
							tag.size--;
						} else if (tag.size == 1 && (tag.data[0] == '-' || tag.data[0] == ' ')) {
							// self closing
						} else {
							tagStack.push(RichTagString(tag));
						}
					}

					if (desc.allowBold && tag.size == 1 && tag.data[0] == 'b') {
						boldCount += closing ? -1 : +1;
						styleDirty = true;
					} else if (desc.allowItalic && tag.size == 1 && tag.data[0] == 'i') {
						italicCount += closing ? -1 : +1;
						styleDirty = true;
					} else if (desc.allowNoBreak && tag.size == 2 && tag.data[0] == 'n' && tag.data[1] == 'b') {
						noBreakCount += closing ? -1 : +1;
						styleDirty = true;
					} else if (desc.allowSoftHyphen && tag.size == 1 && tag.data[0] == '-') {
						softBreak = true;
						softHyphen = true;
					} else if (desc.allowSoftBreak && tag.size == 1 && tag.data[0] == ' ') {
						softBreak = true;
					} else if (desc.allowColorHex && tag.size >= 1 && tag.data[0] == '#') {
						if (closing) {
							if (colorStack.size > 0) colorStack.pop();
						} else {
							colorStack.push(parseColor(tag.data + 1, tag.size - 1));
						}
						styleDirty = true;
					} else {
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
							styleDirty = true;
						}
						if (const sf::KeyVal<RichTagString, RichTagSprite> *spriteTag = style.spriteTags.find(tag)) {
							if (closing && spriteTag->val.close.sprite) {
								preSprite = &spriteTag->val.close;
							} else if (!closing && spriteTag->val.open.sprite) {
								postSprite = &spriteTag->val.open;
							}
						}
					}

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
					draw.softBreak = softBreak;
					draw.softHyphen = softHyphen;
					draw.style = dynamicStyle;
				}

				newParagraph = nextNewParagraph;
				newLine = nextNewLine;
				begin = end;
			}

			if (preSprite) {
				setSpriteDraw(draws.push(), preSprite, dynamicStyle);
			}

			if (styleDirty) {
				dynamicStyle = evaluateStyle(desc.fontHeight, style.font, baseColor, fontStack, colorStack, boldCount, italicCount, noBreakCount);
			}

			if (postSprite) {
				setSpriteDraw(draws.push(), postSprite, dynamicStyle);
			}
		}
	}

	// Algorithm: Start with `line.begin/end` ([]) of 0.
	//   Repeat: Find the next whitespace delimited word `word.begin/end` (<>).
	//   `wordEnd` points to the next whitespace character or end-of-text.
	//   Measure the text from `line.end` (inclusive) to `word.end` (exclusive) (]...>)
	//   and add it speculatively to the current line width. If this width is
	//   less than `wrapWidth` (|) accept the new line width and use `wordEnd` as
	//   the new `line.end`. Otherwise flush the current line and start a new one
	//   setting `line.begin/end` to `word.begin/end` and measure `word.begin` (inclusive)
	//   to `word.end` (exclusive) and set that as the new line width.
	//
	// Legend:
	//   [] line.begin/end
	//   <> word.begin/end
	//   | wrapWidth
	//   
	//   This is word wrap example text.
	//   ]   >         |
	//   [   ]< >      |
	//   [      ]<   > |
	//   [           ]<|  >
	//                [|  ] (visualization shift to left)
	//   wrap example text.
	//   [   ]<      > |
	//   [           ]<|   >
	//                [|   ] (visualization shift to left)
	//   text.         |
	//   [    ]        |
	//
	// Output:
	//   This is word
	//   wrap example
	//   text.

	float wrapWidth = sf::max(desc.wrapWidth, 0.0f);
	sf::Vec2 origin = desc.offset;
	float extraWidth = 0.0f;
	float currentWidth = 0.0f;
	RichSpan line = findNextWord(draws, RichOffset{ 0, 0 });
	currentWidth = measureRichText(draws, line.begin, line.end, &extraWidth);

	while (line.begin.draw < draws.size) {

		// Break the current line down until it fits
		while (!desc.overflow && currentWidth + extraWidth > wrapWidth) {

			RichOffset split = { 0, 0 };

			// Find the last draw that fits
			{
				uint32_t drawLo = line.begin.draw;
				uint32_t drawHi = line.end.draw;
				while (drawLo < drawHi) {
					uint32_t drawMid = (drawLo + drawHi) >> 1;
					uint32_t offset = drawMid == line.begin.draw ? line.begin.offset : 0;
					RichOffset end = { drawMid, offset };
					float extra;
					float width = measureRichText(draws, line.begin, end, &extra, true);
					if (width + extra > wrapWidth) {
						drawHi = drawMid;
					} else {
						drawLo = drawMid + 1;
					}
				}
				split.draw = drawHi > line.begin.draw ? drawHi - 1 : line.begin.draw;
			}

			// Find the last offset that fits
			if (split.draw < draws.size) {
				uint32_t offsetStart = split.draw == line.begin.draw ? line.begin.offset : 0;
				uint32_t offsetLo = offsetStart;
				uint32_t offsetHi = draws[split.draw].text.size;
				while (offsetLo < offsetHi) {
					uint32_t offsetMid = (offsetLo + offsetHi) >> 1;
					RichOffset end = { split.draw, offsetMid };
					float extra;
					float width = measureRichText(draws, line.begin, end, &extra, true);
					if (width + extra > wrapWidth) {
						offsetHi = offsetMid;
					} else {
						offsetLo = offsetMid + 1;
					}
				}
				split.offset = offsetHi > offsetStart ? offsetHi - 1 : offsetStart;
			}

			if (split == line.begin) {
				split = findNextCharacter(draws, split);
			}

			drawRichLine(draws, line.begin, split, canvas, origin, true);
			origin.x = desc.offset.x;
			float spacing = desc.lineSpacing;
			origin.y += desc.fontHeight * spacing;

			line.begin = split;
			currentWidth = measureRichText(draws, line.begin, line.end, &extraWidth);
		}

		RichSpan word = findNextWord(draws, line.end);
		float newExtraWidth = 0.0f;
		float newWidth = currentWidth + measureRichText(draws, line.end, word.end, &newExtraWidth);
		if (newWidth + newExtraWidth <= wrapWidth && line.end.draw < draws.size) {
			currentWidth = newWidth;
			extraWidth = newExtraWidth;
			line.end = word.end;
			continue;
		}

		drawRichLine(draws, line.begin, line.end, canvas, origin);
		origin.x = desc.offset.x;
		float spacing = desc.lineSpacing;
		if (word.end.draw > word.begin.draw && word.end.draw < draws.size && draws[word.end.draw].newParagraph) {
			spacing = desc.paragraphSpacing;
		}
		origin.y += desc.fontHeight * spacing;

		currentWidth = measureRichText(draws, word.begin, word.end, &extraWidth);
		line = word;
	}
	return origin - desc.offset;
}

}
