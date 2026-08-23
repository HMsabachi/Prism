#include "prpch.h"
#include "Buffer/OpenGLUniformBuffer.h"
#include "OpenGLMaterialBackend.h"

namespace Prism
{


    OpenGLMaterialBackend::OpenGLMaterialBackend(const WeakRef<Material>& material)
        : m_Material(material)
    {

    }
    OpenGLMaterialBackend::~OpenGLMaterialBackend()
    {

    }


    void OpenGLMaterialBackend::OnAllocate()
    {
        m_UniformBuffer = Ref<OpenGLUniformBuffer>::Create((uint32_t)m_Material->m_PropertyBuffer.GetSize());
    }


    const Ref<OpenGLUniformBuffer>& OpenGLMaterialBackend::RT_GetUniformBuffer() const
    {
        if (!m_Material->m_DataDirty) return m_UniformBuffer;
        m_UniformBuffer->RT_SetData(m_Material->m_PropertyBuffer);
        m_Material->m_DataDirty = false;
        return m_UniformBuffer;
    }

}
