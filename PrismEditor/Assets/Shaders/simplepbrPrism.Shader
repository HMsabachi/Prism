#version 450 core
#line 1 "Bindings.glsl"
#ifndef PRISM_BINDINGS
    #define PRISM_BINDINGS

    #define PRISM_BINDING_FRAME 0
    #define PRISM_BINDING_OBJECT 1
    #define PRISM_BINDING_MATERIAL 2

    #define PRISM_SHADOW_MAP0 3
    #define PRISM_SHADOW_MAP1 4
    #define PRISM_SHADOW_MAP2 5
    #define PRISM_SHADOW_MAP3 6

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

const int PRISM_MAX_LIGHTS = 1;
const int PRISM_MAX_CASCADES = 4;

layout(std140, binding = 0) uniform PrismFrame
{
    mat4 Prism_ViewProjection;
    mat4 Prism_InverseViewProjection;
    mat4 Prism_View;
    mat4 Prism_Projection;

    vec4 Prism_Time;
    vec3 Prism_CameraPosition;

    float Prism_DeltaTime;
    float Prism_AspectRatio;
    vec2 Prism_Resolution;

    Prism_Light Prism_Lights[PRISM_MAX_LIGHTS];

    mat4 Prism_ShadowMatrices[PRISM_MAX_CASCADES];
    vec4 Prism_CascadeSplits;
    vec4 Prism_ShadowParams;
};
#line 1 "PrismObject.glsl"
// PerObject UBO — binding=1

const int PRISM_MAX_BONES = 128;

layout(std140, binding = 1) uniform PrismObject
{
    mat4 Prism_Model;
    mat4 Prism_PreviousModel;
    mat4 Prism_Bones[PRISM_MAX_BONES];
};
#line 1 "PrismShadow.glsl"
// PrismShadow.glsl

layout(binding = 3) uniform sampler2DShadow Prism_ShadowMap0;
layout(binding = 4) uniform sampler2DShadow Prism_ShadowMap1;
layout(binding = 5) uniform sampler2DShadow Prism_ShadowMap2;
layout(binding = 6) uniform sampler2DShadow Prism_ShadowMap3;

const vec2 PRISM_POISSON_DISK[16] = vec2[](
vec2(0.130818, 0.331688), vec2(-0.261065, -0.005615),
vec2(-0.722590, -0.071435), vec2(-0.497219, 0.523423),
vec2(0.295304, -0.488956), vec2(-0.192958, -0.909489),
vec2(-0.618316, 0.755979), vec2(0.425002, -0.259947),
vec2(0.912014, 0.125262), vec2(-0.159244, 0.718788),
vec2(0.474572, 0.676472), vec2(0.113612, -0.777447),
vec2(0.736447, -0.475556), vec2(0.071983, -0.211430),
vec2(-0.335085, -0.523819), vec2(0.860069, 0.432796)
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
    for (int i = 0;
    i < 16; i++)
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
    for (int i = 0;
    i < cascadeCount - 1; i++)
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
layout(binding = 16) uniform sampler2D u_BRDFLUTTexture;
layout(binding = 17) uniform sampler2D u_AlbedoTexture;
layout(binding = 18) uniform sampler2D u_NormalTexture;
layout(binding = 19) uniform sampler2D u_MetalnessTexture;
layout(binding = 20) uniform sampler2D u_RoughnessTexture;
layout(binding = 21) uniform samplerCube u_EnvRadianceTex;
layout(binding = 22) uniform samplerCube u_EnvIrradianceTex;
layout(std140, binding = 2) uniform PrismMaterial
{
    vec3 u_Color;
    vec3 u_AlbedoColor;
    float u_Metalness;
    float u_Roughness;
    float u_EnvMapRotation;
    float u_RadiancePrefilter;
    vec3 u_LightDirection;
    vec3 u_LightRadiance;
};

#line 53 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 53 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 54 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 55 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 56 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 1 "PrismBuiltin.glsl"
#line 58 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 1 "PrismPBR.glsl"
const float PI = 3.141592;
const float Epsilon = 0.00001;
// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2
float ndfGGX(float cosLh, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;    // Epic suggests using this roughness remapping for analytic lights.
    return gaSchlickG1(cosLi, k) * gaSchlickG1(NdotV, k);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Shlick's approximation of the Fresnel factor.
vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(vec3 F0, float cosTheta, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// ---------------------------------------------------------------------------------------------------
// The following code (from Unreal Engine 4's paper) shows how to filter the environment map
// for different roughnesses. This is mean to be computed offline and stored in cube map mips,
// so turning this on online will cause poor performance
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e - 10f;   // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, float Roughness, vec3 N)
{
    float a = Roughness * Roughness;
    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt((1 - Xi.y) / (1 + (a * a - 1) * Xi.y) );
    float SinTheta = sqrt(1 - CosTheta * CosTheta);
    vec3 H;
    H.x = SinTheta * cos(Phi);
    H.y = SinTheta * sin(Phi);
    H.z = CosTheta;
    vec3 UpVector = abs(N.z) < 0.999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
    vec3 TangentX = normalize(cross(UpVector, N) );
    vec3 TangentY = cross(N, TangentX);
    // Tangent to world space
    return TangentX * H.x + TangentY * H.y + N * H.z;
}

vec3 PrefilterEnvMap(float Roughness, vec3 R)
{
    vec3 N = R;
    vec3 V = R;
    vec3 PrefilteredColor = vec3(0.0);
    int NumSamples = 1024;
    float TotalWeight = 0.0;
    for (int i = 0;
    i < NumSamples; i++)
    {
        vec2 Xi = Hammersley(i, NumSamples);
        vec3 H = ImportanceSampleGGX(Xi, Roughness, N);
        vec3 L = 2 * dot(V, H) * H - V;
        float NoL = clamp(dot(N, L), 0.0, 1.0);
        if (NoL > 0)
        {
            PrefilteredColor += texture(u_EnvRadianceTex, L).rgb * NoL;
            TotalWeight += NoL;
        }
    }
    return PrefilteredColor / TotalWeight;
}
#line 59 "Assets/Shaders/PrismPBR_Anim.Shader"
layout(location = 0) in vec3 a_Position;
#line 59 "Assets/Shaders/PrismPBR_Anim.Shader"
layout(location = 1) in vec3 a_Normal;
#line 60 "Assets/Shaders/PrismPBR_Anim.Shader"
layout(location = 2) in vec3 a_Tangent;
#line 61 "Assets/Shaders/PrismPBR_Anim.Shader"
layout(location = 3) in vec3 a_Binormal;
#line 62 "Assets/Shaders/PrismPBR_Anim.Shader"
layout(location = 4) in vec2 a_TexCoord;
#line 63 "Assets/Shaders/PrismPBR_Anim.Shader"
layout(location = 6) in ivec4 a_BoneIndices;
#line 64 "Assets/Shaders/PrismPBR_Anim.Shader"
layout(location = 7) in vec4 a_BoneWeights;
#line 65 "Assets/Shaders/PrismPBR_Anim.Shader"
const int MAX_BONES = 100;
uniform mat4 u_BoneTransforms[100];


out VertexOutput
{
    vec3 WorldPosition;
    vec3 Normal;
    vec2 TexCoord;
    mat3 WorldNormals;
    mat3 WorldTransform;
    vec3 Binormal;
}
vs_Output;
#line 71 "Assets/Shaders/PrismPBR_Anim.Shader"
const vec3 Fdielectric = vec3(0.04);

struct PBRParameters
{
    vec3 Albedo;
    float Roughness;
    float Metalness;

    vec3 Normal;
    vec3 View;
    float NdotV;
};

PBRParameters m_Params;

// ---------------------------------------------------------------------------------------------------

vec3 RotateVectorAboutY(float angle, vec3 vec)
{
    angle = radians(angle);
    mat3x3 rotationMatrix =
    {
        vec3(cos(angle), 0.0, sin(angle)),
        vec3(0.0, 1.0, 0.0),
        vec3(-sin(angle), 0.0, cos(angle))
    };
    return rotationMatrix * vec;
}

vec3 Lighting(vec3 F0)
{
    vec3 result = vec3(0.0);
    for (int i = 0;
    i < PRISM_MAX_LIGHTS; i++)
    {
        vec3 Li = -Prism_Lights[i].Direction;
        vec3 Lradiance = Prism_Lights[i].Radiance * Prism_Lights[i].Multiplier;
        vec3 Lh = normalize(Li + m_Params.View);

        // Calculate angles between surface normal and various light vectors.
        float cosLi = max(0.0, dot(m_Params.Normal, Li));
        float cosLh = max(0.0, dot(m_Params.Normal, Lh));

        vec3 F = fresnelSchlick(F0, max(0.0, dot(Lh, m_Params.View)));
        float D = ndfGGX(cosLh, m_Params.Roughness);
        float G = gaSchlickGGX(cosLi, m_Params.NdotV, m_Params.Roughness);

        vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
        vec3 diffuseBRDF = kd * m_Params.Albedo;

        // Cook-Torrance
        vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * m_Params.NdotV);

        result += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
    }

    float shadow = 1.0;
    if (Prism_ShadowParams.z > 0.0)
    {
        vec3 biasedPos = vs_Output.WorldPosition + m_Params.Normal * Prism_ShadowParams.y;
        shadow = Prism_GetShadow(biasedPos, -(Prism_View * vec4(vs_Output.WorldPosition, 1.0)).z);
    }
    result *= shadow;

    return result;
}

vec3 IBL(vec3 F0, vec3 Lr)
{
    vec3 irradiance = texture(u_EnvIrradianceTex, m_Params.Normal).rgb;
    vec3 F = fresnelSchlickRoughness(F0, m_Params.NdotV, m_Params.Roughness);
    vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
    vec3 diffuseIBL = m_Params.Albedo * irradiance;

    int u_EnvRadianceTexLevels = textureQueryLevels(u_EnvRadianceTex);
    float NoV = clamp(m_Params.NdotV, 0.0, 1.0);
    vec3 R = 2.0 * dot(m_Params.View, m_Params.Normal) * m_Params.Normal - m_Params.View;
    vec3 specularIrradiance = vec3(0.0);

    if (u_RadiancePrefilter > 0.5)
        specularIrradiance = PrefilterEnvMap(m_Params.Roughness * m_Params.Roughness, R) * u_RadiancePrefilter;
    else
        specularIrradiance = textureLod(u_EnvRadianceTex, RotateVectorAboutY(u_EnvMapRotation, Lr), sqrt(m_Params.Roughness) * u_EnvRadianceTexLevels).rgb * (1.0 - u_RadiancePrefilter);

    // Sample BRDF Lut, 1.0 - roughness for y-coord because texture was generated (in Sparky) for gloss model
    vec2 specularBRDF = texture(u_BRDFLUTTexture, vec2(m_Params.NdotV, 1.0 - m_Params.Roughness)).rg;
    vec3 specularIBL = specularIrradiance * (F * specularBRDF.x + specularBRDF.y);

    return kd * diffuseIBL + specularIBL;
}

#line 80 "Assets/Shaders/PrismPBR_Anim.Shader"
void main()
{
    mat4 u_ViewProjectionMatrix = Prism_ViewProjection;
    mat4 u_ModelMatrix = Prism_Model;
    mat4 boneTransform = u_BoneTransforms[a_BoneIndices[0]] * a_BoneWeights[0];
    boneTransform += u_BoneTransforms[a_BoneIndices[1]] * a_BoneWeights[1];
    boneTransform += u_BoneTransforms[a_BoneIndices[2]] * a_BoneWeights[2];
    boneTransform += u_BoneTransforms[a_BoneIndices[3]] * a_BoneWeights[3];

    vec4 localPosition = boneTransform * vec4(a_Position, 1.0);

    vs_Output.WorldPosition = vec3(u_ModelMatrix * boneTransform * vec4(a_Position, 1.0));
    vs_Output.Normal = mat3(u_ModelMatrix) * mat3(boneTransform) * a_Normal;


    vs_Output.TexCoord = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);
    vs_Output.WorldNormals = mat3(u_ModelMatrix) * mat3(a_Tangent, a_Binormal, a_Normal);
    vs_Output.Binormal = mat3(boneTransform) * a_Binormal;
    vs_Output.WorldTransform = mat3(u_ModelMatrix);

    //gl_Position = u_ViewProjectionMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
    gl_Position = u_ViewProjectionMatrix * u_ModelMatrix * localPosition;
}

[23 : 23 : 11] PRISM : Source :
#version 450 core
layout(location = 0) out vec4 FragColor;
#line 1 "Bindings.glsl"
#ifndef PRISM_BINDINGS
    #define PRISM_BINDINGS

    #define PRISM_BINDING_FRAME 0
    #define PRISM_BINDING_OBJECT 1
    #define PRISM_BINDING_MATERIAL 2

    #define PRISM_SHADOW_MAP0 3
    #define PRISM_SHADOW_MAP1 4
    #define PRISM_SHADOW_MAP2 5
    #define PRISM_SHADOW_MAP3 6

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

const int PRISM_MAX_LIGHTS = 1;
const int PRISM_MAX_CASCADES = 4;

layout(std140, binding = 0) uniform PrismFrame
{
    mat4 Prism_ViewProjection;
    mat4 Prism_InverseViewProjection;
    mat4 Prism_View;
    mat4 Prism_Projection;

    vec4 Prism_Time;
    vec3 Prism_CameraPosition;

    float Prism_DeltaTime;
    float Prism_AspectRatio;
    vec2 Prism_Resolution;

    Prism_Light Prism_Lights[PRISM_MAX_LIGHTS];

    mat4 Prism_ShadowMatrices[PRISM_MAX_CASCADES];
    vec4 Prism_CascadeSplits;
    vec4 Prism_ShadowParams;
};
#line 1 "PrismObject.glsl"
// PerObject UBO — binding=1

const int PRISM_MAX_BONES = 128;

layout(std140, binding = 1) uniform PrismObject
{
    mat4 Prism_Model;
    mat4 Prism_PreviousModel;
    mat4 Prism_Bones[PRISM_MAX_BONES];
};
#line 1 "PrismShadow.glsl"
// PrismShadow.glsl

layout(binding = 3) uniform sampler2DShadow Prism_ShadowMap0;
layout(binding = 4) uniform sampler2DShadow Prism_ShadowMap1;
layout(binding = 5) uniform sampler2DShadow Prism_ShadowMap2;
layout(binding = 6) uniform sampler2DShadow Prism_ShadowMap3;

const vec2 PRISM_POISSON_DISK[16] = vec2[](
vec2(0.130818, 0.331688), vec2(-0.261065, -0.005615),
vec2(-0.722590, -0.071435), vec2(-0.497219, 0.523423),
vec2(0.295304, -0.488956), vec2(-0.192958, -0.909489),
vec2(-0.618316, 0.755979), vec2(0.425002, -0.259947),
vec2(0.912014, 0.125262), vec2(-0.159244, 0.718788),
vec2(0.474572, 0.676472), vec2(0.113612, -0.777447),
vec2(0.736447, -0.475556), vec2(0.071983, -0.211430),
vec2(-0.335085, -0.523819), vec2(0.860069, 0.432796)
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
    for (int i = 0;
    i < 16; i++)
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
    for (int i = 0;
    i < cascadeCount - 1; i++)
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
layout(binding = 16) uniform sampler2D u_BRDFLUTTexture;
layout(binding = 17) uniform sampler2D u_AlbedoTexture;
layout(binding = 18) uniform sampler2D u_NormalTexture;
layout(binding = 19) uniform sampler2D u_MetalnessTexture;
layout(binding = 20) uniform sampler2D u_RoughnessTexture;
layout(binding = 21) uniform samplerCube u_EnvRadianceTex;
layout(binding = 22) uniform samplerCube u_EnvIrradianceTex;
layout(std140, binding = 2) uniform PrismMaterial
{
    vec3 u_Color;
    vec3 u_AlbedoColor;
    float u_Metalness;
    float u_Roughness;
    float u_EnvMapRotation;
    float u_RadiancePrefilter;
    vec3 u_LightDirection;
    vec3 u_LightRadiance;
};

#line 53 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 53 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 54 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 55 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 56 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 1 "PrismBuiltin.glsl"
#line 58 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 1 "PrismPBR.glsl"
const float PI = 3.141592;
const float Epsilon = 0.00001;
// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2
float ndfGGX(float cosLh, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;    // Epic suggests using this roughness remapping for analytic lights.
    return gaSchlickG1(cosLi, k) * gaSchlickG1(NdotV, k);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Shlick's approximation of the Fresnel factor.
vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(vec3 F0, float cosTheta, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// ---------------------------------------------------------------------------------------------------
// The following code (from Unreal Engine 4's paper) shows how to filter the environment map
// for different roughnesses. This is mean to be computed offline and stored in cube map mips,
// so turning this on online will cause poor performance
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e - 10f;   // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, float Roughness, vec3 N)
{
    float a = Roughness * Roughness;
    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt((1 - Xi.y) / (1 + (a * a - 1) * Xi.y) );
    float SinTheta = sqrt(1 - CosTheta * CosTheta);
    vec3 H;
    H.x = SinTheta * cos(Phi);
    H.y = SinTheta * sin(Phi);
    H.z = CosTheta;
    vec3 UpVector = abs(N.z) < 0.999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
    vec3 TangentX = normalize(cross(UpVector, N) );
    vec3 TangentY = cross(N, TangentX);
    // Tangent to world space
    return TangentX * H.x + TangentY * H.y + N * H.z;
}

vec3 PrefilterEnvMap(float Roughness, vec3 R)
{
    vec3 N = R;
    vec3 V = R;
    vec3 PrefilteredColor = vec3(0.0);
    int NumSamples = 1024;
    float TotalWeight = 0.0;
    for (int i = 0;
    i < NumSamples; i++)
    {
        vec2 Xi = Hammersley(i, NumSamples);
        vec3 H = ImportanceSampleGGX(Xi, Roughness, N);
        vec3 L = 2 * dot(V, H) * H - V;
        float NoL = clamp(dot(N, L), 0.0, 1.0);
        if (NoL > 0)
        {
            PrefilteredColor += texture(u_EnvRadianceTex, L).rgb * NoL;
            TotalWeight += NoL;
        }
    }
    return PrefilteredColor / TotalWeight;
}
#line 59 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 59 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 60 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 61 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 62 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 63 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 64 "Assets/Shaders/PrismPBR_Anim.Shader"
#line 65 "Assets/Shaders/PrismPBR_Anim.Shader"
const int MAX_BONES = 100;
uniform mat4 u_BoneTransforms[100];


in VertexOutput
{
    vec3 WorldPosition;
    vec3 Normal;
    vec2 TexCoord;
    mat3 WorldNormals;
    mat3 WorldTransform;
    vec3 Binormal;
}
vs_Output;
#line 71 "Assets/Shaders/PrismPBR_Anim.Shader"
const vec3 Fdielectric = vec3(0.04);

struct PBRParameters
{
    vec3 Albedo;
    float Roughness;
    float Metalness;

    vec3 Normal;
    vec3 View;
    float NdotV;
};

PBRParameters m_Params;

// ---------------------------------------------------------------------------------------------------

vec3 RotateVectorAboutY(float angle, vec3 vec)
{
    angle = radians(angle);
    mat3x3 rotationMatrix =
    {
        vec3(cos(angle), 0.0, sin(angle)),
        vec3(0.0, 1.0, 0.0),
        vec3(-sin(angle), 0.0, cos(angle))
    };
    return rotationMatrix * vec;
}

vec3 Lighting(vec3 F0)
{
    vec3 result = vec3(0.0);
    for (int i = 0;
    i < PRISM_MAX_LIGHTS; i++)
    {
        vec3 Li = -Prism_Lights[i].Direction;
        vec3 Lradiance = Prism_Lights[i].Radiance * Prism_Lights[i].Multiplier;
        vec3 Lh = normalize(Li + m_Params.View);

        // Calculate angles between surface normal and various light vectors.
        float cosLi = max(0.0, dot(m_Params.Normal, Li));
        float cosLh = max(0.0, dot(m_Params.Normal, Lh));

        vec3 F = fresnelSchlick(F0, max(0.0, dot(Lh, m_Params.View)));
        float D = ndfGGX(cosLh, m_Params.Roughness);
        float G = gaSchlickGGX(cosLi, m_Params.NdotV, m_Params.Roughness);

        vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
        vec3 diffuseBRDF = kd * m_Params.Albedo;

        // Cook-Torrance
        vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * m_Params.NdotV);

        result += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
    }

    float shadow = 1.0;
    if (Prism_ShadowParams.z > 0.0)
    {
        vec3 biasedPos = vs_Output.WorldPosition + m_Params.Normal * Prism_ShadowParams.y;
        shadow = Prism_GetShadow(biasedPos, -(Prism_View * vec4(vs_Output.WorldPosition, 1.0)).z);
    }
    result *= shadow;

    return result;
}

vec3 IBL(vec3 F0, vec3 Lr)
{
    vec3 irradiance = texture(u_EnvIrradianceTex, m_Params.Normal).rgb;
    vec3 F = fresnelSchlickRoughness(F0, m_Params.NdotV, m_Params.Roughness);
    vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
    vec3 diffuseIBL = m_Params.Albedo * irradiance;

    int u_EnvRadianceTexLevels = textureQueryLevels(u_EnvRadianceTex);
    float NoV = clamp(m_Params.NdotV, 0.0, 1.0);
    vec3 R = 2.0 * dot(m_Params.View, m_Params.Normal) * m_Params.Normal - m_Params.View;
    vec3 specularIrradiance = vec3(0.0);

    if (u_RadiancePrefilter > 0.5)
        specularIrradiance = PrefilterEnvMap(m_Params.Roughness * m_Params.Roughness, R) * u_RadiancePrefilter;
    else
        specularIrradiance = textureLod(u_EnvRadianceTex, RotateVectorAboutY(u_EnvMapRotation, Lr), sqrt(m_Params.Roughness) * u_EnvRadianceTexLevels).rgb * (1.0 - u_RadiancePrefilter);

    // Sample BRDF Lut, 1.0 - roughness for y-coord because texture was generated (in Sparky) for gloss model
    vec2 specularBRDF = texture(u_BRDFLUTTexture, vec2(m_Params.NdotV, 1.0 - m_Params.Roughness)).rg;
    vec3 specularIBL = specularIrradiance * (F * specularBRDF.x + specularBRDF.y);

    return kd * diffuseIBL + specularIBL;
}

#line 192 "Assets/Shaders/PrismPBR_Anim.Shader"
void main()
{
    vec3 u_CameraPosition = Prism_CameraPosition;
    // Standard PBR inputs
    #ifdef ALBEDO_MAP
        m_Params.Albedo = texture(u_AlbedoTexture, vs_Output.TexCoord).rgb;
    #else
            m_Params.Albedo = u_AlbedoColor;
    #endif
    #ifdef METALNESS_MAP
        m_Params.Metalness = texture(u_MetalnessTexture, vs_Output.TexCoord).r;
    #else
            m_Params.Metalness = u_Metalness;
    #endif
    #ifdef ROUGHNESS_MAP
        m_Params.Roughness = texture(u_RoughnessTexture, vs_Output.TexCoord).r;
    #else
            m_Params.Roughness = u_Roughness;
    #endif
    m_Params.Roughness = max(m_Params.Roughness, 0.05);   // Minimum roughness of 0.05 to keep specular highlight

    m_Params.Albedo = mix(m_Params.Albedo, u_Color, 0.2);
    // Normals (either from vertex or map)
    #ifdef NORMAL_MAP
        m_Params.Normal = normalize(2.0 * texture(u_NormalTexture, vs_Output.TexCoord).rgb - 1.0);
        m_Params.Normal = normalize(vs_Output.WorldNormals * m_Params.Normal);
    #else
            m_Params.Normal = normalize(vs_Output.Normal);
    #endif

    m_Params.View = normalize(u_CameraPosition - vs_Output.WorldPosition);
    m_Params.NdotV = max(dot(m_Params.Normal, m_Params.View), 0.0);

    // Specular reflection vector
    vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;

    // Fresnel reflectance, metals use albedo
    vec3 F0 = mix(Fdielectric, m_Params.Albedo, m_Params.Metalness);

    vec3 lightContribution = Lighting(F0);
    vec3 iblContribution = IBL(F0, Lr);

    FragColor = vec4(lightContribution + iblContribution, 1.0);
    //FragColor = vec4(u_Color, 1.0);
}
