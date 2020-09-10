#pragma once

uniform samplerCube envmap;

vec3 EnvDFGPolynomial(vec3 f0, float gloss, float nDotV)
{
    float x = gloss;
    float y = nDotV;
 
    float b1 = -0.1688;
    float b2 = 1.895;
    float b3 = 0.9903;
    float b4 = -4.853;
    float b5 = 8.404;
    float b6 = -5.069;
    float bias = saturate( min( b1 * x + b2 * x * x, b3 + b4 * y + b5 * y * y + b6 * y * y * y ) );
 
    float d0 = 0.6045;
    float d1 = 1.699;
    float d2 = -0.5228;
    float d3 = -3.603;
    float d4 = 1.404;
    float d5 = 0.1939;
    float d6 = 2.661;
    float delta = saturate( d0 + d1 * x + d2 * y + d3 * x * x + d4 * x * y + d5 * y * y + d6 * x * x * x );
    float scale = delta - bias;
 
    bias *= saturate( 50.0 * f0.y );
    return f0 * scale + bias;
}

vec3 evaluateIBL(vec3 N, vec3 V, vec3 cdiff, vec3 f0, float roughness)
{
    float gloss = 1.0 - roughness;
    gloss *= gloss;
    vec3 R = reflect(-V, N);
    const float NumSpecularLods = 5.0;
    const float DiffuseLod = 6.0;

    vec3 diffEnv = textureLod(envmap, N, DiffuseLod).xyz;
    vec3 specEnv = textureLod(envmap, R, roughness * NumSpecularLods).xyz;

    float nDotV = dot(N, V);
    vec3 result = cdiff * diffEnv + specEnv * EnvDFGPolynomial(f0, gloss, nDotV);

    return result * 0.33;
}
