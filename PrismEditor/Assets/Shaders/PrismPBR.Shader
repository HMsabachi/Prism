// Prism Shader Language v1.0
Shader "Standard/PrismPBR"
{
    Properties
    {
        u_Color("主色", Color3) = (1, 1, 1)
        u_AlbedoColor("颜色", Color3) = (1, 1, 1)
        u_Metalness("金属度", Range(0, 1)) = 1.0
        u_Roughness("粗糙度", Range(0, 1)) = 0.0
        u_EnvMapRotation("环境光旋转", Range(0, 360)) = 0
        // PBR texture inputs
        u_AlbedoTexture("颜色贴图", Texture2D) = {}
        u_NormalTexture("法线贴图", Texture2D) = {}
        u_MetalnessTexture("金属度贴图", Texture2D) = {}
        u_RoughnessTexture("粗糙度贴图", Texture2D) = {}
    }

    RenderCommand
    {
        Cull Back
        ZTest LEqual
        ZWrite On
        Blend One OneMinusSrcAlpha
    }

    SubShader
    {

        Pass
        {
            Tags { "Queue" = "Geometry" "RenderType" = "Opaque" "LightMode" = "ForwardBase" }
            Name "ForwardBase"
            GLSL
            {
                #pragma shader_feature SKINNED SHOW_CASCADES
                #pragma shader_feature ALBEDO_MAP NORMAL_MAP METALNESS_MAP ROUGHNESS_MAP
                #include "PrismEngine.glsl"
                #include "Pass/GeometryPass.glsl"
                #include "PrismShadow.glsl"
                attribute vec3 a_Position : POSITION;
                attribute vec3 a_Normal : NORMAL;
                attribute vec3 a_Tangent : TANGENT;
                attribute vec3 a_Binormal : BINORMAL;
                attribute vec2 a_TexCoord : TEXCOORD0;
            #ifdef SKINNED
                attribute ivec4 a_BoneIndices : BONEINDICES;
                attribute vec4 a_BoneWeights : BONEWEIGHTS;
            #endif

                varying VertexOutput
                {
                    vec3 WorldPosition;
                    vec3 Normal;
                    vec2 TexCoord;
                    mat3 WorldNormals;
                    mat3 WorldTransform;
                    vec3 Binormal;
                    vec4 ShadowMapCoords[4];
                    vec3 ViewPosition;
                } vs_Output;

                void vert()
                {
                    mat4 u_ViewProjectionMatrix = Prism_ViewProjection;
                    mat4 u_ModelMatrix = Prism_Model;
                #ifdef SKINNED
                    mat4 boneTransform = Prism_Bones[a_BoneIndices[0]] * a_BoneWeights[0];
                    boneTransform += Prism_Bones[a_BoneIndices[1]] * a_BoneWeights[1];
                    boneTransform += Prism_Bones[a_BoneIndices[2]] * a_BoneWeights[2];
                    boneTransform += Prism_Bones[a_BoneIndices[3]] * a_BoneWeights[3];

                    vec4 localPosition = boneTransform * vec4(a_Position, 1.0);

                    vs_Output.WorldPosition = vec3(u_ModelMatrix * boneTransform * vec4(a_Position, 1.0));
                    vs_Output.Normal = mat3(u_ModelMatrix) * mat3(boneTransform) * a_Normal;
                    vs_Output.Binormal = mat3(boneTransform) * a_Binormal;
                #else
                    vs_Output.WorldPosition = vec3(u_ModelMatrix * vec4(a_Position, 1.0));
                    vs_Output.Normal = mat3(u_ModelMatrix) * a_Normal;
                    vs_Output.Binormal = a_Binormal;
                #endif
                    vs_Output.TexCoord = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);
                    vs_Output.WorldNormals = mat3(u_ModelMatrix) * mat3(a_Tangent, a_Binormal, a_Normal);
                    vs_Output.WorldTransform = mat3(u_ModelMatrix);

                    vs_Output.ShadowMapCoords[0] = Prism_ShadowMatrices[0] * vec4(vs_Output.WorldPosition, 1.0);
                    vs_Output.ShadowMapCoords[1] = Prism_ShadowMatrices[1] * vec4(vs_Output.WorldPosition, 1.0);
                    vs_Output.ShadowMapCoords[2] = Prism_ShadowMatrices[2] * vec4(vs_Output.WorldPosition, 1.0);
                    vs_Output.ShadowMapCoords[3] = Prism_ShadowMatrices[3] * vec4(vs_Output.WorldPosition, 1.0);
                    vs_Output.ViewPosition = vec3(Prism_View * vec4(vs_Output.WorldPosition, 1.0));

                #ifdef SKINNED
                    gl_Position = u_ViewProjectionMatrix * u_ModelMatrix * localPosition;
                #else
                    gl_Position = u_ViewProjectionMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
                #endif
                }

                const vec3 Fdielectric = vec3(0.04);
                const float Epsilon = 0.00001;

                // GGX/Towbridge-Reitz normal distribution function.
                float ndfGGX(float cosLh, float roughness)
                {
                    float alpha = roughness * roughness;
                    float alphaSq = alpha * alpha;
                    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
                    return alphaSq / (3.141592 * denom * denom);
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
                    float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
                    return gaSchlickG1(cosLi, k) * gaSchlickG1(NdotV, k);
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
                
                vec3 RotateVectorAboutY(float angle, vec3 vec)
                {
                    angle = radians(angle);
                    mat3x3 rotationMatrix ={vec3(cos(angle),0.0,sin(angle)),
                                            vec3(0.0,1.0,0.0),
                                            vec3(-sin(angle),0.0,cos(angle))};
                    return rotationMatrix * vec;
                }

                vec3 Lighting(vec3 F0)
                {
                    vec3 result = vec3(0.0);
                    for(int i = 0; i < PRISM_MAX_LIGHTS; i++)
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
                        shadow = Prism_GetShadow(biasedPos, -vs_Output.ViewPosition.z, m_Params.Normal);
                    }
                    result *= shadow;

                    return result;
                }

                vec3 IBL(vec3 F0, vec3 Lr)
                {
                    vec3 irradiance = texture(Prism_EnvIrradianceTex, m_Params.Normal).rgb;
                    vec3 F = fresnelSchlickRoughness(F0, m_Params.NdotV, m_Params.Roughness);
                    vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
                    vec3 diffuseIBL = m_Params.Albedo * irradiance;

                    int u_EnvRadianceTexLevels = textureQueryLevels(Prism_EnvRadianceTex);
                    vec3 specularIrradiance = textureLod(Prism_EnvRadianceTex, RotateVectorAboutY(u_EnvMapRotation, Lr), (m_Params.Roughness * m_Params.Roughness) * u_EnvRadianceTexLevels).rgb;

                    // Sample BRDF Lut, 1.0 - roughness for y-coord because texture was generated (in Sparky) for gloss model
                    vec2 specularBRDF = texture(Prism_BRDFLUT, vec2(m_Params.NdotV, 1.0 - m_Params.Roughness)).rg;
                    vec3 specularIBL = specularIrradiance * (F * specularBRDF.x + specularBRDF.y);

                    return kd * diffuseIBL + specularIBL;
                }

                void frag()
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
                    m_Params.Roughness = max(m_Params.Roughness, 0.05); // Minimum roughness of 0.05 to keep specular highlight

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

                    // ShowCascades debug — uncomment #define SHOW_CASCADES above to enable
                #ifdef SHOW_CASCADES
                    if (Prism_ShadowParams.z > 0.0)
                    {
                        int cascadeIdx = 0;
                        for (int i = 0; i < min(int(Prism_ShadowParams.z), PRISM_MAX_CASCADES) - 1; i++)
                            if (-vs_Output.ViewPosition.z > Prism_CascadeSplits[i])
                                cascadeIdx = i + 1;
                        if (cascadeIdx == 0) FragColor.rgb *= vec3(1.0, 0.25, 0.25);
                        if (cascadeIdx == 1) FragColor.rgb *= vec3(0.25, 1.0, 0.25);
                        if (cascadeIdx == 2) FragColor.rgb *= vec3(0.25, 0.25, 1.0);
                        if (cascadeIdx == 3) FragColor.rgb *= vec3(1.0, 1.0, 0.25);
                    }
                #endif

                    // Bloom
                    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
                    BloomColor = brightness > 1.0 ? FragColor : vec4(0.0, 0.0, 0.0, 1.0);
                }

            }
        }

        Pass
        {
            Tags { "LightMode" = "ShadowCaster" }
            Name "ShadowCaster"
            RenderCommand { Cull Back ZWrite On ZTest LEqual }
            GLSL
            {
                #pragma shader_feature SKINNED
                #include "PrismEngine.glsl"
                #include "Pass/ShadowPass.glsl"

                attribute vec3 a_Position : POSITION;
            #ifdef SKINNED
                attribute ivec4 a_BoneIndices : BONEINDICES;
                attribute vec4 a_BoneWeights : BONEWEIGHTS;
            #endif

                void vert()
                {
                #ifdef SKINNED
                    mat4 boneTransform = Prism_Bones[a_BoneIndices[0]] * a_BoneWeights[0];
                    boneTransform += Prism_Bones[a_BoneIndices[1]] * a_BoneWeights[1];
                    boneTransform += Prism_Bones[a_BoneIndices[2]] * a_BoneWeights[2];
                    boneTransform += Prism_Bones[a_BoneIndices[3]] * a_BoneWeights[3];
                    vec4 localPosition = boneTransform * vec4(a_Position, 1.0);
                    gl_Position = Prism_ShadowMatrices[int(Prism_ObjectReserved.x)] * Prism_Model * localPosition;
                #else
                    gl_Position = Prism_ShadowMatrices[int(Prism_ObjectReserved.x)] * Prism_Model * vec4(a_Position, 1.0);
                #endif
                }

                void frag()
                {
                }
            }
        }
    }
}
