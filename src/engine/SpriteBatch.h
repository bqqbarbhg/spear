#pragma once

#include "Sprite.h"
#include "sf/Matrix.h"
#include "sf/Vector.h"

struct SpriteBatch
{
	SpriteBatch();
	~SpriteBatch();
	SpriteBatch(const SpriteBatch &) = delete;
	SpriteBatch(SpriteBatch &&rhs) noexcept : data(rhs.data) { rhs.data = nullptr; }

	void draw(Sprite sprite, const sf::Mat23 &transform, const sf::Vec4 &color);
	void end();

	struct Data;
	Data *data;
};
