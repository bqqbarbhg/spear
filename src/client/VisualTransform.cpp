#include "VisualTransform.h"

namespace cl {

VisualTransform VisualHermite::evaluate(float t) const
{
	float t2 = t * t;
	float t3 = t2 * t;
	float h00 = 2.0f*t3 - 3.0f*t2 + 1.0f;
	float h10 = t3 - 2.0f*t2 + t;
	float h01 = -2.0f*t3 + 3.0f*t2;
	float h11 = t3 - t2;
	return VisualTransform::lcomb4(p0, h00, d0, h10, p1, h01, d1, h11);
}

VisualTransform VisualHermite::derivative(float t) const
{
	float t2 = t * t;
	float h00 = 6.0f*t2 - 6.0f*t;
	float h10 = 3.0f*t2 - 4.0f*t + 1.0f;
	float h01 = -h00;
	float h11 = 3.0f*t2 - 2.0f*t;
	return VisualTransform::lcomb4(p0, h00, d0, h10, p1, h01, d1, h11);
}

}
