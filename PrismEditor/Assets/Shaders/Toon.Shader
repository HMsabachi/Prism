// Prism Shader Language v1.0
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
        u_BrightThreshold("亮面阈值", Range(0, 1)) = 0.75
        u_ShadowSmooth("色阶柔化", Range(0.001, 0.5)) = 0.05
        u_ShadowColor("阴影色", Color) = (0.35, 0.45, 0.65, 1.0)
        u_ShadowBoost("阴影亮度", Range(0, 1)) = 0.6
        u_BrightBoost("亮面增强", Range(1.0, 3.0)) = 1.2

        u_SpecularColor("高光色", Color) = (1.0, 1.0, 1.0, 1.0)
        u_SpecularIntensity("高光强度", Range(0, 5)) = 1.5
        u_Shininess("光泽度", Range(1, 200)) = 30.0
        u_SpecularSmooth("高光柔化", Range(0.001, 1.0)) = 0.15

        u_RimColor("边缘光色", Color) = (1.0, 0.6, 0.3, 1.0)
        u_RimIntensity("边缘光强度", Range(0, 5)) = 1.2
        u_RimPower("边缘光范围", Range(0.1, 10)) = 3.0
        u_RimSmooth("边缘光柔化", Range(0.001, 0.5)) = 0.05

        u_EmissionColor("自发光色", Color3) = (0.0, 0.0, 0.0)
        u_EmissionTexture("自发光贴图", Texture2D) = {}

        u_AmbientColor("环境光色", Color3) = (0.05, 0.05, 0.10)

        u_OutlineColor("描边色", Color) = (0.05, 0.05, 0.1, 1.0)
        u_OutlineWidth("描边宽度", Float) = 0.02
    }

    RenderCommand
    {
        Cull Back
        ZTest LessEqual
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
                #pragma shader_feature SKINNED
                #pragma shader_feature ALBEDO_MAP NORMAL_MAP EMISSION_MAP
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
                    float NdotV = max(dot(N, V), 0.0);

                    vec3 color = vec3(0.0);
                    for (int i = 0; i < PRISM_MAX_LIGHTS; i++)
                    {
                        vec3 L = normalize(-Prism_Lights[i].Direction);
                        vec3 H = normalize(L + V);
                        float NdotL = dot(N, L);
                        float NdotH = max(dot(N, H), 0.0);
                        float cosLi = max(NdotL, 0.0);

                        float shadowStep = smoothstep(
                            u_ShadowThreshold - u_ShadowSmooth,
                            u_ShadowThreshold + u_ShadowSmooth,
                            NdotL
                        );
                        float brightStep = smoothstep(
                            u_BrightThreshold - u_ShadowSmooth,
                            u_BrightThreshold + u_ShadowSmooth,
                            NdotL
                        );

                        vec3 diffuse = mix(u_ShadowColor.rgb * u_ShadowBoost, vec3(1.0), shadowStep);
                        diffuse = mix(diffuse, vec3(u_BrightBoost), brightStep);
                        diffuse *= albedo;

                        float spec = pow(NdotH, u_Shininess);
                        float specStep = smoothstep(
                            0.5 - u_SpecularSmooth,
                            0.5 + u_SpecularSmooth,
                            spec
                        );
                        vec3 specular = specStep * u_SpecularColor.rgb * u_SpecularIntensity;

                        vec3 radiance = Prism_Lights[i].Radiance * Prism_Lights[i].Multiplier;
                        color += (diffuse + specular) * radiance * cosLi;
                    }

                    float shadow = 1.0;
                    if (Prism_ShadowParams.z > 0.0)
                    {
                        vec3 biasedPos = vs_Output.WorldPosition + N * Prism_ShadowParams.y;
                        shadow = Prism_GetShadow(biasedPos, -vs_Output.ViewPosition.z, N);
                    }
                    color *= shadow;
                    color += albedo * u_AmbientColor;

                    float rim = 1.0 - NdotV;
                    rim = pow(rim, u_RimPower);
                    float rimStep = smoothstep(0.5 - u_RimSmooth, 0.5 + u_RimSmooth, rim);
                    color += rimStep * u_RimColor.rgb * u_RimIntensity;

                    vec3 emission = u_EmissionColor;
                #ifdef EMISSION_MAP
                    emission *= texture(u_EmissionTexture, vs_Output.TexCoord).rgb;
                #endif
                    color += emission;

                    FragColor = vec4(color, 1.0);

                    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
                    BloomColor = brightness > 1.0 ? FragColor : vec4(0.0);
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
                #include "PrismEngine.glsl"
                #include "Pass/GeometryPass.glsl"

                attribute vec3 a_Position : POSITION;
                attribute vec3 a_Normal : NORMAL;
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
                    vec3 worldNormal = mat3(Prism_Model) * mat3(boneTransform) * a_Normal;
                    vec3 worldPos = vec3(Prism_Model * localPosition);
                    vec4 viewPos = Prism_View * vec4(worldPos, 1.0);
                    vec3 viewNormal = mat3(Prism_View) * worldNormal;
                    viewPos.xyz += normalize(viewNormal) * u_OutlineWidth * (-viewPos.z);
                    gl_Position = Prism_Projection * viewPos;
                #else
                    vec4 viewPos = Prism_View * Prism_Model * vec4(a_Position, 1.0);
                    vec3 viewNormal = mat3(Prism_View) * mat3(Prism_Model) * a_Normal;
                    viewPos.xyz += normalize(viewNormal) * u_OutlineWidth * (-viewPos.z);
                    gl_Position = Prism_Projection * viewPos;
                #endif
                }

                void frag()
                {
                    FragColor = u_OutlineColor;
                }
            }
        }

        UsePass "Standard/PrismPBR" "ShadowCaster"
    }
}
