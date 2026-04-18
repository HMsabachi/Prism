// Prism Shader Language v1.0
Shader "Custom/SceneComposite"
{
    // ==================== Properties（材质参数） ====================
    Properties
    {
        u_Texture("输入纹理", Texture2DMS) = {}
        u_Exposure("曝光度", Float) = 1.0
        u_TextureSamples("纹理采样数", Int) = 8
    }
    SubShader
    {
        Pass
        {
            Tags { "Queue" = "Overlay" "RenderType" = "Opaque" }
            Name "ForwardBase"
            GLSL
            {
                #include "PrismBuiltin.glsl"

                attribute vec3 a_Position : POSITION;
                attribute vec2 a_TexCoord : TEXCOORD0;

                VARYING VertexOutput
                {
                    vec2 TexCoord;
                } vs_Output;

                void main()
                {
                    vec4 position = vec4(a_Position.xy, 1.0, 1.0);
                    gl_Position = position;

                    vs_Output.TexCoord = a_TexCoord;
                }

                vec4 MultiSampleTexture(sampler2DMS tex, ivec2 texCoord, int samples)
                {
                    vec4 result = vec4(0.0);
                    for (int i = 0; i < samples; i++)
                        result += texelFetch(tex, texCoord, i);
                    result /= float(samples);
                    return result;
                }

                void frag()
                {
                    const float gamma     = 2.2;
                    const float pureWhite = 1.0;

                    ivec2 texSize = textureSize(u_Texture);
                    ivec2 texCoord = ivec2(vs_Output.TexCoord * texSize);
                    vec4 msColor = MultiSampleTexture(u_Texture, texCoord, u_TextureSamples);

                    vec3 color = msColor.rgb * u_Exposure;//texture(u_Texture, vs_Output.TexCoord).rgb * u_Exposure;
                    // Reinhard tonemapping operator.
                    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
                    float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);

                    vec3 mappedColor = (mappedLuminance / luminance) * color;

                    // Gamma correction.
                    FragColor = vec4(pow(mappedColor, vec3(1.0/gamma)), 1.0);
                }
            }
        }
    }
}