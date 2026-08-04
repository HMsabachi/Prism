// Prism Shader Language v1.0
// Glass Lens — 7-layer optical simulation
// Refraction · Chromatic Aberration · Fresnel · Thin Film · Caustics · Surface Noise · Beer-Lambert
Shader "Custom/GlassLens"
{
    Properties
    {
        u_GlassColor("玻璃色调", Color3) = (0.92, 0.97, 1.0)
        u_IOR("折射率 IOR", Range(1.0, 2.5)) = 1.5
        u_Aberration("色散强度", Range(0.0, 10.0)) = 3.0
        u_FilmThickness("薄膜厚度nm", Range(0.0, 1000.0)) = 400.0
        u_FilmIOR("薄膜折射率", Range(1.0, 2.0)) = 1.3
        u_CausticIntensity("焦散强度", Range(0.0, 1.0)) = 0.25
        u_CausticScale("焦散缩放", Range(1.0, 50.0)) = 10.0
        u_CausticSpeed("焦散速度", Range(0.0, 5.0)) = 0.8
        u_NoiseScale("表面噪声", Range(1.0, 200.0)) = 40.0
        u_NoiseStrength("表面扰动", Range(0.0, 1.0)) = 0.12
        u_Roughness("磨砂粗糙度", Range(0.0, 1.0)) = 0.0
        u_Opacity("透明度", Range(0.0, 1.0)) = 0.85
        u_Thickness("透镜厚度", Range(0.1, 3.0)) = 1.0
    }

    RenderCommand
    {
        Cull Off
        ZTest LEqual
        ZWrite Off
        Blend SrcAlpha OneMinusSrcAlpha
    }

    SubShader
    {
        Pass
        {
            Tags { "Queue" = "Transparent" "RenderType" = "Transparent" "LightMode" = "ForwardBase" }
            Name "ForwardBase"
            GLSL
            {
                #include "PrismEngine.glsl"
                #include "FastNoise/FastNoiseLite.glsl"

                attribute vec3 a_Position : POSITION;
                attribute vec3 a_Normal : NORMAL;
                attribute vec2 a_TexCoord : TEXCOORD0;

                layout(location = 0) out vec4 FragColor;

                varying VertexOutput
                {
                    vec3 WorldPosition;
                    vec3 Normal;
                    vec2 TexCoord;
                } vs_Output;

                const float PI = 3.14159265;

                // ================================================================
                //  Snell's Law refraction (dielectric)
                // ================================================================
                vec3 refractGlass(vec3 I, vec3 N, float eta)
                {
                    float cosI = clamp(dot(N, I), -1.0, 1.0);
                    float k = 1.0 - eta * eta * (1.0 - cosI * cosI);
                    if (k < 0.0) return vec3(0.0);
                    return eta * I - (eta * cosI + sqrt(k)) * N;
                }

                // ================================================================
                //  Fresnel: exact dielectric reflectance
                // ================================================================
                float fresnelDielectric(float cosTheta, float eta)
                {
                    float sin2T = 1.0 - cosTheta * cosTheta;
                    float cosT = sqrt(max(0.0, 1.0 - sin2T / (eta * eta)));
                    float rp = (eta * cosTheta - cosT) / (eta * cosTheta + cosT);
                    float rs = (cosTheta - eta * cosT) / (cosTheta + eta * cosT);
                    return clamp(rp * rp + rs * rs, 0.0, 1.0);
                }

                float fresnelSchlick(float cosTheta, float F0)
                {
                    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
                }

                // ================================================================
                //  Thin Film Interference — iridescent rainbow
                // ================================================================
                vec3 thinFilmInterference(float cosTheta, float thickness, float filmIOR)
                {
                    float cosF = sqrt(max(0.0, 1.0 - (1.0 - cosTheta * cosTheta) / (filmIOR * filmIOR)));
                    float path = 2.0 * filmIOR * thickness * cosF;
                    vec3 wavelengths = vec3(650.0, 510.0, 475.0);
                    vec3 phase = 2.0 * PI * path / wavelengths;
                    return 0.5 + 0.5 * cos(phase);
                }

                // ================================================================
                //  Beer-Lambert Absorption
                // ================================================================
                vec3 beerLambert(vec3 glassColor, float depth)
                {
                    vec3 sigma = -log(clamp(glassColor, 0.001, 1.0));
                    return exp(-sigma * depth);
                }

                // ================================================================
                //  Voronoi Caustic — dancing focused-light pattern
                // ================================================================
                float voronoiCaustic(vec2 uv, float time)
                {
                    vec2 n = floor(uv);
                    vec2 f = fract(uv);
                    float md = 5.0;
                    float md2 = 5.0;

                    for (int j = -1; j <= 1; j++)
                    {
                        for (int i = -1; i <= 1; i++)
                        {
                            vec2 g = vec2(float(i), float(j));
                            vec2 o = fract(sin(vec2(
                                dot(n + g, vec2(127.1, 311.7)),
                                dot(n + g, vec2(269.5, 183.3))
                            )) * 43758.5453);
                            o = 0.5 + 0.4 * sin(time + 6.2831 * o);
                            vec2 r = g + o - f;
                            float d = dot(r, r);

                            if (d < md) { md2 = md; md = d; }
                            else if (d < md2) { md2 = d; }
                        }
                    }

                    return pow(clamp(md2 - md, 0.0, 1.0), 0.8) * 2.0;
                }

                // ================================================================
                //  MSAA Resolve (8x)
                // ================================================================
                vec4 resolveMSAA(sampler2DMS tex, ivec2 coord)
                {
                    vec4 result = vec4(0.0);
                    for (int i = 0; i < 8; i++)
                        result += texelFetch(tex, coord, i);
                    return result / 8.0;
                }

                // ================================================================
                //  Sample MSAA texture at pixel offset (for chromatic aberration)
                // ================================================================
                vec3 sampleScenePixel(sampler2DMS tex, ivec2 coord)
                {
                    ivec2 c = clamp(coord, ivec2(0), textureSize(tex) - 1);
                    vec4 result = vec4(0.0);
                    for (int i = 0; i < 8; i++)
                        result += texelFetch(tex, c, i);
                    return (result / 8.0).rgb;
                }

                // ================================================================
                //  Screen UV → pixel coord
                // ================================================================
                vec2 worldToScreenUV(vec3 worldPos)
                {
                    vec4 clip = Prism_ViewProjection * vec4(worldPos, 1.0);
                    return (clip.xy / clip.w) * 0.5 + 0.5;
                }

                ivec2 screenUVToPixel(vec2 uv)
                {
                    vec2 res = textureSize(Prism_GeometryPassTexture);
                    return ivec2(uv * res);
                }

                // ================================================================
                //  Vertex Shader
                // ================================================================
                void vert()
                {
                    vec4 worldPos = Prism_Model * vec4(a_Position, 1.0);
                    gl_Position = Prism_ViewProjection * worldPos;
                    vs_Output.WorldPosition = worldPos.xyz;
                    vs_Output.Normal = normalize(mat3(Prism_Model) * a_Normal);
                    vs_Output.TexCoord = a_TexCoord;
                }

                // ================================================================
                //  Fragment Shader
                // ================================================================
                void frag()
                {
                    ivec2 pixelCoord = ivec2(gl_FragCoord.xy);
                    vec3 sceneColor = sampleScenePixel(Prism_GeometryPassTexture, pixelCoord);

                    vec3 worldPos = vs_Output.WorldPosition;
                    vec3 normal = normalize(vs_Output.Normal);
                    vec3 viewDir = normalize(Prism_CameraPosition - worldPos);
                    float NdotV = max(dot(normal, viewDir), 0.0);
                    vec2 screenUV = worldToScreenUV(worldPos);

                    // --- 1. Surface noise perturbation ---
                    fnl_state noiseState = fnlCreateState(42);
                    noiseState.noise_type = FNL_NOISE_OPENSIMPLEX2;
                    noiseState.fractal_type = FNL_FRACTAL_FBM;
                    noiseState.frequency = u_NoiseScale / 1000.0;
                    noiseState.octaves = 4;
                    noiseState.lacunarity = 2.0;
                    noiseState.gain = 0.5;

                    float nx = fnlGetNoise3D(noiseState, worldPos.x, worldPos.y + Prism_Time.x * 0.3, worldPos.z);
                    float nz = fnlGetNoise3D(noiseState, worldPos.z, worldPos.x + Prism_Time.x * 0.3, worldPos.y);
                    vec3 perturbedNormal = normalize(normal + vec3(nx, 0.0, nz) * u_NoiseStrength);

                    // --- 2. Snell's law per RGB (chromatic aberration) ---
                    float iorR = u_IOR - u_Aberration * 0.01;
                    float iorG = u_IOR;
                    float iorB = u_IOR + u_Aberration * 0.01;

                    vec3 refractedR = refractGlass(-viewDir, perturbedNormal, 1.0 / iorR);
                    vec3 refractedG = refractGlass(-viewDir, perturbedNormal, 1.0 / iorG);
                    vec3 refractedB = refractGlass(-viewDir, perturbedNormal, 1.0 / iorB);

                    vec2 uvR = worldToScreenUV(worldPos + refractedR * 0.2);
                    vec2 uvG = worldToScreenUV(worldPos + refractedG * 0.2);
                    vec2 uvB = worldToScreenUV(worldPos + refractedB * 0.2);

                    // --- 3. Roughness blur (frosted glass) ---
                    if (u_Roughness > 0.001)
                    {
                        float blur = u_Roughness * 0.008;
                        fnl_state blurState = fnlCreateState(7);
                        blurState.noise_type = FNL_NOISE_OPENSIMPLEX2;
                        blurState.frequency = 0.5;

                        float bx = fnlGetNoise3D(blurState, worldPos.x * 50.0, worldPos.y * 50.0, Prism_Time.x * 2.0);
                        float by = fnlGetNoise3D(blurState, worldPos.z * 50.0, worldPos.x * 50.0, Prism_Time.x * 2.0);
                        vec2 blurOffset = vec2(bx, by) * blur;

                        uvR += blurOffset;
                        uvG += blurOffset * 0.7;
                        uvB -= blurOffset;
                    }

                    // --- 4. Sample scene with per-channel pixel offset ---
                    ivec2 pixR = screenUVToPixel(clamp(uvR, 0.0, 1.0));
                    ivec2 pixG = screenUVToPixel(clamp(uvG, 0.0, 1.0));
                    ivec2 pixB = screenUVToPixel(clamp(uvB, 0.0, 1.0));

                    vec3 refractedColor;
                    refractedColor.r = sampleScenePixel(Prism_GeometryPassTexture, pixR).r;
                    refractedColor.g = sampleScenePixel(Prism_GeometryPassTexture, pixG).g;
                    refractedColor.b = sampleScenePixel(Prism_GeometryPassTexture, pixB).b;

                    // --- 5. Fresnel ---
                    float fresnel = fresnelDielectric(NdotV, 1.0 / u_IOR);
                    float fresnelEdge = fresnelSchlick(NdotV, 0.04);

                    // --- 6. Thin film interference ---
                    vec3 filmColor = thinFilmInterference(NdotV, u_FilmThickness, u_FilmIOR);
                    float filmInfluence = mix(0.0, 0.35, 1.0 - fresnelEdge);

                    // --- 7. Caustic pattern ---
                    vec2 causticUV = (screenUV - 0.5) * 2.0 * u_CausticScale;
                    float caustic = voronoiCaustic(causticUV, Prism_Time.x * u_CausticSpeed);
                    vec3 causticColor = vec3(caustic) * u_CausticIntensity;

                    // --- 8. Beer-Lambert absorption ---
                    vec2 distFromCenter = (vs_Output.TexCoord - 0.5) * 2.0;
                    float depth = u_Thickness * (1.0 - dot(distFromCenter, distFromCenter) * 0.5);
                    depth = max(depth, 0.05);
                    vec3 absorption = beerLambert(u_GlassColor, depth);

                    // --- 9. Fake environment specular ---
                    vec3 R = reflect(-viewDir, perturbedNormal);
                    vec3 envUp = mix(vec3(0.15, 0.2, 0.25), vec3(0.6, 0.75, 0.95), R.y * 0.5 + 0.5);
                    vec3 envHorizon = vec3(0.5, 0.55, 0.6);
                    vec3 envColor = mix(envHorizon, envUp, clamp(R.y * 0.5 + 0.5, 0.0, 1.0));

                    vec3 lightDir = normalize(-Prism_Lights[0].Direction);
                    vec3 halfDir = normalize(lightDir + viewDir);
                    float spec = pow(max(dot(perturbedNormal, halfDir), 0.0), 256.0);
                    vec3 specHighlight = vec3(1.0, 0.98, 0.95) * spec * 1.5;

                    // --- 10. Composite ---
                    vec3 glassBase = refractedColor * absorption;

                    glassBase += causticColor * glassBase;

                    glassBase = mix(glassBase, glassBase * filmColor, filmInfluence);

                    vec3 reflectedColor = envColor * fresnelEdge;
                    glassBase = mix(glassBase, reflectedColor + specHighlight, fresnelEdge);

                    float edgeGlow = pow(1.0 - NdotV, 3.0) * 0.15;
                    glassBase += u_GlassColor * edgeGlow;

                    float alpha = mix(u_Opacity, 1.0, fresnelEdge);

                    FragColor = vec4(glassBase, alpha);
                }
            }
        }
    }
}
