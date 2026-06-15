// Prism Shader Language v1.0
Shader "Custom/NoiseTest"
{
    // ==================== Properties（材质参数） ====================
    Properties
    {
        u_MainColor("Main Color", Color) = (1, 1, 1, 1)
    }

    RenderCommand
    {
        Cull Off
        ZTest LEqual
        ZWrite On
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
                // #include "FastNoise/FastNoiseLite.glsl"

                attribute vec3 a_Position : POSITION;
                attribute vec2 a_TexCoord : TEXCOORD0;

                VARYING VertexOutput
                {
                    vec2 TexCoord;
                } vs_Output;

                void vert()
                {
                    gl_Position = Prism_ViewProjection * Prism_Model * vec4(a_Position, 1.0);
                    vs_Output.TexCoord = a_TexCoord;
                }

 
                void frag()
                {
                    // fnl_state state = fnlCreateState(1337);
                    // state.noise_type = FNL_NOISE_CELLULAR;
                    // state.fractal_type = FNL_FRACTAL_FBM;
                    // state.frequency = .01f;
                    // state.octaves = 4;
                    // state.lacunarity = 2.f;
                    // state.gain = .5f;

                    // vec2 uv = vs_Output.TexCoord * Prism_Resolution.xy;
                    
                    // float noise = fnlGetNoise3D(state, uv.x, Prism_Time.x * 50.f, uv.y) / 2.f + 0.5f;
                    FragColor = u_MainColor;
                    // FragColor.rgb *= noise;
                }
            }
        }
    }
}