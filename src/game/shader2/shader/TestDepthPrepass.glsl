
#include "util/Defines.glsl"

layout(location=0) attribute vec3 a_position;

#ifdef SP_VS

uniform Transform
{
	mat4 worldToClip;
};

vec3 unpackUnorm10ToSnormVec3(vec3 v) { return v * (1024.0/511.0) - asVec3(1.0); }

void main()
{
    gl_Position = mul(vec4(a_position, 1.0), worldToClip);
}

#endif

#ifdef SP_FS


void main()
{
}

#endif

