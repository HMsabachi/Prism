#pragma once

#include "Shader/PrismShader.h"
#include "Prism/Core/Buffer.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Prism
{
    class UniformBuffer;
    class Texture;
    class Texture2D;
    class TextureCube;

    class PRISM_API Material : public RefCounted
    {
    public:
        static Ref<Material> Create(AssetHandle shaderHandle);
        static Ref<Material> Create(const Ref<Material>& material);

    public:
        Material(AssetHandle shaderHandle);
        Material(const Ref<Material>& material);
        virtual ~Material();

        AssetHandle GetShaderHandle() const { return m_Shader->Handle; }
        const Ref<PrismShader>& GetShader() const { return m_Shader; }
        void SetShader(AssetHandle shaderHandle);

        // Name
        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }

#pragma region Get/Set Material Data
        void SetBool(const std::string& name, bool value);
        void SetInt(const std::string& name, int value);
        void SetFloat(const std::string& name, float value);
        void SetVec2(const std::string& name, const glm::vec2& value);
        void SetVec3(const std::string& name, const glm::vec3& value);
        void SetVec4(const std::string& name, const glm::vec4& value);
        void SetColor3(const std::string& name, const glm::vec3& value);
        void SetColor(const std::string& name, const glm::vec4& value);
        void SetMatrix3(const std::string& name, const glm::mat3& value);
        void SetMatrix4(const std::string& name, const glm::mat4& value);
        void SetTexture(const std::string& name, const Ref<Texture2D>& texture);
        void SetTexture(const std::string& name, const Ref<TextureCube>& texture);
        bool GetBool(const std::string& name) const;
        int GetInt(const std::string& name) const;
        float GetFloat(const std::string& name) const;
        glm::vec2 GetVec2(const std::string& name) const;
        glm::vec3 GetVec3(const std::string& name) const;
        glm::vec4 GetVec4(const std::string& name) const;
        glm::vec3 GetColor3(const std::string& name) const;
        glm::vec4 GetColor(const std::string& name) const;
        glm::mat3 GetMatrix3(const std::string& name) const;
        glm::mat4 GetMatrix4(const std::string& name) const;
        Ref<Texture2D> GetTexture2D(const std::string& name) const;
        Ref<TextureCube> GetTextureCube(const std::string& name) const;
#pragma endregion
        bool HasProperty(const std::string& name) const;
        // Keywords
        void SetKeyword(const std::string& name, bool enabled);
        bool IsKeywordEnabled(const std::string& name) const;
        KeywordMask GetKeywordMask() const { return m_KeywordMask; }

        Ref<Shader> GetProgram(uint32_t passIndex = 0) const;
        void BindProgram(uint32_t passIndex = 0) const;
        void BindUniform() const;
        void BindTexture() const;
        void Bind(uint32_t passIndex = 0) const;
        uint32_t GetPassCount() const { return m_Shader->GetPassCount(); }

    private:
        void AllocateStorage();
        void OnShaderReloaded();
        void WriteUniform(const std::string& name, const void* data, uint32_t size);

        Ref<PrismShader> m_Shader;
        std::string m_Name;
        Buffer m_PropertyBuffer;
        mutable Ref<UniformBuffer> m_UniformBuffer;
        std::vector<Ref<Texture>> m_Textures;
        std::vector<PrismShaderCompiler::AST::ShaderUniform> m_Uniforms;
        KeywordMask m_KeywordMask = 0;
        ShaderReloadedToken m_ReloadToken = 0;
        mutable bool m_Dirty = true;
    };
}
