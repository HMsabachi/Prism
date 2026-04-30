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
    }

    SubShader
    {
        Pass
        {
            Tags { "Queue" = "Geometry" "RenderType" = "Opaque" }
            Name "ForwardBase"
            GLSL
            {
                #include "PrismBuiltin.glsl"

                attribute vec3 a_Position : POSITION;
                attribute vec2 a_TexCoord : TEXCOORD0; // 暂时这样需要修改

                VARYING VertexOutput
                {
                    vec2 TexCoord;
                } vs_Output;

                void main()
                {
                    gl_Position = Prism_ViewProjection * Prism_Model * vec4(a_Position, 1.0);
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