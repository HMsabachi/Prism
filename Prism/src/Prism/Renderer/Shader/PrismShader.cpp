#include "prpch.h"
#include "PrismShader.h"
#include "Prism/Utilities/Utilities.h"
#include "Prism/ShaderCompiler/ShaderCompiler.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <iostream>

namespace Prism
{
    std::vector<Ref<PrismShader>> PrismShader::s_AllShaders;

    Ref<PrismShader> PrismShader::Create(const std::string& path)
    {
        Ref<PrismShader> shader = Ref<PrismShader>::Create(path);
        s_AllShaders.push_back(shader);
        return shader;
    }

    Ref<PrismShader> PrismShader::CreateFromString(const std::string& source)
    {
        Ref<PrismShader> shader = Ref<PrismShader>::Create();
        shader->Load(source);
        s_AllShaders.push_back(shader);
        return shader;
    }

    PrismShader::PrismShader(const std::string& path)
    {
        m_FilePath = path;
        Reload();
    }

    PrismShader::PrismShader()
    {
    }

    PrismShader::~PrismShader()
    {
    }

    void PrismShader::Reload()
    {
        auto source = File::ReadFile(m_FilePath);
        Load(source);
    }

    void PrismShader::Load(const std::string& source)
    {
        auto& compiler = ShaderCompiler::Get();
        m_Compiled = compiler.Compile(source, m_FilePath);

        if (m_Compiled.ShaderName.empty())
        {
            PR_CORE_ERROR("PrismShader::Load - Parse failed for '{}'", m_FilePath);
            return;
        }

        m_Name = std::move(m_Compiled.ShaderName);

        PR_CORE_INFO("PSL parsed '{}': {} uniforms, {} passes",
            m_Name, m_Compiled.Uniforms.size(), m_Compiled.Passes.size());

        CompilePasses();
        CompileVariants();

        for (auto& cb : m_ReloadedCallbacks)
            cb();
    }

    void PrismShader::CompilePasses()
    {
        auto& compiler = ShaderCompiler::Get();

        m_Passes.clear();
        m_Passes.reserve(m_Compiled.Passes.size());

        for (uint32_t i = 0; i < m_Compiled.Passes.size(); ++i)
        {
            auto out = compiler.GenerateGLSL(m_Compiled, i);

            ShaderPass pass;
            pass.Name = m_Compiled.Passes[i].Name;
            pass.Tags = m_Compiled.Passes[i].Tags;
            pass.Program = Ref<Shader>(Shader::Create(out.VertexShader, out.FragmentShader));

            PR_CORE_INFO("  Pass '{}': VS={}B  FS={}B",
                pass.Name, out.VertexShader.size(), out.FragmentShader.size());

            m_Passes.push_back(std::move(pass));
        }

        m_Shader = m_Passes[0].Program;
    }

    void PrismShader::CompileVariants()
    {
        auto& compiler = ShaderCompiler::Get();

        m_Keywords.clear();
        m_VariantCache.clear();

        for (auto& kw : m_Compiled.Keywords)
        {
            m_Keywords.push_back({ kw, (uint8_t)m_Keywords.size() });
            if (m_Keywords.size() >= MAX_KEYWORDS_PER_SHADER) break;
        }

        m_VariantCache[0] = m_Shader;

        uint32_t kwCount = (uint32_t)m_Keywords.size();
        if (kwCount == 0) return;

        if (kwCount > 10)
        {
            PR_CORE_WARN("Shader '{}' has {} keywords, too many for eager compilation", m_Name, kwCount);
            return;
        }

        uint32_t numVariants = 1u << kwCount;

        for (KeywordMask mask = 1; mask < numVariants; mask++)
        {
            auto keywords = KeywordsForMask(mask);

            std::string debugName;
            for (auto& kw : keywords)
            {
                if (!debugName.empty()) debugName += "|";
                debugName += kw;
            }

            auto out = compiler.GenerateGLSL(m_Compiled, 0, keywords);
            Ref<Shader> program = Ref<Shader>(Shader::Create(out.VertexShader, out.FragmentShader));
            m_VariantCache[mask] = program;

            PR_CORE_INFO("  Variant [{}]", debugName);
        }
    }

    std::vector<std::string> PrismShader::KeywordsForMask(KeywordMask mask) const
    {
        std::vector<std::string> result;
        for (auto& kw : m_Keywords)
            if (mask & (KeywordMask(1) << kw.Index))
                result.push_back(kw.Name);
        return result;
    }

    const PrismShaderCompiler::AST::ShaderUniform* PrismShader::FindUniform(const std::string& name) const
    {
        for (auto& u : m_Compiled.Uniforms)
            if (u.Name == name)
                return &u;
        return nullptr;
    }

    Ref<Shader> PrismShader::GetOriginalShader() const
    {
        return m_Shader;
    }

#pragma region Keyword / Variant

    uint8_t PrismShader::GetKeywordIndex(const std::string& name) const
    {
        for (auto& kw : m_Keywords)
            if (kw.Name == name)
                return kw.Index;
        PR_CORE_ERROR("Keyword '{}' not defined in shader '{}'", name, m_Name);
        PR_CORE_ASSERT(false);
        return 0xFF;
    }

    bool PrismShader::IsKeywordDefined(const std::string& name) const
    {
        for (auto& kw : m_Keywords)
            if (kw.Name == name)
                return true;
        return false;
    }

    Ref<Shader> PrismShader::GetVariant(KeywordMask mask) const
    {
        auto it = m_VariantCache.find(mask);
        if (it != m_VariantCache.end())
            return it->second;

        if (mask == 0)
            return m_Shader;

        auto& compiler = ShaderCompiler::Get();
        auto keywords = KeywordsForMask(mask);
        auto out = compiler.GenerateGLSL(m_Compiled, 0, keywords);
        Ref<Shader> program = Ref<Shader>(Shader::Create(out.VertexShader, out.FragmentShader));
        m_VariantCache[mask] = program;
        return m_VariantCache[mask];
    }

    Ref<Shader> PrismShader::GetPassProgram(uint32_t passIndex, KeywordMask mask) const
    {
        PR_CORE_ASSERT(passIndex < m_Passes.size(),
            "GetPassProgram: passIndex {} out of range ({} passes)", passIndex, m_Passes.size());

        if (mask == 0)
            return m_Passes[passIndex].Program;

        Ref<Shader> variant = GetVariant(mask);
        return variant ? variant : m_Passes[passIndex].Program;
    }

#pragma endregion

    void PrismShader::AddShaderReloadedCallback(const ShaderReloadedCallback& callback)
    {
        m_ReloadedCallbacks.push_back(callback);
    }

    // ShaderLibrary

    void ShaderLibrary::Add(const Ref<PrismShader>& shader)
    {
        auto& name = shader->GetName();
        PR_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end());
        m_Shaders[name] = shader;
    }

    void ShaderLibrary::Load(const std::string& path)
    {
        auto shader = Ref<PrismShader>(PrismShader::Create(path));
        auto& name = shader->GetName();
        PR_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end());
        m_Shaders[name] = shader;
    }

    void ShaderLibrary::Load(const std::string& name, const std::string& path)
    {
        PR_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end());
        m_Shaders[name] = Ref<PrismShader>(PrismShader::Create(path));
    }

    void ShaderLibrary::LoadAll(const std::string& directory)
    {
        PR_CORE_INFO("Scanning .Shader files in '{}'...", directory);
        uint32_t success = 0, failed = 0;
        for (auto& entry : std::filesystem::recursive_directory_iterator(directory))
        {
            if (entry.path().extension() != ".Shader")
                continue;

            std::string path = entry.path().generic_string();

            auto shader = Ref<PrismShader>(PrismShader::Create(path));

            if (shader->GetName().empty())
            {
                PR_CORE_ERROR("Parse failed, skipping: {}", path);
                failed++;
                continue;
            }

            auto& name = shader->GetName();
            if (m_Shaders.find(name) != m_Shaders.end())
            {
                PR_CORE_WARN("Duplicate shader name '{}' from: {}", name, path);
                continue;
            }
            m_Shaders[name] = shader;
            success++;
        }
        PR_CORE_INFO("Shader loading done: {} success, {} failed", success, failed);
    }

    const Ref<PrismShader>& ShaderLibrary::Get(const std::string& name) const
    {
        PR_CORE_ASSERT(m_Shaders.find(name) != m_Shaders.end());
        return m_Shaders.at(name);
    }

    const std::unordered_map<std::string, Ref<Prism::PrismShader>>& ShaderLibrary::GetAll() const
    {
        return m_Shaders;
    }

}
