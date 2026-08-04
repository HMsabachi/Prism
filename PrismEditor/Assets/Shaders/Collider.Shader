// Prism Shader Language v1.0
Shader "Debug/Collider"
{
    Properties
    {
        u_Color("Color", Color) = (0.1, 1.0, 0.1, 1.0)
    }

    RenderCommand
    {
        Cull Off
        ZTest Off
        ZWrite Off
        Blend SrcAlpha OneMinusSrcAlpha
        PolygonMode Line
        LineWidth 3
    }

    SubShader
    {
        Pass
        {
            Tags { "Queue" = "Transparent" "RenderType" = "Transparent" }
            Name "ForwardBase"
            GLSL
            {
                #include "PrismEngine.glsl"

                attribute vec3 a_Position : POSITION;

                layout(location = 0) out vec4 FragColor;

                void main()
                {
                    gl_Position = Prism_ViewProjection * Prism_Model * vec4(a_Position, 1.0);
                }

                void frag()
                {
                    FragColor = u_Color;
                }
            }
        }
    }
}
