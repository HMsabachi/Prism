Shader "Prism/Renderer2D"
{
    Properties
    {
    }

    RenderCommand
    {
        Blend SrcAlpha OneMinusSrcAlpha
        Cull Off
        ZTest Off
        ZWrite On
    }

    SubShader
    {
        Pass
        {
            Tags { "Queue" = "Transparent" "RenderType" = "Transparent" }
            Name "Renderer2D_Quad"

            GLSL
            {
                #include "PrismBuiltin.glsl"

                attribute vec3 a_Position : POSITION;
                attribute vec4 a_Color : COLOR;
                attribute vec2 a_TexCoord : TEXCOORD0;
                attribute float a_TexIndex : INDEX0;
                attribute float a_TilingFactor : OTHER0;

                uniform sampler2D u_Textures[32];

                VARYING VertexOutput
                {
                    vec4 Color;
                    vec2 TexCoord;
                    float TexIndex;
                    float TilingFactor;
                } vs_Output;

                void main()
                {
                    vs_Output.Color        = a_Color;
                    vs_Output.TexCoord     = a_TexCoord;
                    vs_Output.TexIndex     = a_TexIndex;
                    vs_Output.TilingFactor = a_TilingFactor;

                    gl_Position = Prism_ViewProjection * vec4(a_Position, 1.0);
                }

                void frag()
                {
                    FragColor = texture(u_Textures[int(vs_Output.TexIndex)], vs_Output.TexCoord * vs_Output.TilingFactor) * vs_Output.Color;
                }
            }
        }
    }
}