#pragma once
#include "Prism/Core/Core.h"
#include "Prism/Renderer/Shader.h"
#include "ShaderCommand.h"
#include "ShaderVariant.h"
#include "Prism/Shader/PSL/AST.h"
#include <functional>
#include <unordered_map>

namespace Prism
{
    using ShaderReloadedCallback = std::function<void()>;

    struct ShaderPass
    {
        Ref<Shader> Program;
        ShaderCommand Command;
        std::string Name;
        std::unordered_map<std::string, std::string> Tags;
    };

    class PRISM_API PrismShader : public RefCounted
    {
    public:
        static Ref<PrismShader> Create(const std::string& path);
        static Ref<PrismShader> CreateFromString(const std::string& source);

        Ref<Shader> GetOriginalShader() const;

    public:
        PrismShader(const std::string& path);
        PrismShader();
        ~PrismShader();

        void Reload();
        void Load(const std::string& source);

    public:
        void AddShaderReloadedCallback(const ShaderReloadedCallback& callback);
        const std::string& GetFilePath() const { return m_FilePath; }
        const std::string& GetName() const { return m_Name; }

        // Property metadata
        const std::vector<PSL::AST::ShaderUniform>& GetUniforms() const { return m_Uniforms; }
        const PSL::AST::ShaderUniform* FindUniform(const std::string& name) const;
        const PSL::PropertyLayout& GetMaterialLayout() const { return m_MaterialLayout; }

        // Multi-Pass API
        uint32_t GetPassCount() const { return (uint32_t)m_Passes.size(); }
        const ShaderPass& GetPass(uint32_t index) const { return m_Passes[index]; }

        // Keyword / Variant API
        const std::vector<ShaderKeyword>& GetKeywords() const { return m_Keywords; }
        uint8_t GetKeywordIndex(const std::string& name) const;
        bool IsKeywordDefined(const std::string& name) const;
        Ref<Shader> GetVariant(KeywordMask mask) const;
        Ref<Shader> GetPassProgram(uint32_t passIndex, KeywordMask mask) const;

    private:
        void CompilePasses(const PSL::AST::ShaderDocument& doc);
        void CompileVariants(const PSL::AST::ShaderDocument& doc);
        std::vector<std::string> KeywordsForMask(KeywordMask mask) const;

    private:
        std::string m_Name;
        std::string m_FilePath;

        std::vector<ShaderPass> m_Passes;
        std::vector<PSL::AST::GLSLCode> m_PassGLSL;
        Ref<Shader> m_Shader;

        std::vector<PSL::AST::ShaderUniform> m_Uniforms;
        PSL::PropertyLayout m_MaterialLayout;

        std::vector<ShaderReloadedCallback> m_ReloadedCallbacks;

        std::vector<ShaderKeyword> m_Keywords;
        mutable std::unordered_map<KeywordMask, Ref<Shader>> m_VariantCache;

    public:
        static std::vector<Ref<PrismShader>> s_AllShaders;
    };

    class PRISM_API ShaderLibrary : public RefCounted
    {
    public:
        ShaderLibrary() = default;
        ~ShaderLibrary() = default;

        void Add(const Ref<PrismShader>& shader);
        void Load(const std::string& path);
        void Load(const std::string& name, const std::string& path);
        void LoadAll(const std::string& directory);

        const Ref<PrismShader>& Get(const std::string& name) const;
        const std::unordered_map<std::string, Ref<PrismShader>>& GetAll() const;
    private:
        std::unordered_map<std::string, Ref<PrismShader>> m_Shaders;
    };

}
