Shader "Custom/Toon"
{
    Properties
    {
        u_AlbedoColor("固有色", Color3) = (1.0, 1.0, 1.0)
        u_AlbedoTexture("固有色贴图", Texture2D) = {}
        u_NormalTexture("法线贴图", Texture2D) = {}
        u_AlbedoTextureTiling("贴图缩放", Vector2) = (1.0, 1.0)
        u_AlbedoTextureOffset("贴图偏移", Vector2) = (0.0, 0.0)

        u_ShadowThreshold("阴影阈值", Range(0, 1)) = 0.45
        u_ShadowSmooth("阴影柔化", Range(0.001, 0.5)) = 0.05
        u_ShadowColor("阴影色", Color) = (0.35, 0.45, 0.65, 1.0)
        u_ShadowBrightness("阴影亮度", Range(0, 1)) = 0.6
        u_HighlightThreshold("高光阈值", Range(0, 1)) = 0.75
        u_HighlightIntensity("高光强度", Range(1.0, 2.0)) = 1.2

        u_SpecularColor("高光色", Color) = (1.0, 1.0, 1.0, 1.0)
        u_SpecularIntensity("高光强度", Range(0, 5)) = 1.5
        u_SpecularThreshold("高光阈值", Range(0, 1)) = 0.35
        u_Shininess("光泽度", Range(1, 200)) = 30.0
        u_SpecularWidth("高光宽度", Range(0.01, 1.0)) = 0.15

        u_RimColor("边缘光色", Color) = (1.0, 0.6, 0.3, 1.0)
        u_RimIntensity("边缘光强度", Range(0, 5)) = 1.2
        u_RimPower("边缘光范围", Range(0.1, 10)) = 3.0
        u_RimSmooth("边缘光柔化", Range(0.001, 0.5)) = 0.05

        u_OutlineColor("描边色", Color) = (0.05, 0.05, 0.1, 1.0)
        u_OutlineWidth("描边宽度", Float) = 0.02
    }

    RenderCommand
    {
        Cull Back
        ZTest LEqual
        ZWrite On
        Blend SrcAlpha OneMinusSrcAlpha
    }

    SubShader
    {
        Pass
        {
            Tags { "Queue" = "Geometry" "RenderType" = "Opaque" "LightMode" = "ForwardBase" }
            Name "ForwardBase"
            GLSL
            {
                #pragma shader_feature SKINNED NORMAL_MAP ALBEDO_MAP
                #include "PrismCommon.glsl"

                attribute vec3 a_Position : POSITION;
                attribute vec3 a_Normal : NORMAL;
                attribute vec3 a_Tangent : TANGENT;
                attribute vec3 a_Binormal : BINORMAL;
                attribute vec2 a_TexCoord : TEXCOORD0;
            #ifdef SKINNED
                attribute ivec4 a_BoneIndices : BONEINDICES;
                attribute vec4 a_BoneWeights : BONEWEIGHTS;
            #endif

                layout(location = 0) out vec4 FragColor;

                varying VertexOutput
                {
                    vec3 WorldPosition;
                    vec3 Normal;
                    vec2 TexCoord;
                    mat3 WorldNormals;
                    vec4 ShadowMapCoords[4];
                    vec3 ViewPosition;
                } vs_Output;

                void vert()
                {
                #ifdef SKINNED
                    mat4 boneTransform = Prism_Bones[a_BoneIndices[0]] * a_BoneWeights[0];
                    boneTransform += Prism_Bones[a_BoneIndices[1]] * a_BoneWeights[1];
                    boneTransform += Prism_Bones[a_BoneIndices[2]] * a_BoneWeights[2];
                    boneTransform += Prism_Bones[a_BoneIndices[3]] * a_BoneWeights[3];

                    vec4 localPosition = boneTransform * vec4(a_Position, 1.0);
                    vs_Output.WorldPosition = vec3(Prism_Model * localPosition);
                    vs_Output.Normal = mat3(Prism_Model) * mat3(boneTransform) * a_Normal;
                #else
                    vs_Output.WorldPosition = vec3(Prism_Model * vec4(a_Position, 1.0));
                    vs_Output.Normal = mat3(Prism_Model) * a_Normal;
                #endif

                    vs_Output.TexCoord = vec2(
                        a_TexCoord.x * u_AlbedoTextureTiling.x + u_AlbedoTextureOffset.x,
                        a_TexCoord.y * u_AlbedoTextureTiling.y + u_AlbedoTextureOffset.y
                    );
                    vs_Output.WorldNormals = mat3(Prism_Model) * mat3(a_Tangent, a_Binormal, a_Normal);

                    vs_Output.ShadowMapCoords[0] = Prism_ShadowMatrices[0] * vec4(vs_Output.WorldPosition, 1.0);
                    vs_Output.ShadowMapCoords[1] = Prism_ShadowMatrices[1] * vec4(vs_Output.WorldPosition, 1.0);
                    vs_Output.ShadowMapCoords[2] = Prism_ShadowMatrices[2] * vec4(vs_Output.WorldPosition, 1.0);
                    vs_Output.ShadowMapCoords[3] = Prism_ShadowMatrices[3] * vec4(vs_Output.WorldPosition, 1.0);
                    vs_Output.ViewPosition = vec3(Prism_View * vec4(vs_Output.WorldPosition, 1.0));

                #ifdef SKINNED
                    gl_Position = Prism_ViewProjection * Prism_Model * localPosition;
                #else
                    gl_Position = Prism_ViewProjection * Prism_Model * vec4(a_Position, 1.0);
                #endif
                }

                void frag()
                {
                    vec3 albedo = u_AlbedoColor;
                #ifdef ALBEDO_MAP
                    albedo *= texture(u_AlbedoTexture, vs_Output.TexCoord).rgb;
                #endif

                    vec3 N = normalize(vs_Output.Normal);
                #ifdef NORMAL_MAP
                    vec3 tangentNormal = normalize(2.0 * texture(u_NormalTexture, vs_Output.TexCoord).rgb - 1.0);
                    N = normalize(vs_Output.WorldNormals * tangentNormal);
                #endif

                    vec3 V = normalize(Prism_CameraPosition - vs_Output.WorldPosition);
                    vec3 L = normalize(-Prism_Lights[0].Direction);
                    vec3 H = normalize(L + V);

                    float NdotL = dot(N, L);
                    float NdotV = max(dot(N, V), 0.0);
                    float NdotH = max(dot(N, H), 0.0);

                    float shadow = 1.0;
                    if (Prism_ShadowParams.z > 0.0)
                    {
                        vec3 biasedPos = vs_Output.WorldPosition + N * Prism_ShadowParams.y;
                        shadow = Prism_GetShadow(biasedPos, -vs_Output.ViewPosition.z, N);
                    }

                    vec3 lightRadiance = Prism_Lights[0].Radiance * Prism_Lights[0].Multiplier;

                    vec3 shadowColor = albedo * u_ShadowColor.rgb * u_ShadowBrightness;
                    vec3 highlightColor = albedo * u_HighlightIntensity;

                    float shadowStep = smoothstep(
                        u_ShadowThreshold - u_ShadowSmooth,
                        u_ShadowThreshold + u_ShadowSmooth,
                        NdotL
                    );
                    float highlightStep = smoothstep(
                        u_HighlightThreshold - u_ShadowSmooth,
                        u_HighlightThreshold + u_ShadowSmooth,
                        NdotL
                    );

                    vec3 diffuseColor = mix(shadowColor, albedo, shadowStep);
                    diffuseColor = mix(diffuseColor, highlightColor, highlightStep);

                    float spec = pow(NdotH, u_Shininess);
                    float specStep = smoothstep(
                        u_SpecularThreshold - u_SpecularWidth * 0.5,
                        u_SpecularThreshold + u_SpecularWidth * 0.5,
                        spec
                    );
                    vec3 specularColor = specStep * u_SpecularColor.rgb * u_SpecularIntensity;

                    float rim = 1.0 - NdotV;
                    rim = pow(rim, u_RimPower);
                    float rimStep = smoothstep(0.5 - u_RimSmooth, 0.5 + u_RimSmooth, rim);
                    vec3 rimColor = rimStep * u_RimColor.rgb * u_RimIntensity;

                    vec3 finalColor = (diffuseColor * lightRadiance + specularColor) * shadow + rimColor;
                    FragColor = vec4(finalColor, 1.0);
                }
            }
        }

        Pass
        {
            RenderCommand { Cull Front ZWrite On Blend SrcAlpha OneMinusSrcAlpha }

            Tags { "Queue" = "Geometry" "RenderType" = "Opaque" }
            Name "Outline"
            GLSL
            {
                #pragma shader_feature SKINNED
                #include "PrismCommon.glsl"

                attribute vec3 a_Position : POSITION;
                attribute vec3 a_Normal : NORMAL;
            #ifdef SKINNED
                attribute ivec4 a_BoneIndices : BONEINDICES;
                attribute vec4 a_BoneWeights : BONEWEIGHTS;
            #endif

                layout(location = 0) out vec4 FragColor;

                void vert()
                {
                #ifdef SKINNED
                    mat4 boneTransform = Prism_Bones[a_BoneIndices[0]] * a_BoneWeights[0];
                    boneTransform += Prism_Bones[a_BoneIndices[1]] * a_BoneWeights[1];
                    boneTransform += Prism_Bones[a_BoneIndices[2]] * a_BoneWeights[2];
                    boneTransform += Prism_Bones[a_BoneIndices[3]] * a_BoneWeights[3];

                    vec4 localPosition = boneTransform * vec4(a_Position, 1.0);
                    vec3 worldNormal = mat3(Prism_Model) * mat3(boneTransform) * a_Normal;
                    vec3 pos = localPosition.xyz + normalize(worldNormal) * u_OutlineWidth;
                    gl_Position = Prism_ViewProjection * Prism_Model * pos;
                #else
                    vec3 pos = a_Position + normalize(a_Normal) * u_OutlineWidth;
                    gl_Position = Prism_ViewProjection * Prism_Model * vec4(pos, 1.0);
                #endif
                }

                void frag()
                {
                    FragColor = u_OutlineColor;
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
                #include "PrismCommon.glsl"

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
