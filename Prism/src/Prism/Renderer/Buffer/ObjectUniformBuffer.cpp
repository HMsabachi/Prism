#include "prpch.h"
#include "ObjectUniformBuffer.h"
#include "Prism/Shader/PSL/PrismBindings.h"

namespace Prism
{

    void ObjectUniformBuffer::Init()
    {
        m_Buffer = UniformBuffer::Create(PSL::PRISM_BINDING_OBJECT, sizeof(Data));
    }

    void ObjectUniformBuffer::SetModel(const glm::mat4& model)
    {
        m_Data.Model = model;
    }

    void ObjectUniformBuffer::SetBones(const glm::mat4* bones, uint32_t count)
    {
        if (count > PRISM_MAX_BONES) count = PRISM_MAX_BONES;
        for (uint32_t i = 0; i < count; i++)
            m_Data.Bones[i] = bones[i];
    }

    void ObjectUniformBuffer::Upload()
    {
        m_Buffer->SetData(&m_Data, sizeof(Data));
    }

    void ObjectUniformBuffer::Bind() const
    {
        m_Buffer->Bind();
    }
}
