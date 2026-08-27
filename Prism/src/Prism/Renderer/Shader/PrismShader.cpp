#include "prpch.h"
#include "PrismShader.h"
#include "Prism/Utilities/Utilities.h"
#include "Prism/ShaderCompiler/ShaderCompiler.h"
#include "Prism/Core/Hash.h"
#include "Prism/Asset/AssetManager.h"
#include "Prism/Asset/AssetSerializer.h"

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

    PrismShader::PrismShader(const std::string& path)
    {
        m_FilePath = path;
        Type = AssetType::Shader;
        Reload();
    }

    PrismShader::PrismShader()
    {
        Type = AssetType::Shader;
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
        for (const auto& tag : m_Compiled.Tags)
            m_ShaderTags[Hash::GenerateFNVHash64(tag.first)] = Hash::GenerateFNVHash64(tag.second);

        PR_CORE_INFO("PSL parsed '{}': {} uniforms, {} passes", m_Name, m_Compiled.Uniforms.size(), m_Compiled.Passes.size());

        m_VariantCache.clear();
        CompilePasses();
        CompileVariants();

        m_ReloadedCallbacks.Invoke();
    }

    void PrismShader::CompilePasses()
    {
        m_Passes.clear();
        m_Passes.reserve(m_Compiled.Passes.size());

        for (uint32_t i = 0; i < m_Compiled.Passes.size(); ++i)
        {
            ShaderPass pass;
            pass.Name = m_Compiled.Passes[i].Name;
            pass.NameHash = Hash::GenerateFNVHash64(pass.Name);
            for (auto& tag : m_Compiled.Passes[i].Tags)
                pass.Tags[Hash::GenerateFNVHash64(tag.first)] = Hash::GenerateFNVHash64(tag.second);
            pass.RenderState = m_Compiled.Passes[i].RenderState ?
                *m_Compiled.Passes[i].RenderState : PrismShaderCompiler::PipelineState::Default();
            pass.RenderState.CalculateHash();

            PR_CORE_INFO("  Pass '{}'", pass.Name);

            m_Passes.push_back(std::move(pass));
            m_VariantCache[i][0] = Shader::Create(m_Compiled, i);
        }
    }

    void PrismShader::CompileVariants()
    {
        m_Keywords.clear();
        m_MultiCompileMask = 0;

        uint8_t bit = 0;
        for (auto& kw : m_Compiled.Keywords)
        {
            if (m_Keywords.size() >= MAX_KEYWORDS_PER_SHADER) break;
            if (kw.IsMultiCompile)
            {
                m_Keywords.push_back({ kw.Name, bit++, true });
                m_MultiCompileMask |= (KeywordMask(1) << (bit - 1));
            }
        }
        for (auto& kw : m_Compiled.Keywords)
        {
            if (m_Keywords.size() >= MAX_KEYWORDS_PER_SHADER) break;
            if (!kw.IsMultiCompile)
                m_Keywords.push_back({ kw.Name, bit++, false });
        }

        m_PassKeywordMasks.assign(m_Compiled.Passes.size(), 0);
        for (uint32_t p = 0; p < m_Compiled.Passes.size(); ++p)
        {
            KeywordMask m = 0;
            for (auto& pragma : m_Compiled.Passes[p].Glsl.Pragmas)
                for (auto& name : pragma.Keywords)
                    for (auto& kw : m_Keywords)
                        if (kw.Name == name) m |= (KeywordMask(1) << kw.Index);
            m_PassKeywordMasks[p] = m;
        }

        uint32_t kwCount = (uint32_t)m_Keywords.size();
        if (kwCount == 0) return;

#if defined(PR_DEBUG) || defined(PR_RELEASE)
        for (uint32_t p = 0; p < m_Passes.size(); ++p)
        {
            KeywordMask passMulti = m_MultiCompileMask & m_PassKeywordMasks[p];
            if (passMulti == 0) continue;
            for (KeywordMask sub = passMulti; sub > 0; sub = (sub - 1) & passMulti)
            {
                auto keywords = KeywordsForMask(sub);
                m_VariantCache[p][sub] = Shader::Create(m_Compiled, p, keywords);
                PR_CORE_INFO("  Pass '{}' variant: [{}]", m_Passes[p].Name, Utilities::Join(keywords, ", "));
            }
        }
#endif
    }

    KeywordMask PrismShader::MultiCompileMask() const
    {
        return m_MultiCompileMask;
    }

    KeywordMask PrismShader::PassKeywordMask(uint32_t passIndex) const
    {
        if (passIndex >= m_PassKeywordMasks.size()) return 0;
        return m_PassKeywordMasks[passIndex];
    }

    KeywordMask PrismShader::ProjectMaskToPass(KeywordMask mask, uint32_t passIndex) const
    {
        return mask & PassKeywordMask(passIndex);
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

    int32_t PrismShader::FindPassByTag(uint64_t keyHash, uint64_t valueHash) const
    {
        for (uint32_t i = 0; i < m_Passes.size(); ++i)
        {
            auto& pass = m_Passes[i];
            auto it = pass.Tags.find(keyHash);
            if (it != pass.Tags.end() && it->second == valueHash)
                return i;
        }
        return -1;
    }


	int32_t PrismShader::FindPassByName(uint64_t nameHash) const
	{
        for (uint32_t i = 0; i < m_Passes.size(); ++i)
        {
            auto& pass = m_Passes[i];
            if (pass.NameHash == nameHash)
                return i;
        }
        return -1;
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

    Ref<Shader> PrismShader::GetVariant(uint32_t passIndex, KeywordMask mask) const
    {
        KeywordMask localMask = ProjectMaskToPass(mask, passIndex);
        auto& passCache = m_VariantCache[passIndex];
        auto it = passCache.find(localMask);
        if (it != passCache.end()) return it->second;

        auto keywords = KeywordsForMask(localMask);
        Ref<Shader> program = Shader::Create(m_Compiled, passIndex, keywords);
        passCache[localMask] = program;
        PR_CORE_TRACE("Shader'{}' Pass'{}' variant:[{}]", m_Name, m_Passes[passIndex].Name, Utilities::Join(keywords, ", "));
        return program;
    }

    Ref<Shader> PrismShader::GetPassProgram(uint32_t passIndex, KeywordMask mask) const
    {
        PR_CORE_ASSERT(passIndex < m_Passes.size(),
            "GetPassProgram: passIndex {} out of range ({} passes)", passIndex, m_Passes.size());
        return GetVariant(passIndex, mask);
    }

    #pragma endregion

    ShaderReloadedToken PrismShader::AddShaderReloadedCallback(const ShaderReloadedCallback& callback)
    {
        return m_ReloadedCallbacks.Add(callback);
    }

    void PrismShader::RemoveShaderReloadedCallback(ShaderReloadedToken token)
    {
        m_ReloadedCallbacks.Remove(token);
    }

    // ShaderLibrary

    Ref<PrismShader> ShaderLibrary::Load(const std::string& filePath)
    {
        std::string path = filePath;
        for (auto& c : path) if (c == '\\') c = '/';
        if (m_PathFromName.empty()) 
            m_PathFromName = ShaderCompiler::Get().ScanShaderDirectory("Assets");
        auto shader = Ref<PrismShader>(PrismShader::Create(path));
        auto& name = shader->GetName();
        if (name.empty())
        {
            PR_CORE_ERROR("ShaderLibrary::Load - Parse failed for '{}'", path);
            return shader;
        }
        if (Exists(name)) return shader;
        m_Shaders[name] = shader;
        return shader;
    }

    void ShaderLibrary::LoadAll(const std::string& directory)
    {
        PR_CORE_INFO("Scanning .Shader files in '{}'...", directory);
        m_PathFromName = ShaderCompiler::Get().ScanShaderDirectory(directory);
        uint32_t success = 0, failed = 0;
        for (auto& [name, path] : m_PathFromName)
        {
            AssetHandle handle = AssetManager::GetAssetHandleFromFilePath(path);
            Ref<PrismShader> shader = AssetManager::GetAsset<PrismShader>(handle);
            if (Exists(name)) { success++; continue; }
            if (shader->GetName().empty())
            {
                failed++;
                continue;
            }
            m_Shaders[name] = shader;
            success++;
        }
        PR_CORE_INFO("Shader loading done: {} success, {} failed", success, failed);
    }


    PrismShaderCompiler::CompiledShader ShaderLibrary::OnResolveUsePass(const std::string& name)
    {
        if (Exists(name))
        {
            return Get(name)->m_Compiled;
        }
        else
        {
            auto it = m_PathFromName.find(name);
            if (it != m_PathFromName.end())
            {
                auto shader = Ref<PrismShader>(PrismShader::Create(it->second));
                if (shader->GetName().empty())
                {
                    PR_CORE_ERROR("ShaderLibrary::OnResolveUsePass - Parse failed for '{}'", it->second);
                    return {};
                }
                m_Shaders[name] = shader;
                return shader->m_Compiled;
            }
            else
            {
                PR_CORE_ERROR("ShaderLibrary::OnResolveUsePass - Shader '{}' not found", name);
                return {};
            }
        }
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
