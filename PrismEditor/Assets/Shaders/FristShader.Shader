// Prism Shader Language v1.0
Shader "Custom/MyFirstShader"
{
    // ==================== Properties（材质参数） ====================
    Properties
    {
        _MainColor("主颜色", Color) = (1, 1, 1, 1)
        _MainTex("基础贴图", Texture2D) = "white" {}
        _Gloss("光泽度", Float) = 0.5
        _Cutoff("透明裁剪", Range(0, 1)) = 0.5
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
                VARYING vec2 vUV; // Prism 引擎定义的跨阶段变量
                VARYING vec3 vNormal;
                void main()
                {
                    gl_Position = Prism_ViewProjection * Prism_Model * vec4(aPos, 1.0); // Prism_ 前缀是引擎全局变量
                    vUV = aUV;
                }

                void frag()
                {
                    vec4 col = vec4(1.0, 1.0, 1.0, 1.0);
					col.r *= sin(Prism_Time.x * 2.0) * 0.5 + 0.5;
					col.g *= sin(Prism_Time.y * 2.0) * 0.5 + 0.5;
					col.b *= sin(Prism_Time.z * 2.0) * 0.5 + 0.5;
					FragColor = col;
                }
            }
        }
    }
}
