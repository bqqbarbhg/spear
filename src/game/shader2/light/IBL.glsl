#pragma once

uniform samplerCube envmap;
uniform sampler2DArray diffuseEnvmapAtlas;

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

    float w1;
    float slice0 = 0.0;
    if (P.y < 1.5) {
        w1 = saturate(P.y * (1.0 / 1.5));
    } else {
        w1 = saturate((P.y - 1.5) * (1.0 / 1.5));
        slice0 = 6.0;
    }
    float slice1 = slice0 + 6.0;
    float w0 = 1.0 - w1;

	vec3 diffEnv, specEnv;
	diffEnv  = N.x*N.x*w0*textureLod(diffuseEnvmapAtlas, vec3(uv.x, uv.y, slice0 + (N.x>0 ? 0.0 : 1.0)), 0.0).xyz;
	diffEnv += N.y*N.y*w0*textureLod(diffuseEnvmapAtlas, vec3(uv.x, uv.y, slice0 + (N.y>0 ? 2.0 : 3.0)), 0.0).xyz;
	diffEnv += N.z*N.z*w0*textureLod(diffuseEnvmapAtlas, vec3(uv.x, uv.y, slice0 + (N.z>0 ? 4.0 : 5.0)), 0.0).xyz;
	diffEnv += N.x*N.x*w1*textureLod(diffuseEnvmapAtlas, vec3(uv.x, uv.y, slice1 + (N.x>0 ? 0.0 : 1.0)), 0.0).xyz;
	diffEnv += N.y*N.y*w1*textureLod(diffuseEnvmapAtlas, vec3(uv.x, uv.y, slice1 + (N.y>0 ? 2.0 : 3.0)), 0.0).xyz;
	diffEnv += N.z*N.z*w1*textureLod(diffuseEnvmapAtlas, vec3(uv.x, uv.y, slice1 + (N.z>0 ? 4.0 : 5.0)), 0.0).xyz;

	specEnv  = R.x*R.x*w0*textureLod(diffuseEnvmapAtlas, vec3(uv.x, uv.y, slice0 + (R.x>0 ? 0.0 : 1.0)), 0.0).xyz;
	specEnv += R.y*R.y*w0*textureLod(diffuseEnvmapAtlas, vec3(uv.x, uv.y, slice0 + (R.y>0 ? 2.0 : 3.0)), 0.0).xyz;
	specEnv += R.z*R.z*w0*textureLod(diffuseEnvmapAtlas, vec3(uv.x, uv.y, slice0 + (R.z>0 ? 4.0 : 5.0)), 0.0).xyz;
	specEnv += R.x*R.x*w1*textureLod(diffuseEnvmapAtlas, vec3(uv.x, uv.y, slice1 + (R.x>0 ? 0.0 : 1.0)), 0.0).xyz;
	specEnv += R.y*R.y*w1*textureLod(diffuseEnvmapAtlas, vec3(uv.x, uv.y, slice1 + (R.y>0 ? 2.0 : 3.0)), 0.0).xyz;
	specEnv += R.z*R.z*w1*textureLod(diffuseEnvmapAtlas, vec3(uv.x, uv.y, slice1 + (R.z>0 ? 4.0 : 5.0)), 0.0).xyz;
    const float NumSpecularLods = 5.0;
    const float DiffuseLod = 6.0;

    vec3 iblSpec = textureLod(envmap, R, roughness * NumSpecularLods).xyz;

    float nDotV = dot(N, V);
    vec3 result = cdiff * diffEnv + specEnv * iblSpec * EnvDFGPolynomial(f0, gloss, nDotV);

	return result;
}
