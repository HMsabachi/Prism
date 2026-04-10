// Prism Shader Language v1.0
Shader "Custom/HDR"
{
    // ==================== Properties（材质参数） ====================
    Properties
    {
        u_Texture("HDR输入纹理", Texture2D) = {}
        u_Exposure("曝光度", Float) = 1.0
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
                attribute vec2 a_TexCoord : NORMAL;

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

                void frag()
                {
                    const float gamma     = 2.2;
                    const float pureWhite = 1.0;

                    vec3 color = texture(u_Texture, vs_Output.TexCoord).rgb * u_Exposure;
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