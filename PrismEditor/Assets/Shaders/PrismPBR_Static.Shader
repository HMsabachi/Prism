// Prism Shader Language v1.0
Shader "Custom/SimplePBR_Static"
{
    // ==================== Properties（材质参数） ====================
    Properties
    {

        u_Color("主色", Vector3) = (0.2, 0.2, 1)
        // BRDF LUT
        u_AlbedoColor("颜色", Vector3) = (1, 1, 1)
        u_BRDFLUTTexture("BRDF LUT", Texture2D) = {}
        u_Metalness("金属度", Range(0, 1)) = 1.0
        u_Roughness("粗糙度", Range(0, 1)) = 0.0
        u_EnvMapRotation("环境光旋转", Range(0, 360)) = 0

        // PBR texture inputs
        u_AlbedoTexture("颜色贴图", Texture2D) = {}
        u_NormalTexture("法线贴图", Texture2D) = {}
        u_MetalnessTexture("金属度贴图", Texture2D) = {}
        u_RoughnessTexture("粗糙度贴图", Texture2D) = {}

        // Environment maps
        u_EnvRadianceTex("环境光贴图", TextureCube) = {}
        u_EnvIrradianceTex("环境光辐射贴图", TextureCube) = {}


        // Toggles
        u_RadiancePrefilter("环境光预过滤", Float) = 0.0
        u_AlbedoTexToggle("颜色贴图开关", Float) = 0.0
        u_NormalTexToggle("法线贴图开关", Float) = 0.0
        u_MetalnessTexToggle("金属度贴图开关", Float) = 0.0
        u_RoughnessTexToggle("粗糙度贴图开关", Float) = 0.0

        // light
        u_LightDirection("光照方向", Vector3) = (1, 1, 1)
        u_LightRadiance("光照颜色", Vector3) = (1, 1, 1)

    }

    RenderCommand
    {
        Cull Back
        ZTest LEqual
        ZWrite On
        Blend One OneMinusSrcAlpha
    }

    SubShader
    {
        
        Pass
        {
            Tags { "Queue" = "Geometry" "RenderType" = "Opaque" }
            Name "ForwardBase"
            GLSL
            {
                #include "PrismBuiltin.glsl"
                #include "PrismPBR.glsl"
                attribute vec3 a_Position : POSITION;
                attribute vec3 a_Normal : NORMAL;
                attribute vec3 a_Tangent : TANGENT;
                attribute vec3 a_Binormal : BINORMAL;
                attribute vec2 a_TexCoord : TEXCOORD0;


                VARYING VertexOutput
                {
                    vec3 WorldPosition;
                    vec3 Normal;
                    vec2 TexCoord;
                    mat3 WorldNormals;
                    mat3 WorldTransform;
                    vec3 Binormal;
                } vs_Output;

                void main()
                {
                    mat4 u_ViewProjectionMatrix = Prism_ViewProjection;
                    mat4 u_ModelMatrix = Prism_Model;
                    vs_Output.WorldPosition = vec3(u_ModelMatrix * vec4(a_Position, 1.0));
                    vs_Output.Normal = a_Normal;
                    vs_Output.TexCoord = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);
                    vs_Output.WorldNormals = mat3(u_ModelMatrix) * mat3(a_Tangent, a_Binormal, a_Normal);
                    vs_Output.WorldTransform = mat3(u_ModelMatrix);
                    vs_Output.Binormal = a_Binormal;

                    gl_Position = u_ViewProjectionMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
                }


                const int LightCount = 1;

                // Constant normal incidence Fresnel factor for all dielectrics.
                const vec3 Fdielectric = vec3(0.04);

                struct Light {
                    vec3 Direction;
                    vec3 Radiance;
                };
                uniform Light lights;

                struct PBRParameters
                {
                    vec3 Albedo;
                    float Roughness;
                    float Metalness;

                    vec3 Normal;
                    vec3 View;
                    float NdotV;
                };

                PBRParameters m_Params;

                // ---------------------------------------------------------------------------------------------------

                vec3 RotateVectorAboutY(float angle, vec3 vec)
                {
                    angle = radians(angle);
                    mat3x3 rotationMatrix ={vec3(cos(angle),0.0,sin(angle)),
                                            vec3(0.0,1.0,0.0),
                                            vec3(-sin(angle),0.0,cos(angle))};
                    return rotationMatrix * vec;
                }

                vec3 Lighting(vec3 F0)
                {
                    vec3 result = vec3(0.0);
                    for(int i = 0; i < LightCount; i++)
                    {
                        vec3 Li = -u_LightDirection;
                        vec3 Lradiance = u_LightRadiance;
                        vec3 Lh = normalize(Li + m_Params.View);

                        // Calculate angles between surface normal and various light vectors.
                        float cosLi = max(0.0, dot(m_Params.Normal, Li));
                        float cosLh = max(0.0, dot(m_Params.Normal, Lh));

                        vec3 F = fresnelSchlick(F0, max(0.0, dot(Lh, m_Params.View)));
                        float D = ndfGGX(cosLh, m_Params.Roughness);
                        float G = gaSchlickGGX(cosLi, m_Params.NdotV, m_Params.Roughness);

                        vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
                        vec3 diffuseBRDF = kd * m_Params.Albedo;

                        // Cook-Torrance
                        vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * m_Params.NdotV);

                        result += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
                    }
                    return result;
                }

                vec3 IBL(vec3 F0, vec3 Lr)
                {
                    vec3 irradiance = texture(u_EnvIrradianceTex, m_Params.Normal).rgb;
                    vec3 F = fresnelSchlickRoughness(F0, m_Params.NdotV, m_Params.Roughness);
                    vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
                    vec3 diffuseIBL = m_Params.Albedo * irradiance;

                    int u_EnvRadianceTexLevels = textureQueryLevels(u_EnvRadianceTex);
                    float NoV = clamp(m_Params.NdotV, 0.0, 1.0);
                    vec3 R = 2.0 * dot(m_Params.View, m_Params.Normal) * m_Params.Normal - m_Params.View;
                    
                    vec3 specularIrradiance = textureLod(u_EnvRadianceTex, RotateVectorAboutY(u_EnvMapRotation, Lr), (m_Params.Roughness * m_Params.Roughness) * u_EnvRadianceTexLevels).rgb;
                    // Sample BRDF Lut, 1.0 - roughness for y-coord because texture was generated (in Sparky) for gloss model
                    vec2 specularBRDF = texture(u_BRDFLUTTexture, vec2(m_Params.NdotV, 1.0 - m_Params.Roughness)).rg;
                    vec3 specularIBL = specularIrradiance * (F * specularBRDF.x + specularBRDF.y);

                    return kd * diffuseIBL + specularIBL;
                }

                void frag()
                {
                    vec3 u_CameraPosition = Prism_CameraPosition;
                    // Standard PBR inputs
                    m_Params.Albedo = u_AlbedoTexToggle > 0.5 ? texture(u_AlbedoTexture, vs_Output.TexCoord).rgb : u_AlbedoColor; 
                    m_Params.Metalness = u_MetalnessTexToggle > 0.5 ? texture(u_MetalnessTexture, vs_Output.TexCoord).r : u_Metalness;
                    m_Params.Roughness = u_RoughnessTexToggle > 0.5 ?  texture(u_RoughnessTexture, vs_Output.TexCoord).r : u_Roughness;
                    m_Params.Roughness = max(m_Params.Roughness, 0.05); // Minimum roughness of 0.05 to keep specular highlight

                    m_Params.Albedo = mix(m_Params.Albedo, u_Color, 0.2);
                    // Normals (either from vertex or map)
                    m_Params.Normal = normalize(vs_Output.Normal);
                    if (u_NormalTexToggle > 0.5)
                    {
                        m_Params.Normal = normalize(2.0 * texture(u_NormalTexture, vs_Output.TexCoord).rgb - 1.0);
                        m_Params.Normal = normalize(vs_Output.WorldNormals * m_Params.Normal);
                    }

                    m_Params.View = normalize(u_CameraPosition - vs_Output.WorldPosition);
                    m_Params.NdotV = max(dot(m_Params.Normal, m_Params.View), 0.0);
                        
                    // Specular reflection vector
                    vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;

                    // Fresnel reflectance, metals use albedo
                    vec3 F0 = mix(Fdielectric, m_Params.Albedo, m_Params.Metalness);

                    vec3 lightContribution = Lighting(F0);
                    vec3 iblContribution = IBL(F0, Lr);

                    FragColor = vec4(lightContribution + iblContribution, 1.0);
                    //FragColor = vec4(u_Color, 1.0);
                }
                
            }
        }
    }
}