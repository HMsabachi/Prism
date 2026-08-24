#pragma once
#include "Prism/Core/Core.h"
#include "Prism/Asset/Asset.h"
#include "Prism/Renderer/Shader.h"
#include "Prism/Utilities/Delegate.h"
#include <PrismShaderCore/Compiler.h>
#include <PrismShaderCore/Pipeline/PipelineState.h>
#include <functional>
#include <optional>
#include <unordered_map>

namespace Prism
{
    using KeywordMask = uint64_t;
    constexpr size_t MAX_KEYWORDS_PER_SHADER = 64;

    struct ShaderKeyword
    {
        std::string Name;
        uint8_t Index = 0;
        bool IsMultiCompile = false;
    };

    using ShaderReloadedCallback = std::function<void()>;
    using ShaderReloadedToken = Delegate<>::Token;

    struct ShaderPass
    {
        std::string Name;
        uint64_t NameHash = 0;
        std::unordered_map<uint64_t, uint64_t> Tags;
        PrismShaderCompiler::PipelineState RenderState;
    };

    class PRISM_API PrismShader : public Asset
    {
    public:
        static Ref<PrismShader> Create(const std::string& path);

    public:
        PrismShader(const std::string& path);
        PrismShader();
        ~PrismShader();

        void Reload();
        void Load(const std::string& source);

    public:
        ShaderReloadedToken AddShaderReloadedCallback(const ShaderReloadedCallback& callback);
        void RemoveShaderReloadedCallback(ShaderReloadedToken token);
        const std::string& GetName() const { return m_Name; }
        uint64_t GetTagValue(uint64_t keyHash) const { auto it = m_ShaderTags.find(keyHash); if (it != m_ShaderTags.end()) return it->second; return 0; }

        const std::vector<PrismShaderCompiler::AST::ShaderUniform>& GetUniforms() const { return m_Compiled.Uniforms; }
        const PrismShaderCompiler::AST::ShaderUniform* FindUniform(const std::string& name) const;
        const PrismShaderCompiler::PropertyLayout& GetMaterialLayout() const { return m_Compiled.MaterialLayout; }

        uint32_t GetPassCount() const { return (uint32_t)m_Passes.size(); }
        const ShaderPass& GetPass(uint32_t index) const { return m_Passes[index]; }
        int32_t FindPassByTag(uint64_t keyHash, uint64_t valueHash) const;
        int32_t FindPassByName(uint64_t nameHash) const;

        const std::vector<ShaderKeyword>& GetKeywords() const { return m_Keywords; }
        uint8_t GetKeywordIndex(const std::string& name) const;
        bool IsKeywordDefined(const std::string& name) const;
        Ref<Shader> GetPassProgram(uint32_t passIndex, KeywordMask mask) const;

    private:
        Ref<Shader> GetVariant(uint32_t passIndex, KeywordMask mask) const;
        void CompilePasses();
        void CompileVariants();
        std::vector<std::string> KeywordsForMask(KeywordMask mask) const;
        KeywordMask MultiCompileMask() const;
        KeywordMask PassKeywordMask(uint32_t passIndex) const;
        KeywordMask ProjectMaskToPass(KeywordMask mask, uint32_t passIndex) const;

    private:
        std::string m_Name;
        std::string m_FilePath;

        PrismShaderCompiler::CompiledShader m_Compiled;

        std::unordered_map<uint64_t, uint64_t> m_ShaderTags;
        std::vector<ShaderPass> m_Passes;

        Delegate<> m_ReloadedCallbacks;

        std::vector<ShaderKeyword> m_Keywords;
        KeywordMask m_MultiCompileMask = 0;
        std::vector<KeywordMask> m_PassKeywordMasks;
        mutable std::unordered_map<uint32_t, std::unordered_map<KeywordMask, Ref<Shader>>> m_VariantCache;

    public:
        static std::vector<Ref<PrismShader>> s_AllShaders;
        friend class ShaderLibrary;
    };

    class PRISM_API ShaderLibrary : public RefCounted
    {
    public:
        ShaderLibrary() = default;
        ~ShaderLibrary() = default;

        Ref<PrismShader> Load(const std::string& filePath);
        void LoadAll(const std::string& directory);
        bool Exists(const std::string& name) const { return m_Shaders.find(name) != m_Shaders.end(); }

        PrismShaderCompiler::CompiledShader OnResolveUsePass(const std::string& name);
        const Ref<PrismShader>& Get(const std::string& name) const;
        const std::unordered_map<std::string, Ref<PrismShader>>& GetAll() const;
    private:
        std::unordered_map<std::string, Ref<PrismShader>> m_Shaders;
        std::unordered_map<std::string, std::string> m_PathFromName;
    };

}
