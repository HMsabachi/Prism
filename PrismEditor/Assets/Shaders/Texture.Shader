// Prism Shader Language v1.0
Shader "Custom/Texture"
{
    // ==================== Properties（材质参数） ====================
    Properties
    {
        _MainColor("主颜色", Vector4) = (1, 1, 1, 1)
        _MainTex("基础贴图", Texture2D) = "white" {}
        _Gloss("光泽度", Float) = 0.5
        _Cutoff("透明裁剪", Range(0, 1)) = 0.5
        _TillingFactor("平铺因子", Float) = 1.0
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
                VARYING vec2 vUV; 
                VARYING vec3 vNormal;
                void main()
                {
                    gl_Position = Prism_ViewProjection * Prism_Model * vec4(aPos, 1.0); 
                    vUV = aUV;
                }

                void frag()
                {
                    FragColor = texture(_MainTex, vUV * _TillingFactor) * _MainColor;
                }
            }
        }
    }
}
