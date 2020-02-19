#include "Image.h"

#include "ext/stb/stb_image_resize.h"

namespace sp {

void Image::blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *srcData, uint32_t srcStride)
{
	sf_assert(x + w <= width);
	sf_assert(y + h <= height);

	uint32_t lineSize = w * 4;
	uint32_t dstStride = width * 4;
	uint8_t *dst = data + y * dstStride + x * 4;
	const uint8_t *src = (const uint8_t*)srcData;
	for (uint32_t dy = 0; dy < h; dy++) {
		memcpy(dst, src, lineSize);
		dst += dstStride;
		src += srcStride;
	}
}

void Image::blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data)
{
	blit(x, y, w, h, data, w * 4);
}

void Image::premultiply()
{
	for (uint8_t *d = data, *e = data + byteSize(); d != e; d += 4) {
		uint32_t a = d[3];
		d[0] = (uint8_t)(d[0] * a / 255);
		d[1] = (uint8_t)(d[1] * a / 255);
		d[2] = (uint8_t)(d[2] * a / 255);
	}
}

void Image::clear(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
	sf_assert(x + w <= width);
	sf_assert(y + h <= height);

	uint32_t srcStride = w * 4;
	uint32_t dstStride = width * 4;
	uint8_t *dst = data + y * dstStride + x * 4;
	for (uint32_t dy = 0; dy < h; dy++) {
		memset(dst, 0, srcStride);
		dst += dstStride;
	}
}

MipImage::MipImage(uint32_t width, uint32_t height)
	: width(width), height(height)
{
	uint32_t w = width, h = height;
	levels.push(Image(w, h));
	while (w > 1 || h > 1) {
		w = w > 1 ? w >> 1 : 1;
		h = h > 1 ? h >> 1 : 1;
		levels.push(Image(w, h));
	}
}

MipImage::MipImage(uint32_t width, uint32_t height, uint32_t numLevels)
	: width(width), height(height)
{
	levels.reserve(numLevels);
	uint32_t w = width, h = height;
	for (uint32_t i = 0; i < numLevels; i++) {
		levels.push(Image(w, h));
		w = w > 1 ? w >> 1 : 1;
		h = h > 1 ? h >> 1 : 1;
	}
}

void MipImage::clear()
{
	for (Image &level : levels) {
		level.clear();
	}
}

void MipImage::calculateMips()
{
	for (uint32_t level = 1; level < levels.size; level++) {
		const Image &src = levels[level - 1];
		Image &dst = levels[level];

		stbir_resize_uint8_srgb(
			src.data, src.width, src.height, 0,
			dst.data, dst.width, dst.height, 0,
			4, 3, STBIR_FLAG_ALPHA_PREMULTIPLIED);
	}
}

}
