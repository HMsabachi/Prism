// Prism Shader Language v1.0
Shader "Hidden/EditorDebug"
{
    Properties
    {
        u_Scale("网格缩放", Float) = 16.025
        u_Res("网格分辨率", Float) = 0.025
        u_CullAlpha("剔除透明度", Range(0.0, 1.0)) = 0.1
        u_ColliderColor("碰撞体颜色", Color) = (0.1, 1.0, 0.1, 1.0)
    }

    SubShader
    {
        Tags { "ShaderType" = "Hidden" }

        Pass
        {
            Name "ObjectID"
            Tags { "LightMode" = "Always" }
            RenderCommand
            {
                Cull Back
                Blend One OneMinusSrcAlpha
                ZTest LEqual
                ZWrite On
            }
            GLSL
            {
                #pragma shader_feature SKINNED
                #include "PrismEngine.glsl"

                attribute vec3 a_Position : POSITION;
            #ifdef SKINNED
                attribute ivec4 a_BoneIndices : BONEINDICES;
                attribute vec4 a_BoneWeights : BONEWEIGHTS;
            #endif

                layout(location = 0) out vec4 FragColor;

                const vec3 COLORS[12] = vec3[](
                    vec3(1.0, 0.0, 0.0), // 红
                    vec3(0.0, 1.0, 0.0), // 绿
                    vec3(0.0, 0.0, 1.0), // 蓝
                    vec3(1.0, 1.0, 0.0), // 黄
                    vec3(1.0, 0.0, 1.0), // 品红
                    vec3(0.0, 1.0, 1.0), // 青
                    vec3(1.0, 0.5, 0.0), // 橙
                    vec3(0.5, 0.0, 1.0), // 紫
                    vec3(0.0, 0.5, 1.0), // 天蓝
                    vec3(0.5, 1.0, 0.0), // 黄绿
                    vec3(1.0, 0.0, 0.5), // 粉红
                    vec3(0.0, 1.0, 0.5)  // 青绿
                );

                void main()
                {
                #ifdef SKINNED
                    mat4 boneTransform = Prism_GetBoneMatrix(a_BoneIndices[0]) * a_BoneWeights[0];
                    boneTransform += Prism_GetBoneMatrix(a_BoneIndices[1]) * a_BoneWeights[1];
                    boneTransform += Prism_GetBoneMatrix(a_BoneIndices[2]) * a_BoneWeights[2];
                    boneTransform += Prism_GetBoneMatrix(a_BoneIndices[3]) * a_BoneWeights[3];

                    vec4 localPosition = boneTransform * vec4(a_Position, 1.0);
                    gl_Position = Prism_ViewProjection * Prism_ObjectToWorld * localPosition;
                #else
                    gl_Position = Prism_ViewProjection * Prism_ObjectToWorld * vec4(a_Position, 1.0);
                #endif
                }


                void frag()
                {
                    FragColor = vec4(COLORS[Prism_DrawIndex % 12], 1.0);
                }
            }
        }

        Pass
        {
            Name "Grid"
            Tags { "LightMode" = "Always" }
            RenderCommand
            {
                Cull Off
                ZTest LEqual
                ZWrite On
                Blend SrcAlpha OneMinusSrcAlpha
            }
            GLSL
            {
                #include "PrismEngine.glsl"
                #include "Pass/GeometryPass.glsl"

                attribute vec3 a_Position : POSITION;
                attribute vec2 a_TexCoord : TEXCOORD0;

                varying VertexOutput
                {
                    vec2 TexCoord;
                } vs_Output;

                void main()
                {
                    mat4 u_MVP = Prism_ViewProjection * Prism_ObjectToWorld;
                    vec4 position = u_MVP * vec4(a_Position, 1.0);
                    gl_Position = position;

                    vs_Output.TexCoord = a_TexCoord;
                }

                float grid(vec2 st, float res)
                {
                    vec2 grid = fract(st);
                    return step(res, grid.x) * step(res, grid.y);
                }

                void frag()
                {
                    float scale = u_Scale;
                    float resolution = u_Res;

                    float x = grid(vs_Output.TexCoord * scale, resolution);
                    if (x > u_CullAlpha) discard;
                    FragColor = vec4(vec3(0.2), 0.5) * (1.0 - x);
                }
            }
        }

        Pass
        {
            Name "Collider"
            Tags { "LightMode" = "ForwardBase" }
            RenderCommand
            {
                Cull Off
                ZTest Off
                ZWrite Off
                Blend SrcAlpha OneMinusSrcAlpha
                PolygonMode Line
                LineWidth 3
            }
            GLSL
            {
                #include "PrismEngine.glsl"
                #include "Pass/GeometryPass.glsl"

                attribute vec3 a_Position : POSITION;

                void main()
                {
                    gl_Position = Prism_ViewProjection * Prism_ObjectToWorld * vec4(a_Position, 1.0);
                }

                void frag()
                {
                    FragColor = u_ColliderColor;
                    BloomColor = vec4(0.0);
                    NormalColor = vec4(0.0);
                }
            }
        }
    }
}
