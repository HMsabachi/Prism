// Prism Shader Language v1.0
Shader "Custom/Quad"
{
    // ==================== Properties（材质参数） ====================
    Properties
    {
        u_Texture("立方体贴图", TextureCube) = {}
    }
    SubShader
    {
        Pass
        {
            Tags { "Queue" = "Background" "RenderType" = "Opaque" }
            Name "ForwardBase"
            GLSL
            {
                #include "PrismBuiltin.glsl"

                attribute vec3 a_Position : POSITION;
                attribute vec2 a_TexCoord : TEXCOORD0;

                VARYING VertexOutput
                {
                    vec3 Position;
                } vs_Output;

                void main()
                {
                    vec4 position = vec4(a_Position.xy, 1.0, 1.0);
                    gl_Position = position;

                    // 使用Properties中声明的u_InverseVP（引擎会自动绑定）
                    vs_Output.Position = (Prism_InverseViewProjection * position).xyz;
                }

                void frag()
                {
                    FragColor = textureLod(u_Texture, vs_Output.Position, 0.0);
                }
            }
        }
    }
}