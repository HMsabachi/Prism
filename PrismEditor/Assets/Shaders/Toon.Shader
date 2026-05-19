Shader "Custom/Toon"
{
    Properties
    {
        u_AlbedoColor("固有色", Color3) = (1.0, 1.0, 1.0)
        u_MainTexture("固有色贴图", Texture2D) = {}
        u_MainTextureTiling("贴图缩放", Vector2) = (1.0, 1.0)
        u_MainTextureOffset("贴图偏移", Vector2) = (0.0, 0.0)

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

        u_LightDirection("光照方向", Vector3) = (1.0, 1.0, 1.0)
        u_LightColor("光照颜色", Vector3) = (1.0, 1.0, 1.0)
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
            Tags { "Queue" = "Geometry" "RenderType" = "Opaque" }
            Name "ForwardBase"
            GLSL
            {
                #pragma shader_feature MAIN_TEXTURE NORMAL_MAP
                #include "PrismBuiltin.glsl"

                attribute vec3 a_Position : POSITION;
                attribute vec3 a_Normal : NORMAL;
                attribute vec3 a_Tangent : TANGENT;
                attribute vec3 a_Binormal : BINORMAL;
                attribute vec2 a_TexCoord : TEXCOORD0;

                VARYING VertexOutput
                {
                    vec3 WorldPosition;
                    vec3 Normal;
                    vec2 TexCoord;
                    mat3 WorldNormals;
                } vs_Output;

                void main()
                {
                    vec4 worldPos = Prism_Model * vec4(a_Position, 1.0);
                    vs_Output.WorldPosition = vec3(worldPos);
                    vs_Output.Normal = mat3(Prism_Model) * a_Normal;
                    vs_Output.TexCoord = vec2(
                        a_TexCoord.x * u_MainTextureTiling.x + u_MainTextureOffset.x,
                        a_TexCoord.y * u_MainTextureTiling.y + u_MainTextureOffset.y
                    );
                    vs_Output.WorldNormals = mat3(Prism_Model) * mat3(a_Tangent, a_Binormal, a_Normal);

                    gl_Position = Prism_ViewProjection * worldPos;
                }

                void frag()
                {
                    vec3 u_CameraPosition = Prism_CameraPosition;
                    vec3 lightDir = normalize(u_LightDirection);
                    vec3 lightColor = u_LightColor;

                    vec3 albedo = u_AlbedoColor;
                #ifdef MAIN_TEXTURE
                    albedo *= texture(u_MainTexture, vs_Output.TexCoord).rgb;
                #endif

                    vec3 N = normalize(vs_Output.Normal);
                #ifdef NORMAL_MAP
                    vec3 tangentNormal = normalize(2.0 * texture(u_NormalTexture, vs_Output.TexCoord).rgb - 1.0);
                    N = normalize(vs_Output.WorldNormals * tangentNormal);
                #endif

                    vec3 V = normalize(u_CameraPosition - vs_Output.WorldPosition);
                    vec3 L = lightDir;
                    vec3 H = normalize(L + V);

                    float NdotL = dot(N, L);
                    float NdotV = max(dot(N, V), 0.0);
                    float NdotH = max(dot(N, H), 0.0);
                    float shadow = 1.0;
                    if (Prism_ShadowParams.z > 0.0)
                    {
                        vec3 biasedPos = vs_Output.WorldPosition + N * Prism_ShadowParams.y;
                        shadow = Prism_GetShadow(biasedPos, -(Prism_View * vec4(vs_Output.WorldPosition, 1.0)).z);
                    }

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
                    vec3 finalColor = (diffuseColor * lightColor + specularColor) * shadow + rimColor;
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
                #include "PrismBuiltin.glsl"

                attribute vec3 a_Position : POSITION;
                attribute vec3 a_Normal : NORMAL;

                void main()
                {
                    vec3 pos = a_Position + normalize(a_Normal) * u_OutlineWidth;
                    gl_Position = Prism_ViewProjection * Prism_Model * vec4(pos, 1.0);
                }

                void frag()
                {
                    FragColor = u_OutlineColor;
                }
            }
        }
    }
}
