// Prism Shader Language v1.0
Shader "PostProcess/BloomBlur"
{
    Properties
    {
        u_Horizontal("水平模糊", Bool) = true
    }
    SubShader
    {
        Pass
        {
            Tags { "Queue" = "Overlay" "RenderType" = "Opaque" }
            Name "ForwardBase"
            GLSL
            {
                #include "PrismEngine.glsl"
                #include "Pass/BloomBlurPass.glsl"

                attribute vec3 a_Position : POSITION;
                attribute vec2 a_TexCoord : TEXCOORD0;

                varying VertexOutput
                {
                    vec2 TexCoord;
                } vs_Output;

                void main()
                {
                    vec4 position = vec4(a_Position.xy, 0.0, 1.0);
                    gl_Position = position;
                    vs_Output.TexCoord = a_TexCoord;
                }

                void frag()
                {
#if 0
                    float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

                    vec2 tex_offset = 1.0 / vec2(textureSize(Prism_BloomInputTexture));
                    vec3 result = texelFetch(Prism_BloomInputTexture, ivec2(vs_Output.TexCoord * vec2(textureSize(Prism_BloomInputTexture))), 0).rgb * weight[0];
                    if (u_Horizontal)
                    {
                        for (int i = 1; i < 5; ++i)
                        {
                            result += texelFetch(Prism_BloomInputTexture, ivec2((vs_Output.TexCoord + vec2(tex_offset.x * float(i), 0.0)) * vec2(textureSize(Prism_BloomInputTexture))), 0).rgb * weight[i];
                            result += texelFetch(Prism_BloomInputTexture, ivec2((vs_Output.TexCoord - vec2(tex_offset.x * float(i), 0.0)) * vec2(textureSize(Prism_BloomInputTexture))), 0).rgb * weight[i];
                        }
                    }
                    else
                    {
                        for (int i = 1; i < 5; ++i)
                        {
                            result += texelFetch(Prism_BloomInputTexture, ivec2((vs_Output.TexCoord + vec2(0.0, tex_offset.y * float(i))) * vec2(textureSize(Prism_BloomInputTexture))), 0).rgb * weight[i];
                            result += texelFetch(Prism_BloomInputTexture, ivec2((vs_Output.TexCoord - vec2(0.0, tex_offset.y * float(i))) * vec2(textureSize(Prism_BloomInputTexture))), 0).rgb * weight[i];
                        }
                    }
                    FragColor = vec4(result, 1.0);
#else
                    FragColor = texelFetch(Prism_BloomInputTexture, ivec2(vs_Output.TexCoord * vec2(textureSize(Prism_BloomInputTexture))), 0);
#endif
                }
            }
        }
    }
}
