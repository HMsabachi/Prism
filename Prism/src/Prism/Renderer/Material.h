#pragma once

#include "Shader/PrismShader.h"
#include "Prism/Renderer/Buffer/UniformBuffer.h"
#include <unordered_set>
#include "Texture.h"
#include "Prism/Shader/PSL/PrismBindings.h"

namespace Prism
{
    class PRISM_API Material : public RefCounted
    {
        friend class MaterialInstance;

    public:
        static Ref<Material> Create(const Ref<PrismShader>& shader);
    public:
        Material(const Ref<PrismShader>& shader);
        virtual ~Material();

        void Bind();
        uint32_t GetPassCount() const { return m_Shader->GetPassCount(); }
        void BindPass(uint32_t passIndex);

        void SetKeyword(const std::string& name, bool enabled);
        bool IsKeywordEnabled(const std::string& name) const;
        KeywordMask GetKeywordMask() const { return m_KeywordMask; }

#pragma region Set
        template <typename T>
        void Set(const std::string& name, const T& value)
        {
            auto* uni = m_Shader->FindUniform(name);
            if (!uni) { PR_CORE_WARN("Material::Set - uniform '{0}' not found", name); return; }
            m_PropertyBuffer.Write((byte*)&value, sizeof(T), uni->BufferOffset);
            m_Dirty = true;
            for (auto mi : m_MaterialInstances)
                mi->OnMaterialValueUpdated(uni);
        }
        void Set(const std::string& name, const Ref<Texture2D>& texture)
        {
            auto* uni = m_Shader->FindUniform(name);
            if (!uni || uni->TextureSlot < 0) { PR_CORE_WARN("Material::Set - texture uniform '{0}' not found", name); return; }
            uint32_t slot = (uint32_t)uni->TextureSlot - PSL::PRISM_BINDING_TEXTURE;
            if (m_Textures.size() <= slot)
                m_Textures.resize(slot + 1);
            m_Textures[slot] = texture;
        }
        void Set(const std::string& name, const Ref<TextureCube>& texture)
        {
            auto* uni = m_Shader->FindUniform(name);
            if (!uni || uni->TextureSlot < 0) { PR_CORE_WARN("Material::Set - texture uniform '{0}' not found", name); return; }
            uint32_t slot = (uint32_t)uni->TextureSlot - PSL::PRISM_BINDING_TEXTURE;
            if (m_Textures.size() <= slot)
                m_Textures.resize(slot + 1);
            m_Textures[slot] = texture;
        }
#pragma endregion

        template<typename T>
        T& Get(const std::string& name)
        {
            auto* uni = m_Shader->FindUniform(name);
            if (!uni) { PR_CORE_WARN("Material::Get - uniform '{0}' not found", name); static T s_Default{}; return s_Default; }
            return m_PropertyBuffer.Read<T>(uni->BufferOffset);
        }

        template<typename T>
        Ref<T> GetResource(const std::string& name)
        {
            auto* uni = m_Shader->FindUniform(name);
            if (!uni || uni->TextureSlot < 0) { PR_CORE_WARN("Material::GetResource - texture '{0}' not found", name); return nullptr; }
            uint32_t slot = (uint32_t)uni->TextureSlot - PSL::PRISM_BINDING_TEXTURE;
            if (slot >= m_Textures.size()) return nullptr;
            return m_Textures[slot];
        }

        const Ref<PrismShader>& GetShader() const { return m_Shader; }
        const std::vector<Ref<Texture>>& GetTextures() const { return m_Textures; }
        RendererID GetUBOHandle() const { return m_UniformBuffer->GetRendererID(); }

    private:
        void AllocateStorage();
        void OnShaderReloaded();
        void BindTextures();

    private:
        Ref<PrismShader> m_Shader;
        std::unordered_set<MaterialInstance*> m_MaterialInstances;
        Buffer m_PropertyBuffer;
        Ref<UniformBuffer> m_UniformBuffer;
        std::vector<Ref<Texture>> m_Textures;
        KeywordMask m_KeywordMask = 0;
        bool m_Dirty = true;
    };

    class PRISM_API MaterialInstance : public RefCounted
    {
        friend class Material;
    public:
        static Ref<MaterialInstance> Create(const Ref<Material>& material, const std::string& name = "");
    public:
        MaterialInstance(const Ref<Material>& material, const std::string& name = "");
        virtual ~MaterialInstance();

        const std::string& GetName() const { return m_Name; }

        uint32_t GetPassCount() const { return m_Material->GetPassCount(); }
        void BindPass(uint32_t passIndex);

        void SetKeyword(const std::string& name, bool enabled);
        bool IsKeywordEnabled(const std::string& name) const;
        KeywordMask GetKeywordMask() const { return m_KeywordMask; }

#pragma region Set
        template <typename T>
        void Set(const std::string& name, const T& value)
        {
            auto* uni = m_Material->m_Shader->FindUniform(name);
            if (!uni) { PR_CORE_WARN("MaterialInstance::Set - uniform '{0}' not found", name); return; }
            m_PropertyBuffer.Write((byte*)&value, sizeof(T), uni->BufferOffset);
            m_OverriddenValues.insert(name);
        }
        void Set(const std::string& name, const Ref<Texture>& texture)
        {
            auto* uni = m_Material->m_Shader->FindUniform(name);
            if (!uni || uni->TextureSlot < 0) { PR_CORE_WARN("MaterialInstance::Set - texture '{0}' not found", name); return; }
            uint32_t slot = (uint32_t)uni->TextureSlot - PSL::PRISM_BINDING_TEXTURE;
            if (m_Textures.size() <= slot)
                m_Textures.resize(slot + 1);
            m_Textures[slot] = texture;
        }
        void Set(const std::string& name, const Ref<Texture2D>& texture)
        {
            Set(name, (const Ref<Texture>&)texture);
        }
        void Set(const std::string& name, const Ref<TextureCube>& texture)
        {
            Set(name, (const Ref<Texture>&)texture);
        }
        void Set(const std::string& name, const glm::mat4& value)
        {
            auto* uni = m_Material->m_Shader->FindUniform(name);
            if (!uni) { PR_CORE_WARN("MaterialInstance::Set - uniform '{0}' not found", name); return; }
            m_PropertyBuffer.Write((byte*)&value, sizeof(glm::mat4), uni->BufferOffset);
        }
#pragma endregion

        template<typename T>
        T& Get(const std::string& name)
        {
            auto* uni = m_Material->m_Shader->FindUniform(name);
            if (!uni) { PR_CORE_WARN("MaterialInstance::Get - uniform '{0}' not found", name); static T s_Default{}; return s_Default; }
            return m_PropertyBuffer.Read<T>(uni->BufferOffset);
        }

        template<typename T>
        Ref<T> GetResource(const std::string& name)
        {
            auto* uni = m_Material->m_Shader->FindUniform(name);
            if (!uni || uni->TextureSlot < 0) { PR_CORE_WARN("MaterialInstance::GetResource - texture '{0}' not found", name); return nullptr; }
            uint32_t slot = (uint32_t)uni->TextureSlot - PSL::PRISM_BINDING_TEXTURE;
            if (slot >= m_Textures.size()) return nullptr;
            return m_Textures[slot];
        }

        template<typename T>
        Ref<T> TryGetResource(const std::string& name)
        {
            auto* uni = m_Material->m_Shader->FindUniform(name);
            if (!uni || uni->TextureSlot < 0) return nullptr;
            uint32_t slot = (uint32_t)uni->TextureSlot - PSL::PRISM_BINDING_TEXTURE;
            if (slot >= m_Textures.size()) return nullptr;
            return m_Textures[slot];
        }

    public:
        void Bind();
        Ref<PrismShader> GetShader() const { return m_Material->m_Shader; }
        void SetShader(const Ref<PrismShader>& shader);

    private:
        void AllocateStorage();
        void OnShaderReloaded();
        void OnMaterialValueUpdated(const PSL::AST::ShaderUniform* uni);

    private:
        Ref<Material> m_Material;
        Buffer m_PropertyBuffer;
        std::vector<Ref<Texture>> m_Textures;
        std::string m_Name;
        std::unordered_set<std::string> m_OverriddenValues;
        KeywordMask m_KeywordMask = 0;
    };
}
