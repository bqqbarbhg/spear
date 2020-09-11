#pragma once

#ifndef IBL_NO_SPECULAR
uniform samplerCube envmap;
#endif

uniform sampler3D diffuseEnvmapAtlas;

vec3 evaluateIBLDiffuse(vec3 P, vec3 N, vec3 cdiff)
{
    vec2 uv = P.xz * diffuseEnvmapMad.xy + diffuseEnvmapMad.zw;

    float d = P.y * 0.5;
    vec4 shR = textureLod(diffuseEnvmapAtlas, vec3((uv.x + 0.0) * (1.0 / 3.0), uv.y, d), 0.0);
    vec4 shG = textureLod(diffuseEnvmapAtlas, vec3((uv.x + 1.0) * (1.0 / 3.0), uv.y, d), 0.0);
    vec4 shB = textureLod(diffuseEnvmapAtlas, vec3((uv.x + 2.0) * (1.0 / 3.0), uv.y, d), 0.0);

    // const float CSH0 = 0.25;
    // const float CSH1 = 0.5;
    const float CSH0 = 1.0;
    const float CSH1 = 1.0;
    vec4 basisN = vec4(CSH0, CSH1*N.x, CSH1*N.y, CSH1*N.z);

    vec3 diffEnv = vec3(dot(shR, basisN), dot(shG, basisN), dot(shB, basisN));
    vec3 result = cdiff * diffEnv;

	return result;
}

#ifndef IBL_NO_SPECULAR
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


vec3 evaluateIBL(vec3 P, vec3 N, vec3 V, vec3 cdiff, vec3 f0, float roughness)
{
    float gloss = 1.0 - roughness;
    gloss *= gloss;
    vec3 R = reflect(-V, N);

    vec2 uv = P.xz * diffuseEnvmapMad.xy + diffuseEnvmapMad.zw;

    float d = P.y * 0.75;

    float x = uv.x * (1.0 / 6.0);
    vec3 diffEnv;
	diffEnv  = N.x*N.x*textureLod(diffuseEnvmapAtlas, vec3(x + (N.x>0 ? 0.0/6.0 : 1.0/6.0), uv.y, d), 0.0).xyz;
	diffEnv += N.y*N.y*textureLod(diffuseEnvmapAtlas, vec3(x + (N.y>0 ? 2.0/6.0 : 3.0/6.0), uv.y, d), 0.0).xyz;
	diffEnv += N.z*N.z*textureLod(diffuseEnvmapAtlas, vec3(x + (N.z>0 ? 4.0/6.0 : 5.0/6.0), uv.y, d), 0.0).xyz;

#if 0
    const float NumSpecularLods = 5.0;

    vec3 iblSpec = textureLod(envmap, R, roughness * NumSpecularLods).xyz;

    float nDotV = dot(N, V);
    vec3 result = cdiff * diffEnv + specEnv * iblSpec * EnvDFGPolynomial(f0, gloss, nDotV);
#endif

	// return result;
	return diffEnv;
}
#endif
