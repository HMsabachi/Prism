#include "prpch.h"
#include "VulkanMaterialBackend.h"

#include "VulkanUniformBuffer.h"
#include "VulkanTexture.h"
#include "Prism/Renderer/Texture.h"

namespace Prism
{
    VulkanMaterialBackend::VulkanMaterialBackend(const WeakRef<Material> material)
        : m_Material(material)
    {
    }

    VulkanMaterialBackend::~VulkanMaterialBackend()
    {
    }

    void VulkanMaterialBackend::OnAllocate()
    {
        m_UniformBuffer = Ref<VulkanUniformBuffer>::Create((uint32_t)m_Material->m_PropertyBuffer.GetSize());

        m_DescriptorSet.Reset();
        m_DescriptorSet.SetInput(0, m_UniformBuffer);
        SetTextureInputs();
        m_DescriptorSet.Bake();

        m_Material->m_DataDirty = true;
        m_Material->m_TexturesDirty = true;
    }

    VkDescriptorSet VulkanMaterialBackend::RT_GetDescriptorSet() const
    {
        if (m_Material->m_DataDirty)
        {
            m_UniformBuffer->RT_SetDataAll(m_Material->m_PropertyBuffer);
            m_Material->m_DataDirty = false;
        }
        if (m_Material->m_TexturesDirty)
        {
            SetTextureInputs();
            m_Material->m_TexturesDirty = false;
        }
        m_DescriptorSet.RT_Prepare();
        return m_DescriptorSet.RT_GetDescriptorSet();
    }

    void VulkanMaterialBackend::SetTextureInputs() const
    {
        for (const auto& uniform : m_Material->GetShader()->GetUniforms())
        {
            if (!PrismShaderCompiler::PropertyTypeUtil::IsTextureType(uniform.Type) || uniform.TextureSlot < 0)
                continue;

            uint32_t binding = (uint32_t)uniform.TextureSlot + 1;
            auto it = m_Material->m_Textures.find((uint32_t)uniform.TextureSlot);
            const Ref<Texture>& texture = (it != m_Material->m_Textures.end()) ? it->second : Ref<Texture>();

            if (!texture)
            {
                if (uniform.Type == PrismShaderCompiler::PropertyType::TextureCube)
                    m_DescriptorSet.SetInput(binding, Ref<VulkanImageCube>());
                else
                    m_DescriptorSet.SetInput(binding, Ref<VulkanImage2D>());
                continue;
            }

            if (texture->GetType() == TextureType::Texture2D)
                m_DescriptorSet.SetInput(binding, texture.As<VulkanTexture2D>()->GetImage().As<VulkanImage2D>());
            else if (texture->GetType() == TextureType::TextureCube)
                m_DescriptorSet.SetInput(binding, texture.As<VulkanTextureCube>()->GetImage().As<VulkanImageCube>());
        }
    }
}
