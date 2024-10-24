#pragma once

#include "sf/Array.h"

namespace sp {

struct Image
{
	uint8_t *data;
	uint32_t width, height;

	Image() : data(nullptr), width(0), height(0) { }
	Image(uint32_t width, uint32_t height)
		: data((uint8_t*)sf::memAlloc(width * height * 4))
		, width(width)
		, height(height)
	{
	}
	Image(const Image &rhs) = delete;
	Image(Image &&rhs) noexcept : data(rhs.data), width(rhs.width), height(rhs.height) {
		rhs.data = nullptr;
	}
	~Image()
	{
		sf::memFree(data);
	}

	void clear()
	{
		memset(data, 0, width * height * 4);
	}

	size_t byteSize() const { return width * height * 4; }

	void blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data, uint32_t stride);
	void blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data);
	void premultiply();
	void clear(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
	void clear(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *value);
};

struct MipImage
{
	sf::Array<Image> levels;
	uint32_t width, height;

	MipImage() : width(0), height(0) { }
	MipImage(uint32_t width, uint32_t height);
	MipImage(uint32_t width, uint32_t height, uint32_t numLevels);
	MipImage(const MipImage&) = delete;
	MipImage(MipImage&&) = default;
	MipImage &operator=(const MipImage&) = delete;
	MipImage &operator=(MipImage&&) = default;

	void clear();
	void calculateMips();
	void calculateMips(uint32_t num, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
};

}
