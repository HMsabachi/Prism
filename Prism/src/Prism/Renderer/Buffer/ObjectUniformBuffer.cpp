#include "prpch.h"
#include "ObjectUniformBuffer.h"
#include "Prism/Renderer/Renderer.h"
#include "Prism/ShaderCompiler/PrismBindings.h"

namespace Prism
{

    void ObjectUniformBuffer::Init()
    {
        m_Buffer = UniformBuffer::Create(sizeof(Data));
    }

    void ObjectUniformBuffer::SetModel(const glm::mat4& model)
    {
        m_Data.Model = model;
    }

    void ObjectUniformBuffer::SetBones(const glm::mat4* bones, uint32_t count)
    {
        // TODO: 这里可能有问题
        if (count > PRISM_MAX_BONES) count = PRISM_MAX_BONES;
        memcpy(m_Data.Bones, bones, count * sizeof(glm::mat4));
        m_BonesDirty = true;
    }

    void ObjectUniformBuffer::Upload()
    {
        if (m_BonesDirty)
            m_Buffer->SetData(&m_Data, sizeof(Data));
        else
            m_Buffer->SetData(&m_Data, offsetof(Data, Bones));
        m_BonesDirty = false;
    }

    void ObjectUniformBuffer::Bind() const
    {
        Renderer::SetUniformBuffer(Config::PRISM_SET_TRANSFORMS, 0, m_Buffer);
    }
}
