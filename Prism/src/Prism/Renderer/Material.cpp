#include "prpch.h"
#include "Material.h"
#include "Prism/Asset/AssetManager.h"
#include "Buffer/UniformBuffer.h"
#include "Prism/Renderer/Texture.h"

namespace Prism
{

    Ref<Material> Material::Create(AssetHandle shaderHandle)
    {
        return Ref<Material>::Create(shaderHandle);
    }


    Ref<Prism::Material> Material::Create(const Ref<Material>& material)
    {
        return Ref<Material>::Create(material);
    }

    Material::Material(AssetHandle shaderHandle)
        : m_Shader(AssetManager::GetAsset<PrismShader>(shaderHandle))
    {
        m_ReloadToken = m_Shader->AddShaderReloadedCallback(std::bind(&Material::OnShaderReloaded, this));
        AllocateStorage();
    }


    Material::Material(const Ref<Material>& material)
        : m_Shader(material->m_Shader)
    {
        m_ReloadToken = m_Shader->AddShaderReloadedCallback(std::bind(&Material::OnShaderReloaded, this));
        AllocateStorage();

        m_PropertyBuffer = material->m_PropertyBuffer;
        m_Textures = material->m_Textures;
        m_KeywordMask = material->m_KeywordMask;
        m_Name = material->m_Name;
        m_Dirty = true;
    }

    Material::~Material()
    {
        if (m_Shader && m_ReloadToken != 0)
            m_Shader->RemoveShaderReloadedCallback(m_ReloadToken);
        m_PropertyBuffer.Free();
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
        auto tempBuffer = std::move(m_PropertyBuffer);
        auto tempTextures = std::move(m_Textures);
        auto& layout = m_Shader->GetMaterialLayout();
        uint32_t totalSize = layout.GetTotalSize();
        m_PropertyBuffer.Allocate(totalSize);
        for (auto& uni : m_Shader->GetUniforms())
        {
            auto it = std::find_if(m_Uniforms.begin(), m_Uniforms.end(), [&](const PrismShaderCompiler::AST::ShaderUniform& u) { return u.Name == uni.Name; });
            if (it != m_Uniforms.end() && uni.Type == it->Type)
            {
                uint32_t offset = (uint32_t)uni.BufferOffset;
                uint32_t size = (uint32_t)uni.BufferSize;
                if (PrismShaderCompiler::PropertyTypeUtil::IsTextureType(uni.Type))
                {
                    auto texIt = tempTextures.find((uint32_t)it->TextureSlot);
                    if (texIt != tempTextures.end())
                        m_Textures[(uint32_t)uni.TextureSlot] = texIt->second;
                    continue;
                }
                m_PropertyBuffer.Write((const byte*)&tempBuffer.Data[it->BufferOffset], size, offset);
                continue;
            }
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
        m_Uniforms = m_Shader->GetUniforms();
        m_Dirty = true;
        m_UniformBuffer = UniformBuffer::Create((uint32_t)m_PropertyBuffer.Size);
    }

    void Material::OnShaderReloaded()
    {
        AllocateStorage();
    }

    void Material::SetShader(AssetHandle shaderHandle)
    {
        if (m_Shader && m_ReloadToken != 0)
            m_Shader->RemoveShaderReloadedCallback(m_ReloadToken);
        m_Shader = AssetManager::GetAsset<PrismShader>(shaderHandle);
        m_ReloadToken = m_Shader->AddShaderReloadedCallback(std::bind(&Material::OnShaderReloaded, this));
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
        m_Textures[(uint32_t)uni->TextureSlot] = texture;
    }

    void Material::SetTexture(const std::string& name, const Ref<TextureCube>& texture)
    {
        auto* uni = m_Shader->FindUniform(name);
        if (!uni || uni->TextureSlot < 0) { PR_CORE_WARN("Material::SetTexture - '{0}' not found", name); return; }
        m_Textures[(uint32_t)uni->TextureSlot] = texture;
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
        auto it = m_Textures.find((uint32_t)uni->TextureSlot);
        if (it == m_Textures.end()) return nullptr;
        return it->second.As<Texture2D>();
    }

    Ref<TextureCube> Material::GetTextureCube(const std::string& name) const
    {
        auto* uni = m_Shader->FindUniform(name);
        if (!uni || uni->TextureSlot < 0 || uni->Type != PrismShaderCompiler::PropertyType::TextureCube) return nullptr;
        auto it = m_Textures.find((uint32_t)uni->TextureSlot);
        if (it == m_Textures.end()) return nullptr;
        return it->second.As<TextureCube>();
    }

    bool Material::HasProperty(const std::string& name) const
    {
        return m_Shader->FindUniform(name) != nullptr;
    }

#pragma endregion

    Ref<Shader> Material::GetProgram(uint32_t passIndex) const
    {
        return m_Shader->GetPassProgram(passIndex, m_KeywordMask);
    }


    const Ref<Prism::UniformBuffer>& Material::RT_GetUniformBuffer() const
    {
        if (!m_Dirty) return m_UniformBuffer;
        m_UniformBuffer->RT_SetData(m_PropertyBuffer);
        m_Dirty = false;
        return m_UniformBuffer;
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
