#include "prpch.h"
#include "PrismShader.h"
#include "Prism/Utilities/Utilities.h"
#include "Prism/Shader/PSL/SourceManager.h"
#include "Prism/Shader/PSL/TokenStream.h"
#include "Prism/Shader/PSL/Parser.h"
#include "Prism/Shader/PSL/GLSLGenerator.h"
#include "Prism/Shader/PSL/Diagnostics.h"
#include "Prism/Shader/Property/PropertyType.h"

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
        PSL::DiagnosticCollector diag;
        PSL::SourceManager sm(m_FilePath);

        if (!sm.IsValid())
        {
            PR_CORE_ERROR("PrismShader::Load - SourceManager failed for '{0}'", m_FilePath);
            return;
        }

        PSL::TokenStream stream(sm, &diag);
        PSL::Parser parser(stream, &diag);
        auto doc = parser.ParseShader();

        if (diag.HasErrors())
        {
            PR_CORE_ERROR("PrismShader::Load - Parse errors in '{0}':", m_FilePath);
            diag.PrintAll();
            return;
        }

        m_Name = std::move(doc.ShaderName);
        m_Uniforms = std::move(doc.Uniforms);
        m_MaterialLayout = std::move(doc.MaterialLayout);

        PR_CORE_INFO("PSL parsed '{0}': {1} uniforms, {2} passes",
            m_Name, m_Uniforms.size(), doc.Passes.size());

        CompilePasses(doc);
        CompileVariants(doc);

        for (auto& cb : m_ReloadedCallbacks)
            cb();
    }

    void PrismShader::CompilePasses(const PSL::AST::ShaderDocument& doc)
    {
        m_Passes.clear();
        m_PassGLSL.clear();
        m_Passes.reserve(doc.Passes.size());
        m_PassGLSL.reserve(doc.Passes.size());

        for (auto& passDef : doc.Passes)
        {
            ShaderPass pass;
            pass.Name = passDef.Name;
            pass.Tags = passDef.Tags;

            auto output = PSL::GLSLGen::Generate(passDef.Glsl, m_Uniforms, m_FilePath);
            pass.Program = Ref<Shader>(Shader::Create(output.Vertex, output.Fragment));

            PR_CORE_INFO("  Pass '{}': VS={}B  FS={}B",
                pass.Name, output.Vertex.size(), output.Fragment.size());

            m_Passes.push_back(std::move(pass));
            m_PassGLSL.push_back(std::move(passDef.Glsl));
        }

        m_Shader = m_Passes[0].Program;
    }

    void PrismShader::CompileVariants(const PSL::AST::ShaderDocument& doc)
    {
        m_Keywords.clear();
        m_VariantCache.clear();

        uint8_t kwIdx = 0;
        for (auto& passDef : doc.Passes)
        {
            for (auto& pragma : passDef.Glsl.Pragmas)
            {
                for (auto& kw : pragma.Keywords)
                {
                    if (std::find_if(m_Keywords.begin(), m_Keywords.end(),
                        [&](auto& k) { return k.Name == kw; }) != m_Keywords.end())
                        continue;
                    m_Keywords.push_back({ kw, kwIdx });
                    kwIdx++;
                    if (kwIdx >= MAX_KEYWORDS_PER_SHADER) break;
                }
            }
        }

        m_VariantCache[0] = m_Shader;

        uint32_t kwCount = (uint32_t)m_Keywords.size();
        if (kwCount == 0) return;

        if (kwCount > 10)
        {
            PR_CORE_WARN("Shader '{0}' has {1} keywords, too many for eager compilation", m_Name, kwCount);
            return;
        }

        uint32_t numVariants = 1u << kwCount;
        uint32_t passCount = (uint32_t)m_Passes.size();

        for (KeywordMask mask = 1; mask < numVariants; mask++)
        {
            auto keywords = KeywordsForMask(mask);

            std::string debugName;
            for (auto& kw : keywords)
            {
                if (!debugName.empty()) debugName += "|";
                debugName += kw;
            }

            for (uint32_t p = 0; p < passCount; p++)
            {
                auto output = PSL::GLSLGen::Generate(m_PassGLSL[p], m_Uniforms, m_FilePath, keywords);
                Ref<Shader> program = Ref<Shader>(Shader::Create(output.Vertex, output.Fragment));
                m_VariantCache[mask] = program;
            }

            PR_CORE_INFO("  Variant [{}]: {} program(s)", debugName, passCount);
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

    const PSL::AST::ShaderUniform* PrismShader::FindUniform(const std::string& name) const
    {
        for (auto& u : m_Uniforms)
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
        PR_CORE_ERROR("Keyword '{0}' not defined in shader '{1}'", name, m_Name);
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

        // Lazy compile
        auto keywords = KeywordsForMask(mask);
        for (uint32_t p = 0; p < (uint32_t)m_Passes.size(); p++)
        {
            auto output = PSL::GLSLGen::Generate(m_PassGLSL[p], m_Uniforms, m_FilePath, keywords);
            Ref<Shader> program = Ref<Shader>(Shader::Create(output.Vertex, output.Fragment));
            m_VariantCache[mask] = program;
        }
        return m_VariantCache[mask];
    }

    Ref<Shader> PrismShader::GetPassProgram(uint32_t passIndex, KeywordMask mask) const
    {
        PR_CORE_ASSERT(passIndex < m_Passes.size(),
            "GetPassProgram: passIndex {0} out of range ({1} passes)", passIndex, m_Passes.size());

        if (mask == 0)
            return m_Passes[passIndex].Program;

        Ref<Shader> variant = GetVariant(mask);
        // Return the first variant; per-pass variant lookup needs variant cache restructure
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
        std::string vs = File::ReadFile("E:/PrismEngine/AI/test1.Shader");
        std::string fs = File::ReadFile("E:/PrismEngine/AI/test2.Shader");
        auto scvc = OpenGLShader(vs, fs);
        PR_CORE_INFO("Scanning .Shader files in '{0}'...", directory);
        uint32_t success = 0, failed = 0;
        for (auto& entry : std::filesystem::recursive_directory_iterator(directory))
        {
            if (entry.path().extension() != ".Shader")
                continue;

            std::string path = entry.path().generic_string();

            auto shader = Ref<PrismShader>(PrismShader::Create(path));

            if (shader->GetName().empty())
            {
                PR_CORE_ERROR("Parse failed, skipping: {0}", path);
                failed++;
                continue;
            }

            auto& name = shader->GetName();
            if (m_Shaders.find(name) != m_Shaders.end())
            {
                PR_CORE_WARN("Duplicate shader name '{0}' from: {1}", name, path);
                continue;
            }
            m_Shaders[name] = shader;
            success++;
        }
        PR_CORE_INFO("Shader loading done: {0} success, {1} failed", success, failed);
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
