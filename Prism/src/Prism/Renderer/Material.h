#pragma once

#include "Shader/PrismShader.h"
#include "Prism/Renderer/Buffer/UniformBuffer.h"
#include "Texture.h"
#include <glm/glm.hpp>

namespace Prism
{
    class PRISM_API Material : public RefCounted
    {
    public:
        static Ref<Material> Create(const Ref<PrismShader>& shader);

    public:
        Material(const Ref<PrismShader>& shader);
        virtual ~Material();

        // === Shader ===
        const Ref<PrismShader>& GetShader() const { return m_Shader; }
        void SetShader(const Ref<PrismShader>& shader);

        // === Name ===
        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }

        // === Property setters ===
        void SetFloat(const std::string& name, float value);
        void SetInt(const std::string& name, int value);
        void SetVec3(const std::string& name, const glm::vec3& value);
        void SetVec4(const std::string& name, const glm::vec4& value);
        void SetMatrix(const std::string& name, const glm::mat4& value);
        void SetTexture(const std::string& name, const Ref<Texture2D>& texture);
        void SetTexture(const std::string& name, const Ref<TextureCube>& texture);

        // === Property getters ===
        float GetFloat(const std::string& name) const;
        int GetInt(const std::string& name) const;
        glm::vec3 GetVec3(const std::string& name) const;
        glm::vec4 GetVec4(const std::string& name) const;
        Ref<Texture2D> GetTexture2D(const std::string& name) const;
        Ref<TextureCube> GetTextureCube(const std::string& name) const;

        bool HasProperty(const std::string& name) const;

        // === Keywords ===
        void SetKeyword(const std::string& name, bool enabled);
        bool IsKeywordEnabled(const std::string& name) const;
        KeywordMask GetKeywordMask() const { return m_KeywordMask; }

        // === Engine ===
        void Bind(uint32_t passIndex = 0);
        uint32_t GetPassCount() const { return m_Shader->GetPassCount(); }

    private:
        void AllocateStorage();
        void OnShaderReloaded();
        void BindTextures();
        void WriteUniform(const std::string& name, const void* data, uint32_t size);

        Ref<PrismShader> m_Shader;
        std::string m_Name;
        Buffer m_PropertyBuffer;
        Ref<UniformBuffer> m_UniformBuffer;
        std::vector<Ref<Texture>> m_Textures;
        KeywordMask m_KeywordMask = 0;
        bool m_Dirty = true;
    };
}
