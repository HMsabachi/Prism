Shader "Standard/ObjectID"
{
    Properties
    {
    }

    RenderCommand
    {
        Cull Off
        ZTest Off
        ZWrite Off
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

                void main()
                {
                #ifdef SKINNED
                    mat4 boneTransform = PRISM_BONE(a_BoneIndices[0]) * a_BoneWeights[0];
                    boneTransform += PRISM_BONE(a_BoneIndices[1]) * a_BoneWeights[1];
                    boneTransform += PRISM_BONE(a_BoneIndices[2]) * a_BoneWeights[2];
                    boneTransform += PRISM_BONE(a_BoneIndices[3]) * a_BoneWeights[3];

                    vec4 localPosition = boneTransform * vec4(a_Position, 1.0);
                    gl_Position = Prism_ViewProjection * Prism_ObjectToWorld * localPosition;
                #else
                    gl_Position = Prism_ViewProjection * Prism_ObjectToWorld * vec4(a_Position, 1.0);
                #endif
                }

                void frag()
                {
                    FragColor = vec4(float(Prism_DrawIndex + 1), 0.0, 0.0, 1.0);
                }
            }
        }
    }
}
