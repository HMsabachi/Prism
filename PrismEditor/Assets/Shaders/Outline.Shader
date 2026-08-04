Shader "Standard/Outline"
{
    // ==================== Properties（材质参数） ====================
    Properties
    {
        u_MainColor("Main Color", Color) = (1.0, 0.5, 0.0, 1.0)
    }

    RenderCommand
    {
        Cull Off
        ZTest Off
        ZWrite Off
        Blend SrcAlpha OneMinusSrcAlpha
        Stencil
        {
            Ref 1
            Comp NotEqual
            ReadMask 0xff
            WriteMask 0
        }
        PolygonMode Line
        LineWidth 10
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
                attribute vec2 a_TexCoord : TEXCOORD0; // 暂时这样需要修改
            #ifdef SKINNED
                attribute ivec4 a_BoneIndices : BONEINDICES;
                attribute vec4 a_BoneWeights : BONEWEIGHTS;
            #endif

                layout(location = 0) out vec4 FragColor;

                varying VertexOutput
                {
                    vec2 TexCoord;
                } vs_Output;

                void main()
                {
                #ifdef SKINNED
                    mat4 boneTransform = Prism_Bones[a_BoneIndices[0]] * a_BoneWeights[0];
                    boneTransform += Prism_Bones[a_BoneIndices[1]] * a_BoneWeights[1];
                    boneTransform += Prism_Bones[a_BoneIndices[2]] * a_BoneWeights[2];
                    boneTransform += Prism_Bones[a_BoneIndices[3]] * a_BoneWeights[3];

                    vec4 localPosition = boneTransform * vec4(a_Position, 1.0);
                    gl_Position = Prism_ViewProjection * Prism_Model * localPosition;
                #else
                    gl_Position = Prism_ViewProjection * Prism_Model * vec4(a_Position, 1.0);
                #endif
                    vs_Output.TexCoord = a_TexCoord;
                }


                void frag()
                {
                    FragColor = u_MainColor;
                }
            }
        }
    }
}