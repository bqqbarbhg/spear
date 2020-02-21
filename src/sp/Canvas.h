#pragma once

#include "sf/Base.h"
#include "sf/Matrix.h"
#include "sf/Vector.h"
#include "sf/String.h"

namespace sp {

struct Sprite;
struct Font;
struct Canvas;

struct CanvasRenderOpts
{
	sf::Mat44 transform;
	sf::Vec4 color = sf::Vec4(1.0f);

	static CanvasRenderOpts windowPixels();
	static CanvasRenderOpts pixels(uint32_t width, uint32_t height);
};

struct SpriteDraw
{
	Sprite *sprite = nullptr;
	sf::Mat23 transform;
	sf::Vec2 anchor;
	sf::Vec4 color = sf::Vec4(1.0f);
	float depth = 0.0f;
};

struct TextDraw
{
	Font *font = nullptr;
	sf::String string;
	sf::Mat23 transform;
	sf::Vec4 color = sf::Vec4(1.0f);
	float depth = 0.0f;
};

struct CanvasDraw
{
	Canvas *canvas;
	sf::Mat23 transform;
	sf::Vec4 color = sf::Vec4(1.0f);
	float depth = 0.0f;
};

struct CanvasImp;
struct Canvas
{
	Canvas();
	~Canvas();
	Canvas(Canvas &&rhs) noexcept;
	Canvas(const Canvas &rhs) = delete;

	void clear();

	void draw(const SpriteDraw &draw);
	void drawText(const TextDraw &draw);
	void drawCanvas(const CanvasDraw &draw);

	void render(const CanvasRenderOpts &opts);

	bool isLoaded() const;

	alignas(void*) char impData[128];

	// -- Static API

	// Lifecycle
	static void globalInit();
	static void globalCleanup();
	static void globalUpdate();
};

}
