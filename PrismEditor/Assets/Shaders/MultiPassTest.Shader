Shader "Test/MultiPass"
{
    Properties
    {
        u_Color0("Pass0 颜色", Color) = (1.0, 0.2, 0.2, 1.0)
        u_Color1("Pass1 颜色", Color) = (0.2, 1.0, 0.2, 0.6)
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
            Name "RedPass"
            GLSL
            {
                #include "PrismCommon.glsl"

                attribute vec3 a_Position : POSITION;
                attribute vec3 a_Normal : NORMAL;

                VARYING VertexOutput
                {
                    vec3 Normal;
                } vs_Output;

                void main()
                {
                    vs_Output.Normal = mat3(Prism_Model) * a_Normal;
                    gl_Position = Prism_ViewProjection * Prism_Model * vec4(a_Position, 1.0);
                }

                void frag()
                {
                    vec3 N = normalize(vs_Output.Normal);
                    float NdotL = max(dot(N, normalize(vec3(1, 1, 1))), 0.0);
                    FragColor = vec4(u_Color0.rgb * (0.3 + 0.7 * NdotL), 1.0);
                }
            }
        }

        Pass
        {
            RenderCommand { Cull Off ZTest Off ZWrite Off Blend SrcAlpha OneMinusSrcAlpha }
            Name "GreenPass"
            GLSL
            {
                #include "PrismCommon.glsl"

                attribute vec3 a_Position : POSITION;
                attribute vec3 a_Normal : NORMAL;

                VARYING VertexOutput
                {
                    vec3 Normal;
                } vs_Output;

                void main()
                {
                    vs_Output.Normal = mat3(Prism_Model) * a_Normal;
                    gl_Position = Prism_ViewProjection * Prism_Model * vec4(a_Position, 1.0);
                }

                void frag()
                {
                    FragColor = u_Color1;
                }
            }
        }
    }
}
