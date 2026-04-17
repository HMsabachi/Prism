// Prism Shader Language v1.0
Shader "Custom/Skybox"
{
    // ==================== Properties（材质参数） ====================
    Properties
    {
        u_Texture("立方体贴图", TextureCube) = {}
        u_TextureLod("立方体贴图", Float) = 0.0
    }
    SubShader
    {

    RenderCommand
    {
        Cull Back
        ZTest Off
        ZWrite Off
        Blend One OneMinusSrcAlpha
    }

        Pass
        {
            Tags { "Queue" = "Background" "RenderType" = "Opaque" }
            Name "ForwardBase"
            GLSL
            {
                #include "PrismBuiltin.glsl"

                attribute vec3 a_Position : POSITION;

                VARYING VertexOutput
                {
                    vec3 Position;
                } vs_Output;

                void main()
                {
                    vec4 position = vec4(a_Position.xy, 1.0, 1.0);
                    gl_Position = position;
                    vs_Output.Position = (Prism_InverseViewProjection * position).xyz;
                }

                void frag()
                {
                    FragColor = textureLod(u_Texture, vs_Output.Position, u_TextureLod);
                }
            }
        }
    }
}