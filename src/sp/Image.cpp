#include "Image.h"

namespace sp {

#if 0
  def linearToSrgb(linear: Double): Double = if (linear < 0.0031308) {
    12.92 * linear
  } else {
    1.055 * math.pow(linear, 1.0 / 2.4) - 0.055
  }

  def srgbToLinear(srgb: Double): Double = if (srgb < 0.04045) {
    srgb / 12.92
  } else {
    math.pow((srgb + 0.055) / 1.055, 2.4)
  }
#endif

static float ubyteSrgbToFloat(unsigned v) {
	float f = (float)v / 255.0f;
	if (f < 0.04045) {
		return f / 12.92f;
	} else {
		return powf((f + 0.055f) / 1.055f, 2.4f);
	}
}

static uint8_t floatToUbyteSrgb(float f) {
	float v;
	if (f < 0.0031308) {
		v = 12.92f * f;
	} else {
		v = 1.055f * powf(f, 1.0f / 2.4f) - 0.055f;
	}
	return (uint8_t)(v * 255.0f);
}

static float *generateSrgbLookup()
{
	float *data = (float*)sf::memAlloc(sizeof(float) * 256);
	for (uint32_t i = 0; i < 256; i++) {
		data[i] = ubyteSrgbToFloat(i);
	}
	return data;
}

void Image::blit(uint32_t x, uint32_t y, const void *srcData, uint32_t w, uint32_t h)
{
	sf_assert(x + w <= width);
	sf_assert(y + h <= height);

	uint32_t srcStride = w * 4;
	uint32_t dstStride = width * 4;
	uint8_t *dst = data + y * dstStride + x * 4;
	const uint8_t *src = (const uint8_t*)srcData;
	for (uint32_t dy = 0; dy < h; dy++) {
		memcpy(dst, src, srcStride);
		dst += dstStride;
		src += srcStride;
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

void MipImage::calculateMips()
{
	static float *srgbLookup = generateSrgbLookup();

	for (uint32_t level = 1; level < levels.size; level++) {
		const Image &srcImage = levels[level - 1];
		Image &dstImage = levels[level];

		if (srcImage.width == dstImage.width * 2 && srcImage.height == dstImage.height * 2) {
			uint32_t srcStride = srcImage.width * 4;
			uint32_t dstStride = dstImage.width * 4;

			const uint8_t *src0 = srcImage.data;
			const uint8_t *src1 = src0 + srcStride;

			uint32_t dstHeight = dstImage.height;
			uint8_t *dst = dstImage.data;
			for (uint32_t y = 0; y < dstHeight; y++) {
				uint8_t *dstEnd = dst + dstStride;
				while (dst != dstEnd) {
					float r = (srgbLookup[src0[0]] + srgbLookup[src0[4]] + srgbLookup[src1[0]] + srgbLookup[src1[4]]) * 0.25f;
					float g = (srgbLookup[src0[1]] + srgbLookup[src0[5]] + srgbLookup[src1[1]] + srgbLookup[src1[5]]) * 0.25f;
					float b = (srgbLookup[src0[2]] + srgbLookup[src0[6]] + srgbLookup[src1[2]] + srgbLookup[src1[6]]) * 0.25f;
					unsigned a = (src0[3] + src0[7] + src1[3] + src1[7]) >> 2;

					dst[0] = floatToUbyteSrgb(r);
					dst[1] = floatToUbyteSrgb(g);
					dst[2] = floatToUbyteSrgb(b);
					dst[3] = (uint8_t)a;

					dst += 4;
					src0 += 8;
					src1 += 8;
				}

				src0 += srcStride;
				src1 += srcStride;
			}

		} else {
			sf_failf("TODO: Implement edge case");
		}
	}
}

}
