#include "prpch.h"
#include "Material.h"
#include "Prism/Shader/PSL/PrismBindings.h"
#include "Prism/Utilities/Scalar.h"

namespace Prism
{

    Ref<Material> Material::Create(const Ref<PrismShader>& shader)
    {
        return Ref<Material>::Create(shader);
    }

    Material::Material(const Ref<PrismShader>& shader)
        : m_Shader(shader)
    {
        m_Shader->AddShaderReloadedCallback(std::bind(&Material::OnShaderReloaded, this));
        AllocateStorage();
    }

    Material::~Material()
    {
    }

    void Material::AllocateStorage()
    {
        auto& layout = m_Shader->GetMaterialLayout();
        uint32_t totalSize = layout.GetTotalSize();

        m_PropertyBuffer.Allocate(totalSize);

        for (auto& uni : m_Shader->GetUniforms())
        {
            if (PSL::PropertyTypeUtil::IsTextureType(uni.Type))
                continue;
            uint32_t offset = (uint32_t)uni.BufferOffset;
            uint32_t size = (uint32_t)uni.BufferSize;
            if (size == 0) continue;
            if (!uni.DefaultValue.empty())
            {
                const void* src = &uni.DefaultValue[0];
                uint32_t copySize = (uint32_t)(uni.DefaultValue.size() * sizeof(Scalar));
                if (copySize > size) copySize = size;
                m_PropertyBuffer.Write((const byte*)src, copySize, offset);
            }
        }

        m_UniformBuffer = UniformBuffer::Create(PSL::PRISM_BINDING_MATERIAL, totalSize);
        m_UniformBuffer->SetData(m_PropertyBuffer);

        m_Textures.clear();
        uint32_t maxTexSlot = 0;
        for (auto& uni : m_Shader->GetUniforms())
            if (uni.TextureSlot > (int32_t)maxTexSlot)
                maxTexSlot = (uint32_t)uni.TextureSlot;
        m_Textures.resize(maxTexSlot + 1);

        m_Dirty = false;
    }

    void Material::OnShaderReloaded()
    {
        AllocateStorage();
        for (auto mi : m_MaterialInstances)
            mi->OnShaderReloaded();
    }

    void Material::Bind()
    {
        BindPass(0);
    }

    void Material::BindPass(uint32_t passIndex)
    {
        Ref<Shader> program = m_Shader->GetPassProgram(passIndex, m_KeywordMask);
        program->Bind();
        program->ApplyCommand(m_Shader->GetPass(passIndex).Command);

        if (m_Dirty)
        {
            m_UniformBuffer->SetData(m_PropertyBuffer);
            m_Dirty = false;
        }
        m_UniformBuffer->Bind();
        BindTextures();
    }

    void Material::BindTextures()
    {
		for (size_t i = 0; i < m_Textures.size(); i++)
        {
            if (m_Textures[i])
                m_Textures[i]->Bind((uint32_t)i + PSL::PRISM_BINDING_TEXTURE);
        }
    }

    void Material::SetKeyword(const std::string& name, bool enabled)
    {
        if (!m_Shader->IsKeywordDefined(name))
        {
            PR_CORE_WARN("Keyword '{0}' not defined in shader '{1}'", name, m_Shader->GetName());
            return;
        }
        uint8_t index = m_Shader->GetKeywordIndex(name);
        if (enabled)
            m_KeywordMask |= (KeywordMask(1) << index);
        else
            m_KeywordMask &= ~(KeywordMask(1) << index);
    }

    bool Material::IsKeywordEnabled(const std::string& name) const
    {
        if (!m_Shader->IsKeywordDefined(name))
            return false;
        uint8_t index = m_Shader->GetKeywordIndex(name);
        return (m_KeywordMask & (KeywordMask(1) << index)) != 0;
    }

    // MaterialInstance

    Ref<MaterialInstance> MaterialInstance::Create(const Ref<Material>& material, const std::string& name)
    {
        return Ref<MaterialInstance>::Create(material, name);
    }

    MaterialInstance::MaterialInstance(const Ref<Material>& material, const std::string& name)
        : m_Material(material), m_Name(name)
    {
        m_Material->m_MaterialInstances.insert(this);
        m_KeywordMask = m_Material->m_KeywordMask;
        AllocateStorage();
    }

    MaterialInstance::~MaterialInstance()
    {
        m_Material->m_MaterialInstances.erase(this);
    }

    void MaterialInstance::SetShader(const Ref<PrismShader>& shader)
    {
        m_Material->m_MaterialInstances.erase(this);
        m_Material = Material::Create(shader);
        m_Material->m_MaterialInstances.insert(this);
        AllocateStorage();
        m_OverriddenValues.clear();
        m_KeywordMask = 0;
    }

    void MaterialInstance::OnShaderReloaded()
    {
        AllocateStorage();
        m_OverriddenValues.clear();
    }

    void MaterialInstance::AllocateStorage()
    {
        auto& layout = m_Material->m_Shader->GetMaterialLayout();
        uint32_t totalSize = layout.GetTotalSize();
        m_PropertyBuffer.Allocate(totalSize);

        for (auto& uni : m_Material->m_Shader->GetUniforms())
        {
            if (PSL::PropertyTypeUtil::IsTextureType(uni.Type))
                continue;
            uint32_t offset = (uint32_t)uni.BufferOffset;
            uint32_t size = (uint32_t)uni.BufferSize;
            if (size == 0) continue;
            if (!uni.DefaultValue.empty())
            {
                const void* src = &uni.DefaultValue[0];
                uint32_t copySize = (uint32_t)(uni.DefaultValue.size() * sizeof(Scalar));
                if (copySize > size) copySize = size;
                m_PropertyBuffer.Write((const byte*)src, copySize, offset);
            }
        }
    }

    void MaterialInstance::OnMaterialValueUpdated(const PSL::AST::ShaderUniform* uni)
    {
        if (m_OverriddenValues.find(uni->Name) == m_OverriddenValues.end())
        {
            auto& materialBuffer = m_Material->m_PropertyBuffer;
            m_PropertyBuffer.Write(materialBuffer.Data + uni->BufferOffset, uni->BufferSize, uni->BufferOffset);
        }
    }

    void MaterialInstance::SetKeyword(const std::string& name, bool enabled)
    {
        auto shader = m_Material->m_Shader;
        if (!shader->IsKeywordDefined(name))
        {
            PR_CORE_WARN("Keyword '{0}' not defined in shader '{1}'", name, shader->GetName());
            return;
        }
        uint8_t index = shader->GetKeywordIndex(name);
        if (enabled)
            m_KeywordMask |= (KeywordMask(1) << index);
        else
            m_KeywordMask &= ~(KeywordMask(1) << index);
    }

    bool MaterialInstance::IsKeywordEnabled(const std::string& name) const
    {
        auto shader = m_Material->m_Shader;
        if (!shader->IsKeywordDefined(name))
            return false;
        uint8_t index = shader->GetKeywordIndex(name);
        return (m_KeywordMask & (KeywordMask(1) << index)) != 0;
    }

    void MaterialInstance::Bind()
    {
        BindPass(0);
    }

    void MaterialInstance::BindPass(uint32_t passIndex)
    {
        auto shader = m_Material->m_Shader;
        Ref<Shader> program = shader->GetPassProgram(passIndex, m_KeywordMask);
        program->Bind();
        program->ApplyCommand(shader->GetPass(passIndex).Command);

        if (m_Material->m_Dirty)
        {
            m_Material->m_UniformBuffer->SetData(m_Material->m_PropertyBuffer);
            m_Material->m_Dirty = false;
        }

        Buffer merged;
        merged.Allocate(m_Material->m_PropertyBuffer.GetSize());
        memcpy(merged.Data, m_Material->m_PropertyBuffer.Data, m_Material->m_PropertyBuffer.GetSize());
        for (auto& key : m_OverriddenValues)
        {
            auto* uni = shader->FindUniform(key);
            if (uni && !PSL::PropertyTypeUtil::IsTextureType(uni->Type))
            {
                merged.Write(m_PropertyBuffer.Data + uni->BufferOffset,
                    uni->BufferSize, uni->BufferOffset);
            }
        }
        m_Material->m_UniformBuffer->SetData(merged);

        m_Material->m_UniformBuffer->Bind();

        m_Material->BindTextures();
        for (size_t i = 0; i < m_Textures.size(); i++)
        {
            if (m_Textures[i])
                m_Textures[i]->Bind((uint32_t)i + PSL::PRISM_BINDING_TEXTURE);
        }
    }

}
