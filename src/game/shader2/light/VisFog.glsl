#pragma once

uniform sampler2D visFogTexture;

vec3 evaluateVisFog(vec3 color, vec3 p)
{
    vec2 uv = vec2(p.x, p.z) * visFogMad.xy + visFogMad.zw;
    vec2 factor = textureLod(visFogTexture, uv, 0.0).xy;
    float t = saturate((factor.x - 0.45) * 20.0);
    float u = factor.y;
    return color * (u*u*t);
}

