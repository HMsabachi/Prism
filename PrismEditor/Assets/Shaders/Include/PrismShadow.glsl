#ifndef PRISM_SHADOW
#define PRISM_SHADOW

#include "Pass/GeometryPass.glsl"

const vec2 PoissonDistribution[64] = vec2[](
    vec2(-0.884081,  0.124488), vec2(-0.714377,  0.027940),
    vec2(-0.747945,  0.227922), vec2(-0.939609,  0.243634),
    vec2(-0.985465,  0.045534), vec2(-0.861367, -0.136222),
    vec2(-0.881934,  0.396908), vec2(-0.466938,  0.014526),
    vec2(-0.558207,  0.212662), vec2(-0.578447, -0.095822),
    vec2(-0.740266, -0.095631), vec2(-0.751681,  0.472604),
    vec2(-0.553147, -0.243177), vec2(-0.674762, -0.330730),
    vec2(-0.402765, -0.122087), vec2(-0.319776, -0.312166),
    vec2(-0.413923, -0.439757), vec2(-0.979153, -0.201245),
    vec2(-0.865579, -0.288695), vec2(-0.243704, -0.186378),
    vec2(-0.294920, -0.055748), vec2(-0.604452, -0.544251),
    vec2(-0.418056, -0.587679), vec2(-0.549156, -0.415877),
    vec2(-0.238080, -0.611761), vec2(-0.267004, -0.459702),
    vec2(-0.100006, -0.229116), vec2(-0.101928, -0.380382),
    vec2(-0.681467, -0.700773), vec2(-0.763488, -0.543386),
    vec2(-0.549030, -0.750749), vec2(-0.809045, -0.408738),
    vec2(-0.388134, -0.773448), vec2(-0.429392, -0.894892),
    vec2(-0.131597,  0.065058), vec2(-0.275002,  0.102922),
    vec2(-0.106117, -0.068327), vec2(-0.294586, -0.891515),
    vec2(-0.629418,  0.379387), vec2(-0.407257,  0.339748),
    vec2( 0.071650, -0.384284), vec2( 0.022018, -0.263793),
    vec2( 0.003879, -0.136073), vec2(-0.137533, -0.767844),
    vec2(-0.050874, -0.906068), vec2( 0.114133, -0.070053),
    vec2( 0.163314, -0.217231), vec2(-0.100262, -0.587992),
    vec2(-0.004942,  0.125368), vec2( 0.035302, -0.619310),
    vec2( 0.195646, -0.459022), vec2( 0.303969, -0.346362),
    vec2(-0.678118,  0.685099), vec2(-0.628418,  0.507978),
    vec2(-0.508473,  0.458753), vec2( 0.032134, -0.782030),
    vec2( 0.122595,  0.280353), vec2(-0.043643,  0.312119),
    vec2( 0.132993,  0.085170), vec2(-0.192106,  0.285848),
    vec2( 0.183621, -0.713242), vec2( 0.265220, -0.596716),
    vec2(-0.009628, -0.483058), vec2(-0.018516,  0.435703)
);

vec2 SamplePoisson(int index)
{
    return PoissonDistribution[index % 64];
}

float GetShadowBias(vec3 normal, vec3 lightDir)
{
    float baseBias = Prism_ShadowParams.x;
    float bias = max(baseBias * (1.0 - dot(normal, lightDir)), baseBias);
    return bias;
}

float HardShadows_DirectionalLight(sampler2D shadowMap, vec3 shadowCoords, float bias, float shadowFade)
{
    float z = texture(shadowMap, shadowCoords.xy).r;
    return 1.0 - step(z + bias, shadowCoords.z) * shadowFade;
}

float FindBlockerDistance_DirectionalLight(sampler2D shadowMap, vec3 shadowCoords, float bias)
{
    int numBlockerSearchSamples = 64;
    int blockers = 0;
    float avgBlockerDistance = 0.0;

    vec2 searchWidth = vec2(0.05);
    for (int i = 0; i < numBlockerSearchSamples; i++)
    {
        float z = texture(shadowMap, shadowCoords.xy + SamplePoisson(i) * searchWidth).r;
        if (z < (shadowCoords.z - bias))
        {
            blockers++;
            avgBlockerDistance += z;
        }
    }

    if (blockers > 0)
        return avgBlockerDistance / float(blockers);

    return -1.0;
}

float PCF_DirectionalLight(sampler2D shadowMap, vec3 shadowCoords, float bias, float uvRadius)
{
    int numPCFSamples = 64;
    float sum = 0.0;
    for (int i = 0; i < numPCFSamples; i++)
    {
        float z = texture(shadowMap, shadowCoords.xy + SamplePoisson(i) * uvRadius).r;
        sum += (z < (shadowCoords.z - bias)) ? 1.0 : 0.0;
    }
    return sum / float(numPCFSamples);
}

float PCSS_DirectionalLight(sampler2D shadowMap, vec3 shadowCoords, float bias, float uvLightSize, float shadowFade)
{
    float blockerDistance = FindBlockerDistance_DirectionalLight(shadowMap, shadowCoords, bias);
    if (blockerDistance == -1.0)
        return 1.0;

    float penumbraWidth = (shadowCoords.z - blockerDistance) / blockerDistance;

    float NEAR = 0.01;
    float uvRadius = penumbraWidth * uvLightSize * NEAR / shadowCoords.z;
    return 1.0 - PCF_DirectionalLight(shadowMap, shadowCoords, bias, uvRadius) * shadowFade;
}

float CascadeShadow(sampler2D shadowMap, vec3 worldPos, vec3 normal, int cascade, float shadowFade)
{
    vec4 lightSpacePos = Prism_ShadowMatrices[cascade] * vec4(worldPos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    float bias = GetShadowBias(normal, -Prism_Lights[0].Direction);
    bool softShadows = Prism_ShadowParams.w > 0.5;
    float lightSize = Prism_ShadowData.x;

    if (softShadows)
        return PCSS_DirectionalLight(shadowMap, projCoords, bias, lightSize, shadowFade);
    else
        return HardShadows_DirectionalLight(shadowMap, projCoords, bias, shadowFade);
}

float Prism_GetShadow(vec3 worldPos, float viewDepth, vec3 normal)
{
    int cascadeCount = min(int(Prism_ShadowParams.z), PRISM_MAX_CASCADES);

    // Cascade selection
    int cascadeIndex = 0;
    for (int i = 0; i < cascadeCount - 1; i++)
    {
        if (viewDepth > Prism_CascadeSplits[i])
            cascadeIndex = i + 1;
    }

    // Shadow distance fade
    float shadowDistance = Prism_ShadowData.y;
    float transitionDistance = Prism_ShadowData.z;
    float dist = abs(viewDepth);
    float shadowFade = clamp(1.0 - (dist - (shadowDistance - transitionDistance)) / transitionDistance, 0.0, 1.0);

    bool fadeCascades = Prism_ShadowData.w > 0.5;

    if (fadeCascades)
    {
        float cascadeTransitionFade = 0.05;

        float c0 = smoothstep(Prism_CascadeSplits[0] + cascadeTransitionFade * 0.5, Prism_CascadeSplits[0] - cascadeTransitionFade * 0.5, viewDepth);
        float c1 = smoothstep(Prism_CascadeSplits[1] + cascadeTransitionFade * 0.5, Prism_CascadeSplits[1] - cascadeTransitionFade * 0.5, viewDepth);
        float c2 = smoothstep(Prism_CascadeSplits[2] + cascadeTransitionFade * 0.5, Prism_CascadeSplits[2] - cascadeTransitionFade * 0.5, viewDepth);

        if (c0 > 0.0 && c0 < 1.0)
        {
            float s0 = CascadeShadow(Prism_ShadowMap0, worldPos, normal, 0, shadowFade);
            float s1 = CascadeShadow(Prism_ShadowMap1, worldPos, normal, 1, shadowFade);
            return mix(s0, s1, c0);
        }
        else if (c1 > 0.0 && c1 < 1.0)
        {
            float s1 = CascadeShadow(Prism_ShadowMap1, worldPos, normal, 1, shadowFade);
            float s2 = CascadeShadow(Prism_ShadowMap2, worldPos, normal, 2, shadowFade);
            return mix(s1, s2, c1);
        }
        else if (c2 > 0.0 && c2 < 1.0)
        {
            float s2 = CascadeShadow(Prism_ShadowMap2, worldPos, normal, 2, shadowFade);
            float s3 = CascadeShadow(Prism_ShadowMap3, worldPos, normal, 3, shadowFade);
            return mix(s2, s3, c2);
        }
    }

    // Single cascade (no fade, or center of cascade)
    if (cascadeIndex == 0) return CascadeShadow(Prism_ShadowMap0, worldPos, normal, 0, shadowFade);
    if (cascadeIndex == 1) return CascadeShadow(Prism_ShadowMap1, worldPos, normal, 1, shadowFade);
    if (cascadeIndex == 2) return CascadeShadow(Prism_ShadowMap2, worldPos, normal, 2, shadowFade);
    return CascadeShadow(Prism_ShadowMap3, worldPos, normal, 3, shadowFade);
}

#endif
