// PrismShadow.glsl 

layout(binding = PRISM_SHADOW_MAP0) uniform sampler2DShadow Prism_ShadowMap0;
layout(binding = PRISM_SHADOW_MAP1) uniform sampler2DShadow Prism_ShadowMap1;
layout(binding = PRISM_SHADOW_MAP2) uniform sampler2DShadow Prism_ShadowMap2;
layout(binding = PRISM_SHADOW_MAP3) uniform sampler2DShadow Prism_ShadowMap3;

const vec2 PRISM_POISSON_DISK[16] = vec2[](
    vec2( 0.130818,  0.331688), vec2(-0.261065, -0.005615),
    vec2(-0.722590, -0.071435), vec2(-0.497219,  0.523423),
    vec2( 0.295304, -0.488956), vec2(-0.192958, -0.909489),
    vec2(-0.618316,  0.755979), vec2( 0.425002, -0.259947),
    vec2( 0.912014,  0.125262), vec2(-0.159244,  0.718788),
    vec2( 0.474572,  0.676472), vec2( 0.113612, -0.777447),
    vec2( 0.736447, -0.475556), vec2( 0.071983, -0.211430),
    vec2(-0.335085, -0.523819), vec2( 0.860069,  0.432796)
);

float Prism_Hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float Prism_ShadowSample(sampler2DShadow shadowMap, vec3 projCoords, float bias, vec2 texelSize, float filterRadius)
{
    float angle = Prism_Hash(projCoords.xy) * 3.14159265;
    float s = sin(angle), c = cos(angle);
    mat2 rot = mat2(c, -s, s, c);

    float shadow = 0.0;
    for (int i = 0; i < 16; i++)
    {
        vec2 offset = rot * PRISM_POISSON_DISK[i] * texelSize * filterRadius;
        shadow += texture(shadowMap, vec3(projCoords.xy + offset, projCoords.z - bias));
    }
    return shadow * (1.0 / 16.0);
}

float Prism_GetShadow(vec3 worldPos, float viewDepth)
{
    int cascade = 0;
    int cascadeCount = min(int(Prism_ShadowParams.z), PRISM_MAX_CASCADES);
    for (int i = 0; i < cascadeCount - 1; i++)
    {
        if (viewDepth > Prism_CascadeSplits[i])
            cascade = i + 1;
    }

    vec4 lightSpacePos = Prism_ShadowMatrices[cascade] * vec4(worldPos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    float bias = Prism_ShadowParams.x;
    vec2 texelSize = 1.0 / vec2(textureSize(Prism_ShadowMap0, 0));

    if (cascade == 0) return Prism_ShadowSample(Prism_ShadowMap0, projCoords, bias, texelSize, 1.5);
    if (cascade == 1) return Prism_ShadowSample(Prism_ShadowMap1, projCoords, bias, texelSize, 1.5);
    if (cascade == 2) return Prism_ShadowSample(Prism_ShadowMap2, projCoords, bias, texelSize, 1.5);
    return Prism_ShadowSample(Prism_ShadowMap3, projCoords, bias, texelSize, 1.5);
}
