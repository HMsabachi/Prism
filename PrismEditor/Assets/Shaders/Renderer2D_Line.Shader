// Prism Shader Language v1.0
Shader "Prism/Renderer2D_Line"
{
	Properties
	{
		
	}

    RenderCommand
    {
        Blend SrcAlpha OneMinusSrcAlpha
        Cull Off
        ZTest LEqual
        ZWrite On
    }

    SubShader
    {
        Pass
        {
            Tags { "Queue" = "Transparent" "RenderType" = "Transparent" }
            Name "Renderer2D_Line"

            GLSL
            {
                #include "PrismBuiltin.glsl"
                attribute vec3 a_Position : POSITION;
                attribute vec4 a_Color : COLOR;

                VARYING VertexOutput
                {
                    vec4 Color;
                } vs_Output;

                void main()
                {
                    vs_Output.Color = a_Color;
                    gl_Position = Prism_ViewProjection * vec4(a_Position, 1.0);
                }

                void frag()
                {
                    FragColor = vs_Output.Color;
                }
            }
        }
    }
}