#pragma once
#include "Prism/Renderer/Shader/GlobalUniforms.h"



namespace Prism
{
    class OpenGLGlobalUniform : public GlobalUniforms
    {
    public:
        OpenGLGlobalUniform();
        virtual ~OpenGLGlobalUniform();
    private:
        uint32_t m_GlobalUBO = 0;                    // PrismGlobals UBO
        void CreateGlobalUniformBuffer();
    public:
        virtual void UpdateGlobalUniformBuffer(const PrismGlobalsUBO& globalUniforms) const override;
    };
}