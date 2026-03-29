// Prism Shader Language v1.0
Shader "Prism/StandardLit"
{
    Properties
    {
        _MainColor("主颜色", Color) = (1, 1, 1, 1)
        _MainTex("基础贴图 (Albedo)", Texture2D) = "white" {}
        _NormalTex("法线贴图", Texture2D) = "normal" {}
        _Metallic("金属度", Range(0, 1)) = 0.0
        _Smoothness("光滑度", Range(0, 1)) = 0.5
        _Cutoff("透明裁剪阈值", Range(0, 1)) = 0.5
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

                attribute vec3 aPos : POSITION;
                attribute vec2 aUV : TEXCOORD0;
                attribute vec3 aNormal : NORMAL;
                attribute vec3 aTangent : TANGENT;

                VARYING vec2 vUV;
                VARYING vec3 vNormal;
                VARYING vec3 vWorldPos;

                void main()
                {
                    gl_Position = Prism_ViewProjection * Prism_Model * vec4(aPos, 1.0);
                    
                    vUV = aUV;
                    vNormal = mat3(Prism_Model) * aNormal;   // 法线转换到世界空间
                    vWorldPos = (Prism_Model * vec4(aPos, 1.0)).xyz;
                }

                void frag()
                {
                    // 示例：简单颜色 + 时间闪烁（可替换为 PBR 计算）
                    vec4 albedo = texture(_MainTex, vUV) * _MainColor;
                    
                    // 简单光照模拟
                    vec3 lightDir = normalize(vec3(1.0, 1.0, -1.0));
                    float ndotl = max(dot(normalize(vNormal), lightDir), 0.0);
                    
                    vec3 color = albedo.rgb * (ndotl * 0.8 + 0.2);
                    
                    // 时间动画示例
                    color.r += sin(Prism_Time.x * 3.0) * 0.1;
                    
                    FragColor = vec4(color, albedo.a);
                }
            }
        }
    }
}