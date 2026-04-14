// Prism Shader Language v1.0
Shader "Custom/Grid"
{
    // ==================== Properties（材质参数） ====================
    Properties
    {
        u_Scale("网格缩放", Float) = 8.0
        u_Res("网格分辨率", Float) = 0.05
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
                    mat4 u_MVP = Prism_ViewProjection * Prism_Model;
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
                    FragColor = vec4(vec3(0.2), 0.5) * (1.0 - x);
                }
            }
        }
    }
}