#pragma once
#include "Prism/Renderer/Material.h"

namespace Prism
{
    class OpenGLUniformBuffer;
}

namespace Prism
{
    class OpenGLMaterialBackend : public MaterialBackend
    {
    public:
        OpenGLMaterialBackend(const WeakRef<Material>& material);
        virtual ~OpenGLMaterialBackend();
        virtual void OnAllocate() override;

        const Ref<OpenGLUniformBuffer>& RT_GetUniformBuffer() const;
    private:
        mutable Ref<OpenGLUniformBuffer> m_UniformBuffer;
        const WeakRef<Material> m_Material;
    };
}
