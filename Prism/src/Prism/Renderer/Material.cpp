#include "prpch.h"
#include "Material.h"
#include "Prism/ShaderCompiler/PrismBindings.h"

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

    void Material::WriteUniform(const std::string& name, const void* data, uint32_t size)
    {
        auto* uni = m_Shader->FindUniform(name);
        if (!uni) { PR_CORE_WARN("Material - uniform '{0}' not found", name); return; }
        m_PropertyBuffer.Write((const byte*)data, size, uni->BufferOffset);
        m_Dirty = true;
    }

    void Material::AllocateStorage()
    {
        auto& layout = m_Shader->GetMaterialLayout();
        uint32_t totalSize = layout.GetTotalSize();
        m_PropertyBuffer.Allocate(totalSize);

        for (auto& uni : m_Shader->GetUniforms())
        {
            if (PrismShaderCompiler::PropertyTypeUtil::IsTextureType(uni.Type))
                continue;
            uint32_t offset = (uint32_t)uni.BufferOffset;
            uint32_t size = (uint32_t)uni.BufferSize;
            if (size == 0) continue;
            if (!uni.DefaultValue.empty())
            {
                uint32_t copySize = (uint32_t)(uni.DefaultValue.size() * sizeof(PrismShaderCompiler::Scalar));
                if (copySize > size) copySize = size;
                m_PropertyBuffer.Write((const byte*)&uni.DefaultValue[0], copySize, offset);
            }
        }

        m_UniformBuffer = UniformBuffer::Create(Prism::Config::PRISM_BINDING_MATERIAL, totalSize);
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
    }

    void Material::SetShader(const Ref<PrismShader>& shader)
    {
        m_Shader = shader;
        m_Shader->AddShaderReloadedCallback(std::bind(&Material::OnShaderReloaded, this));
        AllocateStorage();
        m_KeywordMask = 0;
    }

#pragma region Setters

    void Material::SetBool(const std::string& name, bool value)
    {
        WriteUniform(name, &value, sizeof(bool));
    }

    void Material::SetInt(const std::string& name, int value)
    {
        WriteUniform(name, &value, sizeof(int));
    }

    void Material::SetFloat(const std::string& name, float value)
    {
        WriteUniform(name, &value, sizeof(float));
    }

    void Material::SetVec2(const std::string& name, const glm::vec2& value)
    {
        WriteUniform(name, &value, sizeof(glm::vec2));
    }

    void Material::SetVec3(const std::string& name, const glm::vec3& value)
    {
        WriteUniform(name, &value, sizeof(glm::vec3));
    }

    void Material::SetVec4(const std::string& name, const glm::vec4& value)
    {
        WriteUniform(name, &value, sizeof(glm::vec4));
    }

    void Material::SetColor3(const std::string& name, const glm::vec3& value)
    {
        WriteUniform(name, &value, sizeof(glm::vec3));
    }

    void Material::SetColor(const std::string& name, const glm::vec4& value)
    {
        WriteUniform(name, &value, sizeof(glm::vec4));
    }

    void Material::SetMatrix3(const std::string& name, const glm::mat3& value)
    {
        WriteUniform(name, &value, sizeof(glm::mat3));
    }

    void Material::SetMatrix4(const std::string& name, const glm::mat4& value)
    {
        WriteUniform(name, &value, sizeof(glm::mat4));
    }

    void Material::SetTexture(const std::string& name, const Ref<Texture2D>& texture)
    {
        auto* uni = m_Shader->FindUniform(name);
        if (!uni || uni->TextureSlot < 0) { PR_CORE_WARN("Material::SetTexture - '{0}' not found", name); return; }
        uint32_t slot = (uint32_t)uni->TextureSlot - Prism::Config::PRISM_BINDING_TEXTURE;
        if (m_Textures.size() <= slot) m_Textures.resize(slot + 1);
        m_Textures[slot] = texture;
    }

    void Material::SetTexture(const std::string& name, const Ref<TextureCube>& texture)
    {
        auto* uni = m_Shader->FindUniform(name);
        if (!uni || uni->TextureSlot < 0) { PR_CORE_WARN("Material::SetTexture - '{0}' not found", name); return; }
        uint32_t slot = (uint32_t)uni->TextureSlot - Prism::Config::PRISM_BINDING_TEXTURE;
        if (m_Textures.size() <= slot) m_Textures.resize(slot + 1);
        m_Textures[slot] = texture;
    }

#pragma endregion

#pragma region Getters

    bool Material::GetBool(const std::string& name) const
    {
        auto* uni = m_Shader->FindUniform(name);
        if (!uni) return false;
        return m_PropertyBuffer.Read<bool>(uni->BufferOffset);
    }

    int Material::GetInt(const std::string& name) const
    {
        auto* uni = m_Shader->FindUniform(name);
        if (!uni) return 0;
        return m_PropertyBuffer.Read<int>(uni->BufferOffset);
    }

    float Material::GetFloat(const std::string& name) const
    {
        auto* uni = m_Shader->FindUniform(name);
        if (!uni) return 0.0f;
        return m_PropertyBuffer.Read<float>(uni->BufferOffset);
    }

    glm::vec2 Material::GetVec2(const std::string& name) const
    {
        auto* uni = m_Shader->FindUniform(name);
        if (!uni) return glm::vec2(0.0f);
        return m_PropertyBuffer.Read<glm::vec2>(uni->BufferOffset);
    }

    glm::vec3 Material::GetVec3(const std::string& name) const
    {
        auto* uni = m_Shader->FindUniform(name);
        if (!uni) return glm::vec3(0.0f);
        return m_PropertyBuffer.Read<glm::vec3>(uni->BufferOffset);
    }

    glm::vec4 Material::GetVec4(const std::string& name) const
    {
        auto* uni = m_Shader->FindUniform(name);
        if (!uni) return glm::vec4(0.0f);
        return m_PropertyBuffer.Read<glm::vec4>(uni->BufferOffset);
    }

    glm::vec3 Material::GetColor3(const std::string& name) const
    {
        auto* uni = m_Shader->FindUniform(name);
        if (!uni) return glm::vec3(0.0f);
        return m_PropertyBuffer.Read<glm::vec3>(uni->BufferOffset);
    }

    glm::vec4 Material::GetColor(const std::string& name) const
    {
        auto* uni = m_Shader->FindUniform(name);
        if (!uni) return glm::vec4(0.0f);
        return m_PropertyBuffer.Read<glm::vec4>(uni->BufferOffset);
    }

    glm::mat3 Material::GetMatrix3(const std::string& name) const
    {
        auto* uni = m_Shader->FindUniform(name);
        if (!uni) return glm::mat3(1.0f);
        return m_PropertyBuffer.Read<glm::mat3>(uni->BufferOffset);
    }

    glm::mat4 Material::GetMatrix4(const std::string& name) const
    {
        auto* uni = m_Shader->FindUniform(name);
        if (!uni) return glm::mat4(1.0f);
        return m_PropertyBuffer.Read<glm::mat4>(uni->BufferOffset);
    }

    Ref<Texture2D> Material::GetTexture2D(const std::string& name) const
    {
        auto* uni = m_Shader->FindUniform(name);
        if (!uni || uni->TextureSlot < 0 || uni->Type != PrismShaderCompiler::PropertyType::Texture2D) return nullptr;
        uint32_t slot = (uint32_t)uni->TextureSlot - Prism::Config::PRISM_BINDING_TEXTURE;
        if (slot >= m_Textures.size()) return nullptr;
        return m_Textures[slot].As<Texture2D>();
    }

    Ref<TextureCube> Material::GetTextureCube(const std::string& name) const
    {
        auto* uni = m_Shader->FindUniform(name);
        if (!uni || uni->TextureSlot < 0 || uni->Type != PrismShaderCompiler::PropertyType::TextureCube) return nullptr;
        uint32_t slot = (uint32_t)uni->TextureSlot - Prism::Config::PRISM_BINDING_TEXTURE;
        if (slot >= m_Textures.size()) return nullptr;
        return m_Textures[slot].As<TextureCube>();
    }

    bool Material::HasProperty(const std::string& name) const
    {
        return m_Shader->FindUniform(name) != nullptr;
    }

#pragma endregion

    Ref<Shader> Material::GetProgram(uint32_t passIndex)
    {
        return m_Shader->GetPassProgram(passIndex, m_KeywordMask);
    }

    void Material::BindProgram(uint32_t passIndex)
    {
        Ref<Shader> program = m_Shader->GetPassProgram(passIndex, m_KeywordMask);
        program->Bind();
        program->ApplyCommand(m_Shader->GetPass(passIndex).Command);
    }

    void Material::BindUniform()
    {
        if (m_Dirty)
        {
            m_UniformBuffer->SetData(m_PropertyBuffer);
            m_Dirty = false;
        }
        m_UniformBuffer->Bind();
    }

    void Material::BindTexture()
    {
        for (size_t i = 0; i < m_Textures.size(); i++)
        {
            if (m_Textures[i])
                m_Textures[i]->Bind((uint32_t)i + Prism::Config::PRISM_BINDING_TEXTURE);
        }
    }

    void Material::Bind(uint32_t passIndex)
    {
        BindProgram(passIndex);
        BindUniform();
        BindTexture();
    }

    void Material::SetKeyword(const std::string& name, bool enabled)
    {
        if (!m_Shader->IsKeywordDefined(name))
        {
            PR_CORE_WARN("Keyword '{0}' not defined in shader '{1}'", name, m_Shader->GetName());
            return;
        }
        uint8_t index = m_Shader->GetKeywordIndex(name);
        if (enabled) m_KeywordMask |= (KeywordMask(1) << index);
        else         m_KeywordMask &= ~(KeywordMask(1) << index);
    }

    bool Material::IsKeywordEnabled(const std::string& name) const
    {
        if (!m_Shader->IsKeywordDefined(name)) return false;
        uint8_t index = m_Shader->GetKeywordIndex(name);
        return (m_KeywordMask & (KeywordMask(1) << index)) != 0;
    }

}
