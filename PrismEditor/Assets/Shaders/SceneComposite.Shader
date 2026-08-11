// Prism Shader Language v1.0
Shader "Custom/SceneComposite"
{
    // ==================== Properties（材质参数） ====================
    Properties
    {
        u_Exposure("曝光度", Float) = 1.0
        u_OutlineColor("描边颜色", Color) = (1.0, 0.5, 0.0, 1.0)
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
                #include "Pass/CompositePass.glsl"

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

                    vec3 color = texture(Prism_GeometryPassTexture, vs_Output.TexCoord).rgb * u_Exposure;
                    // Reinhard tonemapping operator.
                    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
                    float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);

                    vec3 mappedColor = (mappedLuminance / luminance) * color;

                    vec2 texel = 1.0 / vec2(textureSize(Prism_ObjectIDTexture, 0));
                    float idC = texture(Prism_ObjectIDTexture, vs_Output.TexCoord).r;
                    float idL = texture(Prism_ObjectIDTexture, vs_Output.TexCoord + vec2(-texel.x, 0.0)).r;
                    float idR = texture(Prism_ObjectIDTexture, vs_Output.TexCoord + vec2( texel.x, 0.0)).r;
                    float idD = texture(Prism_ObjectIDTexture, vs_Output.TexCoord + vec2(0.0, -texel.y)).r;
                    float idU = texture(Prism_ObjectIDTexture, vs_Output.TexCoord + vec2(0.0,  texel.y)).r;

                    bool sel = idC > 0.5;
                    bool edge = sel != (idL > 0.5) || sel != (idR > 0.5)
                             || sel != (idD > 0.5) || sel != (idU > 0.5);

                    vec3 finalColor = edge ? u_OutlineColor.rgb : mappedColor;

                    // Gamma correction.
                    FragColor = vec4(pow(finalColor, vec3(1.0/gamma)), 1.0);
                }
            }
        }
    }
}