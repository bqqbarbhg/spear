#pragma once

#include "sf/Float4.h"
#include "sf/Matrix.h"
#include "sf/Geometry.h"

namespace sf {

struct Frustum
{
	Float4 side[4];
	Float4 caps[4];

	Frustum() { }

	Frustum(const sf::Mat44 &mat, float clipNearW)
	{
		Float4 r0 = Float4::loadu(mat.cols[0].v);
		Float4 r1 = Float4::loadu(mat.cols[1].v);
		Float4 r2 = Float4::loadu(mat.cols[2].v);
		Float4 r3 = Float4::loadu(mat.cols[3].v);
		Float4::transpose4(r0, r1, r2, r3);

		{
			Float4 a = r3 - r0, b = r3 + r0, c = r3 - r1, d = r3 + r1;
			Float4::transpose4(a, b, c, d);
			Float4 rcpLen = (a*a + b*b + c*c).rsqrt();
			a *= rcpLen; b *= rcpLen; c *= rcpLen; d *= rcpLen;
			side[0] = a; side[1] = b; side[2] = c; side[3] = d;
		}

		{
			Float4 c0 = Float4::loadu(mat.cols[2].v);
			Float4 a = r3 * -clipNearW + r2, b = r3 - r2, c = a, d = b;
			Float4::transpose4(a, b, c, d);
			Float4 rcpLen = (a*a + b*b + c*c).rsqrt();
			a *= rcpLen; b *= rcpLen; c *= rcpLen; d *= rcpLen;
			caps[0] = a; caps[1] = b; caps[2] = c; caps[3] = d;
		}
	}

	bool intersects(const sf::Sphere &sphere) const
	{
		Float4 r = sphere.radius;
		Float4 so = side[3] + r, co = caps[3] + r;
		Float4 o, s, c;

		o = sphere.origin.x; s = side[0]; c = caps[0];
		so += s*o; co += c*o;

		o = sphere.origin.y; s = side[1]; c = caps[1];
		so += s*o; co += c*o;

		o = sphere.origin.z; s = side[2]; c = caps[2];
		so += s*o; co += c*o;

		return so.min(co).allGreaterThanZero();
	}

	bool intersects(const sf::Bounds3 &bounds) const
	{
		Float4 so = side[3], co = caps[3];
		Float4 o, e, s, c;

		o = bounds.origin.x; e = bounds.extent.x; s = side[0]; c = caps[0];
		so += s*o; co += c*o; so += s.abs()*e; co += c.abs()*e;

		o = bounds.origin.y; e = bounds.extent.y; s = side[1]; c = caps[1];
		so += s*o; co += c*o; so += s.abs()*e; co += c.abs()*e;

		o = bounds.origin.z; e = bounds.extent.z; s = side[2]; c = caps[2];
		so += s*o; co += c*o; so += s.abs()*e; co += c.abs()*e;

		return so.min(co).allGreaterThanZero();
	}

	bool intersects(const sf::Cube3 &cube) const
	{
		Float4 so = side[3], co = caps[3];
		Float4 o, e, s, c;

		o = cube.origin.x; e = cube.extent; s = side[0]; c = caps[0];
		so += s*o; co += c*o; so += s.abs()*e; co += c.abs()*e;

		o = cube.origin.y; e = cube.extent; s = side[1]; c = caps[1];
		so += s*o; co += c*o; so += s.abs()*e; co += c.abs()*e;

		o = cube.origin.z; e = cube.extent; s = side[2]; c = caps[2];
		so += s*o; co += c*o; so += s.abs()*e; co += c.abs()*e;

		return so.min(co).allGreaterThanZero();
	}

};

}
