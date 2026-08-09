// Prism Shader Language v1.0
Shader "PostProcess/BloomBlend"
{
    Properties
    {
        u_Exposure("曝光度", Float) = 1.0
        u_EnableBloom("启用辉光", Bool) = true
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
                #include "Pass/BloomBlendPass.glsl"

                attribute vec3 a_Position : POSITION;
                attribute vec2 a_TexCoord : TEXCOORD0;

                varying VertexOutput
                {
                    vec2 TexCoord;
                } vs_Output;

                void main()
                {
                    vec4 position = vec4(a_Position.xy, 1.0, 1.0);
                    gl_Position = position;
                    vs_Output.TexCoord = a_TexCoord;
                }

                void frag()
                {
                    const float gamma     = 2.2;
                    const float pureWhite = 1.0;

                    vec3 color = texelFetch(Prism_GeometryPassTexture, ivec2(vs_Output.TexCoord * vec2(textureSize(Prism_GeometryPassTexture))), 0).rgb;
#if 0
                    vec3 bloomColor = texelFetch(Prism_BloomTexture, ivec2(vs_Output.TexCoord * vec2(textureSize(Prism_BloomTexture))), 0).rgb;
                    if (u_EnableBloom)
                        color += bloomColor;
#endif

                    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
                    float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);
                    vec3 mappedColor = (mappedLuminance / luminance) * color * u_Exposure;

                    FragColor = vec4(mappedColor, 1.0);
                }
            }
        }
    }
}
