Shader "Hidden/ShadowDepth"
{
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

                uniform mat4 u_LightVP;

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
