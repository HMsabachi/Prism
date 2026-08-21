// Prism Shader Language v1.0
Shader "PostProcess/PrismPostProcess"
{
    // ==================== Properties（材质参数） ====================
    Properties
    {
        u_Exposure("曝光度", Float) = 1.0
        u_OutlineColor("描边颜色", Color) = (1.0, 0.5, 0.0, 1.0)
        u_OutlineWidth("描边宽度", Float) = 2.0
        u_Horizontal("水平模糊", Bool) = true
    }

    SubShader
    {
        Tags { "ShaderType" = "PostProcess" "Queue" = "Overlay" }

        Pass
        {
            Name "SceneComposite"
            Tags { "LightMode" = "Fullscreen" }
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

                bool IsVec3Equal(vec3 a, vec3 b)
                {
                    return all(equal(a, b));
                }

                bool ShouldDrawOutline(vec2 texCoord)
                {
                    vec2 texel = 1.0 / vec2(textureSize(Prism_ObjectIDTexture, 0));
                    vec3 idC = texture(Prism_ObjectIDTexture, vs_Output.TexCoord).rgb;
                    bool edge = false;
                    int radius = int(ceil(u_OutlineWidth));
                    for (int y = -radius; y <= radius; y++)
                    {
                        for (int x = -radius; x <= radius; x++)
                        {
                            if (x == 0 && y == 0)
                                continue;
                            vec2 offset = vec2(x, y) * texel;
                            vec3 id = texture(
                                Prism_ObjectIDTexture,
                                texCoord + offset
                            ).rgb;
                            if (!IsVec3Equal(idC, id))
                            {
                                edge = true;
                            }
                        }
                    }
                    return edge;
                }

                void frag()
                {
                    const float gamma     = 2.2;
                    const float pureWhite = 1.0;

                    vec3 color = texture(Prism_GeometryPassTexture, vs_Output.TexCoord).rgb;
                    color += texture(Prism_BloomTexture, vs_Output.TexCoord).rgb;
                    color *= u_Exposure;
                    // Reinhard tonemapping operator.
                    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
                    float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);

                    vec3 mappedColor = (mappedLuminance / luminance) * color;

                    vec2 texel = u_OutlineWidth / vec2(textureSize(Prism_ObjectIDTexture, 0));
                    vec3 idC = texture(Prism_ObjectIDTexture, vs_Output.TexCoord).rgb;
                    vec3 idL = texture(Prism_ObjectIDTexture, vs_Output.TexCoord + vec2(-texel.x, 0.0)).rgb;
                    vec3 idR = texture(Prism_ObjectIDTexture, vs_Output.TexCoord + vec2( texel.x, 0.0)).rgb;
                    vec3 idD = texture(Prism_ObjectIDTexture, vs_Output.TexCoord + vec2(0.0, -texel.y)).rgb;
                    vec3 idU = texture(Prism_ObjectIDTexture, vs_Output.TexCoord + vec2(0.0,  texel.y)).rgb;
                    bool sel = IsVec3Equal(idC, idL) && IsVec3Equal(idC, idR) && IsVec3Equal(idC, idD) && IsVec3Equal(idC, idU);
                    bool edge = !sel;

                    // bool edge = ShouldDrawOutline(vs_Output.TexCoord);

                    vec3 finalColor = edge ? u_OutlineColor.rgb : mappedColor;

                    // Gamma correction.
                    FragColor = vec4(pow(finalColor, vec3(1.0/gamma)), 1.0);
                }
            }
        }

        Pass
        {
            Name "BloomBlur"
            Tags { "LightMode" = "Fullscreen" }
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
                    float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

                    ivec2 res = textureSize(Prism_BloomInputTexture, 0);
                    vec2 tex_offset = 1.0 / vec2(res);
                    vec3 result = texelFetch(Prism_BloomInputTexture, ivec2(vs_Output.TexCoord * vec2(res)), 0).rgb * weight[0];
                    if (u_Horizontal)
                    {
                        for (int i = 1; i < 5; ++i)
                        {
                            result += texelFetch(Prism_BloomInputTexture, ivec2((vs_Output.TexCoord + vec2(tex_offset.x * float(i), 0.0)) * vec2(res)), 0).rgb * weight[i];
                            result += texelFetch(Prism_BloomInputTexture, ivec2((vs_Output.TexCoord - vec2(tex_offset.x * float(i), 0.0)) * vec2(res)), 0).rgb * weight[i];
                        }
                    }
                    else
                    {
                        for (int i = 1; i < 5; ++i)
                        {
                            result += texelFetch(Prism_BloomInputTexture, ivec2((vs_Output.TexCoord + vec2(0.0, tex_offset.y * float(i))) * vec2(res)), 0).rgb * weight[i];
                            result += texelFetch(Prism_BloomInputTexture, ivec2((vs_Output.TexCoord - vec2(0.0, tex_offset.y * float(i))) * vec2(res)), 0).rgb * weight[i];
                        }
                    }
                    FragColor = vec4(result, 1.0);
                }
            }
        }
    }
}
