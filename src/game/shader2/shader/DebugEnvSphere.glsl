
#include "util/Defines.glsl"

layout(location=0) attribute vec3 a_position;

varying vec3 v_position;
varying vec3 v_normal;

#ifdef SP_VS

uniform DebugEnvSphereVertex
{
    mat4 worldToClip;
    vec4 sphereGridMad;
    ivec2 sphereGridSize;
    vec4 layerHeights;
    int numLayers;
    float sphereRadius;
};

void main()
{
    #if SP_METAL
        uint instanceIndex = uint(gl_InstanceIndex);
    #else
        uint instanceIndex = uint(gl_InstanceID);
    #endif

    uint layer = uint(instanceIndex % uint(numLayers));
    float height;
    if (layer == uint(0)) height = layerHeights.x;
    else if (layer == uint(1)) height = layerHeights.y;
    else if (layer == uint(2)) height = layerHeights.z;
    else height = layerHeights.w;

    uint coordIndex = instanceIndex / uint(numLayers);
    ivec2 coord = ivec2(int(coordIndex % uint(sphereGridSize.x)), int(coordIndex / uint(sphereGridSize.x)));
    vec2 coordF = vec2(coord) * sphereGridMad.xy + sphereGridMad.zw;

    vec3 origin = vec3(coordF.x, height, coordF.y);
    v_position = origin;
    v_normal = a_position;
    gl_Position = mul(vec4(origin + a_position * sphereRadius, 1.0), worldToClip);
}

#endif

#ifdef SP_FS

uniform DebugEnvSpherePixel
{
    vec4 diffuseEnvmapMad;
    vec3 cameraPosition;
    float specular;
};

#include "light/IBL.glsl"
#include "util/Tonemap.glsl"

out vec4 o_color;

void main()
{
    vec3 result = asVec3(0.0);

    vec3 P = v_position;
    vec3 N = normalize(v_normal);
    vec3 V = normalize(cameraPosition - P);

    vec3 cdiff = asVec3(1.0 - specular);
    vec3 f0 = asVec3(specular);
	result = evaluateIBL(P, N, V, cdiff, f0, 0.0);

	o_color = tonemap(result);
}

#endif

