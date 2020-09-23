#pragma once

uniform sampler2D visFogTexture;

vec3 evaluateVisFog(vec3 color, vec3 p)
{
    vec2 uv = vec2(p.x, p.z) * visFogMad.xy + visFogMad.zw;
    vec2 factor = textureLod(visFogTexture, uv, 0.0).xy;
    float t = saturate((factor.x - 0.45) * 20.0);
    float u = factor.y*factor.y;

    vec3 luma = asVec3(dot(color, vec3(0.299, 0.587, 0.114)));
    vec3 desat = lerp(luma * 0.5, color, u);
    return desat * (t * u);
}

