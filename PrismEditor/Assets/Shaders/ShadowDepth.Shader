Shader "Hidden/ShadowDepth"
{
    Properties
    {
        u_LightVP("灯光矩阵", Matrix4) = 0
    }
    RenderCommand
    {
        Cull Back
        ZWrite On
        ZTest LEqual
    }

    SubShader
    {
        Pass
        {
            Name "ShadowCaster"
            GLSL
            {
                #include "PrismCommon.glsl"

                attribute vec3 a_Position : POSITION;

                void main()
                {
                    gl_Position = u_LightVP * Prism_Model * vec4(a_Position, 1.0);
                }

                void frag()
                {
                }
            }
        }
    }
}
