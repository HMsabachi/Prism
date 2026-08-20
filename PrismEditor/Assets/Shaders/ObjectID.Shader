Shader "Standard/ObjectID"
{
    Properties
    {
    }

    RenderCommand
    {
        Cull Back
        Blend One OneMinusSrcAlpha
        ZTest LEqual
        ZWrite On
    }

    SubShader
    {
        Pass
        {
            Tags { "Queue" = "Geometry" "RenderType" = "Opaque" }
            Name "ForwardBase"
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
    }
}
