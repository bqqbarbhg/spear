#include "BSpline.h"

#include "sf/Array.h"

namespace cl {

sf_inline float evaluateBSplineX(const sf::Vec2 &a, const sf::Vec2 &b, const sf::Vec2 &c, const sf:: Vec2 &d, float t)
{
	float nt = 1.0f - t;
	float t2 = t*t;
	float t3 = t2*t;
	float nt2 = nt*nt;
	float nt3 = nt2*nt;

	float p = a.x * (nt3 / 6.0f);
	p += b.x * ((3.0f*t3 - 6.0f*t2 + 4.0f) / 6.0f);
	p += c.x * ((-3.0f*t3 + 3.0f*t2 + 3.0f*t + 1.0f) / 6.0f);
	p += d.x * (t3 / 6.0f);
	return p;
}

sf_inline float evaluateBSplineY(const sf::Vec2 &a, const sf::Vec2 &b, const sf::Vec2 &c, const sf:: Vec2 &d, float t)
{
	float nt = 1.0f - t;
	float t2 = t*t;
	float t3 = t2*t;
	float nt2 = nt*nt;
	float nt3 = nt2*nt;

	float p = a.y * (nt3 / 6.0f);
	p += b.y * ((3.0f*t3 - 6.0f*t2 + 4.0f) / 6.0f);
	p += c.y * ((-3.0f*t3 + 3.0f*t2 + 3.0f*t + 1.0f) / 6.0f);
	p += d.y * (t3 / 6.0f);
	return p;
}

void discretizeBSplineY(sf::Slice<float> yValues, sf::Slice<const sf::Vec2> points, float xMin, float xMax)
{
	if (yValues.size == 0) return;
	if (yValues.size == 1) {
		float y = points[0].y;
		for (float &dst : yValues) dst = y;
		return;
	}

	float curX = xMin;
	float xStep = (xMax - xMin) / (float)(yValues.size - 1);
	uint32_t curPoint = 0;

	int numPoints = (int)points.size;
	for (int i = -1; i < numPoints; i++) {
		sf::Vec2 a = points[i - 1 >= 0 ? i - 1 : 0];
		sf::Vec2 b = points[i >= 0 ? i : 0];
		sf::Vec2 c = points[i + 1 < numPoints ? i + 1 : numPoints - 1];
		sf::Vec2 d = points[i + 2 < numPoints ? i + 2 : numPoints - 1];

		float maxX = (b.x + d.x + 4.0f * c.x) * (1.0f / 6.0f);

		while (curX <= maxX) {
			const constexpr uint32_t steps = 512;
			const float stepSize = 1.0f / (steps - 1);

			uint32_t first = 0;
			uint32_t count = steps;
			while (count > 0) {
				uint32_t step = count >> 1;
				uint32_t it = first + step;
				float t = (float)it * stepSize;
				float x = evaluateBSplineX(a, b, c, d, t);
				if (x < curX) {
					first = it;
					count -= step + 1;
				} else {
					count = step;
				}
			}

			float t = (float)first * stepSize;
			yValues[curPoint] = evaluateBSplineY(a, b, c, d, t);
			if (++curPoint == yValues.size) return;
			curX += xStep;
		}
	}

	for (; curPoint < yValues.size; curPoint++) {
		yValues[curPoint] = curPoint > 0 ? yValues[curPoint - 1] : 0.0f;
	}
}

}
