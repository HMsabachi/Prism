Shader "Prism/Renderer2D_Circle"
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
            Name "Renderer2D_Circle"

            GLSL
            {
                #include "PrismEngine.glsl"

                attribute vec3 a_WorldPosition : POSITION;
                attribute float a_Thickness : INDEX0;
                attribute vec2 a_LocalPosition : TEXCOORD0;
                attribute vec4 a_Color : COLOR;

                layout(location = 0) out vec4 FragColor;

                varying VertexOutput
                {
                    vec2 LocalPosition;
                    float Thickness;
                    vec4 Color;
                } vs_Output;

                void main()
                {
                    vs_Output.LocalPosition = a_LocalPosition;
                    vs_Output.Thickness    = a_Thickness;
                    vs_Output.Color        = a_Color;
                    gl_Position = Prism_ViewProjection * vec4(a_WorldPosition, 1.0);
                }

                void frag()
                {
                    float fade = 0.01;
                    float dist = sqrt(dot(vs_Output.LocalPosition, vs_Output.LocalPosition));
                    if (dist > 1.0 || dist < 1.0 - vs_Output.Thickness - fade)
                        discard;

                    float alpha = 1.0 - smoothstep(1.0 - fade, 1.0, dist);
                    alpha *= smoothstep(1.0 - vs_Output.Thickness - fade, 1.0 - vs_Output.Thickness, dist);
                    FragColor = vs_Output.Color;
                    FragColor.a *= alpha;
                }
            }
        }
    }
}
