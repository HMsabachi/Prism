// Assets/Shaders/Collider.Shader
#version 450 core
#line 1 "Bindings.glsl"
#ifndef PRISM_BINDINGS
#define PRISM_BINDINGS

#define PRISM_BINDING_FRAME    0
#define PRISM_BINDING_OBJECT   1
#define PRISM_BINDING_MATERIAL 2

#define PRISM_SHADOW_MAP0   3
#define PRISM_SHADOW_MAP1   4
#define PRISM_SHADOW_MAP2   5
#define PRISM_SHADOW_MAP3   6

#define PRISM_GEOMETRY_PASS_TEXTURE 7

#define PRISM_BINDING_TEXTURE 16

#endif
#line 1 "PrismFrame.glsl"
// PerFrame UBO — binding=0
struct Prism_Light
{
    vec3 Direction;
    vec3 Radiance;
    float Multiplier;
};

const int PRISM_MAX_LIGHTS   = 1;
const int PRISM_MAX_CASCADES = 4;

layout(std140, binding = PRISM_BINDING_FRAME) uniform PrismFrame
{
    mat4 Prism_ViewProjection;
    mat4 Prism_InverseViewProjection;
    mat4 Prism_View;
    mat4 Prism_Projection;

    vec4 Prism_Time;
    vec3 Prism_CameraPosition;

    float Prism_DeltaTime;
    float Prism_AspectRatio;
    vec2  Prism_Resolution;

    Prism_Light Prism_Lights[PRISM_MAX_LIGHTS];

    mat4 Prism_ShadowMatrices[PRISM_MAX_CASCADES];
    vec4 Prism_CascadeSplits;
    vec4 Prism_ShadowParams;
};
#line 1 "PrismObject.glsl"
// PerObject UBO — binding=1

const int PRISM_MAX_BONES = 128;

layout(std140, binding = PRISM_BINDING_OBJECT) uniform PrismObject
{
    mat4 Prism_Model;
    mat4 Prism_PreviousModel;
    mat4 Prism_Bones[PRISM_MAX_BONES];
};
#line 1 "PrismShadow.glsl"
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
layout(std140, binding = 2) uniform PrismMaterial
{
    vec4 u_Color;
};

#line 25 "Assets/Shaders/Collider.Shader"
#line 1 "PrismBuiltin.glsl"
//

#line 27 "Assets/Shaders/Collider.Shader"
layout(location = 0) in vec3 a_Position;
#line 27 "Assets/Shaders/Collider.Shader"
#line 28 "Assets/Shaders/Collider.Shader"
void main()
{
                    gl_Position = Prism_ViewProjection * Prism_Model * vec4(a_Position, 1.0);
                }

[23:57:29] PRISM: Source:
// Assets/Shaders/Collider.Shader
#version 450 core
layout(location = 0) out vec4 FragColor;
#line 1 "Bindings.glsl"
#ifndef PRISM_BINDINGS
#define PRISM_BINDINGS

#define PRISM_BINDING_FRAME    0
#define PRISM_BINDING_OBJECT   1
#define PRISM_BINDING_MATERIAL 2

#define PRISM_SHADOW_MAP0   3
#define PRISM_SHADOW_MAP1   4
#define PRISM_SHADOW_MAP2   5
#define PRISM_SHADOW_MAP3   6

#define PRISM_GEOMETRY_PASS_TEXTURE 7

#define PRISM_BINDING_TEXTURE 16

#endif
#line 1 "PrismFrame.glsl"
// PerFrame UBO — binding=0
struct Prism_Light
{
    vec3 Direction;
    vec3 Radiance;
    float Multiplier;
};

const int PRISM_MAX_LIGHTS   = 1;
const int PRISM_MAX_CASCADES = 4;

layout(std140, binding = PRISM_BINDING_FRAME) uniform PrismFrame
{
    mat4 Prism_ViewProjection;
    mat4 Prism_InverseViewProjection;
    mat4 Prism_View;
    mat4 Prism_Projection;

    vec4 Prism_Time;
    vec3 Prism_CameraPosition;

    float Prism_DeltaTime;
    float Prism_AspectRatio;
    vec2  Prism_Resolution;

    Prism_Light Prism_Lights[PRISM_MAX_LIGHTS];

    mat4 Prism_ShadowMatrices[PRISM_MAX_CASCADES];
    vec4 Prism_CascadeSplits;
    vec4 Prism_ShadowParams;
};
#line 1 "PrismObject.glsl"
// PerObject UBO — binding=1

const int PRISM_MAX_BONES = 128;

layout(std140, binding = PRISM_BINDING_OBJECT) uniform PrismObject
{
    mat4 Prism_Model;
    mat4 Prism_PreviousModel;
    mat4 Prism_Bones[PRISM_MAX_BONES];
};
#line 1 "PrismShadow.glsl"
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
layout(std140, binding = 2) uniform PrismMaterial
{
    vec4 u_Color;
};

#line 25 "Assets/Shaders/Collider.Shader"
#line 1 "PrismBuiltin.glsl"
//

#line 27 "Assets/Shaders/Collider.Shader"
#line 27 "Assets/Shaders/Collider.Shader"
#line 33 "Assets/Shaders/Collider.Shader"
void main()
{
                    FragColor = u_Color;
                }