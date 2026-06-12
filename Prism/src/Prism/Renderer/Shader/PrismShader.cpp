#include "prpch.h"
#include "PrismShader.h"
#include "Prism/Renderer/Shader/Parser/ShaderParser.h"
#include "Prism/Utilities/Utilities.h"
#include "Prism/Shader/PSL/SourceManager.h"
#include "Prism/Shader/PSL/TokenStream.h"
#include "Prism/Shader/PSL/Parser.h"

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
        m_FilePath = std::filesystem::absolute(path).string();
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

    // ======================================================================
    // 编译单个 Pass 的 Helper
    // ======================================================================
    static Ref<Shader> CompilePass(const std::string& debugName,
        const std::string& vsCode, const std::string& fsCode)
    {
        return Ref<Shader>(Shader::Create(debugName, vsCode, fsCode));
    }

    void PrismShader::Load(const std::string& source)
    {
        ShaderParser parser;
        ParseResult result = parser.Parse(source);
        if (!result.Success)
        {
            PR_CORE_ERROR("Failed to parse shader file '{0}'", m_FilePath);
            return;
        }

        uint32_t passCount = (uint32_t)result.Passes.size();
        PR_CORE_INFO("Loading shader '{0}' with {1} pass(es)", result.ShaderName, passCount);

        m_Name = result.ShaderName;

        // ------------------------------------------------------------------
        // 1. 编译所有 Pass
        // ------------------------------------------------------------------
        m_Passes.clear();
        m_Passes.reserve(passCount);
        m_VertexShaderSources.clear();
        m_FragmentShaderSources.clear();
        m_VertexShaderSources.reserve(passCount);
        m_FragmentShaderSources.reserve(passCount);

        for (uint32_t i = 0; i < passCount; i++)
        {
            const auto& passDesc = result.Passes[i];

            PR_CORE_INFO("  Pass[{0}] '{1}': VS code length={2}, FS code length={3}",
                i, passDesc.Name, passDesc.VertexShaderCode.size(), passDesc.FragmentShaderCode.size());

            ShaderPass pass;
            pass.Name = passDesc.Name;
            pass.Tags = passDesc.Tags;

            pass.Program = CompilePass(
                m_Name + "_pass" + std::to_string(i),
                passDesc.VertexShaderCode,
                passDesc.FragmentShaderCode
            );

            // RenderCommand: Pass 级别优先，否则用 Shader 级别
            std::string rcText = passDesc.RenderCommand.empty()
                ? result.RenderCommand
                : passDesc.RenderCommand;
            pass.Command = ParseShaderCommand(rcText);

            m_Passes.push_back(std::move(pass));
            m_VertexShaderSources.push_back(passDesc.VertexShaderCode);
            m_FragmentShaderSources.push_back(passDesc.FragmentShaderCode);
        }

        // 单 Pass 快捷引用（向后兼容）
        m_Shader = m_Passes[0].Program;

        // ------------------------------------------------------------------
        // 2. 重置关键字 / 变体状态
        // ------------------------------------------------------------------
        m_Keywords.clear();
        m_Variants.clear();

        // 填充关键字
        uint8_t kwIdx = 0;
        for (const auto& kw : result.Keywords)
        {
            m_Keywords.push_back({ kw, kwIdx });
            kwIdx++;
        }
        if (kwIdx > MAX_KEYWORDS_PER_SHADER)
            PR_CORE_WARN("Shader '{0}' has {1} keywords (max {2})", m_Name, kwIdx, MAX_KEYWORDS_PER_SHADER);

        // 注册基础变体 (mask = 0)
        {
            ShaderVariant baseVariant;
            baseVariant.Mask = 0;
            baseVariant.DebugName = "(default)";
            baseVariant.ShaderProgram = m_Shader;
            for (auto& pass : m_Passes)
                baseVariant.PassPrograms.push_back(pass.Program);
            m_Variants[0] = baseVariant;
        }

        // 编译所有关键字变体组合
        if (!m_Keywords.empty())
        {
            size_t kwCount = m_Keywords.size();
            if (kwCount > 10)
            {
                PR_CORE_WARN("Shader '{0}' has {1} keywords -> too many variants, skipping compilation", m_Name, kwCount);
            }
            else
            {
                size_t numVariants = size_t(1) << kwCount;
                for (KeywordMask mask = 1; mask < numVariants; mask++)
                {
                    std::string defines;
                    std::string debugName;
                    for (const auto& kw : m_Keywords)
                    {
                        if (mask & (KeywordMask(1) << kw.Index))
                        {
                            defines += "#define " + kw.Name + "\n";
                            if (!debugName.empty()) debugName += "|";
                            debugName += kw.Name;
                        }
                    }

                    auto makeVariantSource = [](const std::string& base, const std::string& defs) -> std::string {
                        size_t pos = base.find('\n');
                        if (pos == std::string::npos) return base;
                        return base.substr(0, pos + 1) + defs + base.substr(pos + 1);
                        };

                    ShaderVariant variant;
                    variant.Mask = mask;
                    variant.DebugName = debugName;

                    // 为每个 Pass 编译对应的变体
                    for (uint32_t p = 0; p < passCount; p++)
                    {
                        std::string vsCode = makeVariantSource(m_VertexShaderSources[p], defines);
                        std::string fsCode = makeVariantSource(m_FragmentShaderSources[p], defines);

                        std::string variantPassName = m_Name + "#" + debugName + "_pass" + std::to_string(p);
                        Ref<Shader> variantProgram = Ref<Shader>(Shader::Create(variantPassName, vsCode, fsCode));

                        variant.PassPrograms.push_back(variantProgram);
                        if (p == 0)
                            variant.ShaderProgram = variantProgram;
                    }

                    m_Variants[mask] = variant;
                }
            }
        }

        // ------------------------------------------------------------------
        // 3. 属性声明 & 默认值
        // ------------------------------------------------------------------
        m_Declaration = PropertyBufferDeclaration();
        for (const auto& prop : result.Properties)
        {
            auto& decl = m_Declaration.AddProperty(
                PropertyDeclaration(prop.Type, prop.Name, prop.DisplayName)
            );
            if (prop.Type == PropertyDeclarationType::Enum)
                decl.SetEnumOptions(prop.EnumOptions);
        }
        PackDefaultValues(result.Properties);

        SetProperty(m_DefaultValueBuffer);

        for (const auto& callback : m_ReloadedCallbacks)
            callback();
    }

    void PrismShader::PackDefaultValues(const std::vector<PropertyDescriptor>& properties)
    {
        using namespace Prism::StrParse;
        using namespace Prism::PropertyType;

        m_DefaultValueBuffer.Allocate(m_Declaration.GetSize());

        auto declIt = m_Declaration.begin();
        for (size_t i = 0; i < properties.size(); ++i, ++declIt)
        {
            const auto& prop = properties[i];
            uint32_t offset = declIt->GetOffset();

            switch (prop.Type)
            {
            case PropertyDeclarationType::Bool:
            {
                Bool val = (prop.DefaultValue == "true" || prop.DefaultValue == "1");
                m_DefaultValueBuffer.Write((byte*)&val, sizeof(Bool), offset);
                break;
            }
            case PropertyDeclarationType::Color:
            {
                Color val = Parse<Color>(prop.DefaultValue);
                m_DefaultValueBuffer.Write((byte*)&val, sizeof(Color), offset);
                break;
            }
            case PropertyDeclarationType::Color3:
            {
                Vector3 val = Parse<Vector3>(prop.DefaultValue);
                m_DefaultValueBuffer.Write((byte*)&val, sizeof(Vector3), offset);
                break;
            }
            case PropertyDeclarationType::Float:
            {
                Float val = Parse<Float>(prop.DefaultValue);
                m_DefaultValueBuffer.Write((byte*)&val, sizeof(Float), offset);
                break;
            }
            case PropertyDeclarationType::Int:
            {
                Int val = Parse<Int>(prop.DefaultValue);
                m_DefaultValueBuffer.Write((byte*)&val, sizeof(Int), offset);
                break;
            }
            case PropertyDeclarationType::Vector2:
            {
                Vector2 val = Parse<Vector2>(prop.DefaultValue);
                m_DefaultValueBuffer.Write((byte*)&val, sizeof(Vector2), offset);
                break;
            }
            case PropertyDeclarationType::Vector3:
            {
                Vector3 val = Parse<Vector3>(prop.DefaultValue);
                m_DefaultValueBuffer.Write((byte*)&val, sizeof(Vector3), offset);
                break;
            }
            case PropertyDeclarationType::Vector4:
            {
                Vector4 val = Parse<Vector4>(prop.DefaultValue);
                m_DefaultValueBuffer.Write((byte*)&val, sizeof(Vector4), offset);
                break;
            }
            case PropertyDeclarationType::Texture2D:
            case PropertyDeclarationType::Texture2DMS:
            {
                PropertyType::Texture2D tex;
                tex.slot = m_NextTexSlot++;
                m_DefaultValueBuffer.Write((byte*)&tex, sizeof(PropertyType::Texture2D), offset);
                break;
            }
            case PropertyDeclarationType::TextureCube:
            {
                PropertyType::TextureCube tex;
                tex.slot = m_NextTexSlot++;
                m_DefaultValueBuffer.Write((byte*)&tex, sizeof(PropertyType::TextureCube), offset);
                break;
            }
            case PropertyDeclarationType::Range:
            {
                Range val;
                val.min = prop.Min;
                val.max = prop.Max;
                val.value = Parse<float>(prop.DefaultValue);
                m_DefaultValueBuffer.Write((byte*)&val, sizeof(Range), offset);
                break;
            }
            case PropertyDeclarationType::Matrix3:
            {
                Matrix3 val = Parse<Matrix3>(prop.DefaultValue);
                m_DefaultValueBuffer.Write((byte*)&val, sizeof(Matrix3), offset);
                break;
            }
            case PropertyDeclarationType::Matrix4:
            {
                Matrix4 val = Parse<Matrix4>(prop.DefaultValue);
                m_DefaultValueBuffer.Write((byte*)&val, sizeof(Matrix4), offset);
                break;
            }
            case PropertyDeclarationType::Enum:
            {
                Int val = Parse<Int>(prop.DefaultValue);
                m_DefaultValueBuffer.Write((byte*)&val, sizeof(Int), offset);
                break;
            }
            default:
                break;
            }
        }
    }

    uint32_t PrismShader::GetTextureSlot(const std::string& name) const
    {
        const PropertyDeclaration* decl = m_Declaration.FindProperty(name);
        switch (decl->GetType())
        {
        case PropertyDeclarationType::Texture2D:
        case PropertyDeclarationType::Texture2DMS:
        case PropertyDeclarationType::TextureCube:
            break;
        default:
            PR_CORE_ERROR("Property {0} is not a texture type!", name);
            PR_CORE_ASSERT(false, "");
        }
        return decl->GetValue<uint32_t>(m_DefaultValueBuffer);
    }

    const ShaderCommand& PrismShader::GetShaderCommand() const
    {
        // 向后兼容：返回 Pass 0 的命令
        return m_Passes[0].Command;
    }

    void PrismShader::Bind()
    {
        // 向后兼容：绑定 Pass 0
        m_Passes[0].Program->Bind();
        m_Passes[0].Program->ApplyCommand(m_Passes[0].Command);
    }

    void PrismShader::BindPass(uint32_t passIndex)
    {
        PR_CORE_ASSERT(passIndex < m_Passes.size(), "BindPass: passIndex {0} out of range ({1} passes)", passIndex, m_Passes.size());
        auto& pass = m_Passes[passIndex];
        pass.Program->Bind();
        pass.Program->ApplyCommand(pass.Command);
    }

    void PrismShader::SetProperty(const Buffer& buffer)
    {
        m_Shader->SetProperty(m_Declaration, buffer);
    }

    void PrismShader::AddShaderReloadedCallback(const ShaderReloadedCallback& callback)
    {
        m_ReloadedCallbacks.push_back(callback);
    }

    Ref<Shader> PrismShader::GetOriginalShader() const
    {
        return m_Shader;
    }

#pragma region Keyword / Variant

    uint8_t PrismShader::GetKeywordIndex(const std::string& name) const
    {
        for (const auto& kw : m_Keywords)
        {
            if (kw.Name == name)
                return kw.Index;
        }
        PR_CORE_ERROR("Keyword '{0}' is not defined in shader '{1}'", name, m_Name);
        PR_CORE_ASSERT(false);
        return 0xFF;
    }

    bool PrismShader::IsKeywordDefined(const std::string& name) const
    {
        for (const auto& kw : m_Keywords)
        {
            if (kw.Name == name)
                return true;
        }
        return false;
    }

    Ref<Shader> PrismShader::GetVariant(KeywordMask mask) const
    {
        // 向后兼容：返回 Pass 0 的变体程序
        auto it = m_Variants.find(mask);
        if (it != m_Variants.end())
            return it->second.ShaderProgram;

        if (mask != 0)
            PR_CORE_WARN("Variant mask {0} not compiled for shader '{1}', falling back to base", mask, m_Name);
        return m_Shader;
    }

    Ref<Shader> PrismShader::GetPassProgram(uint32_t passIndex, KeywordMask mask) const
    {
        PR_CORE_ASSERT(passIndex < m_Passes.size(),
            "GetPassProgram: passIndex {0} out of range ({1} passes)", passIndex, m_Passes.size());

        auto it = m_Variants.find(mask);
        if (it != m_Variants.end())
        {
            if (passIndex < it->second.PassPrograms.size())
                return it->second.PassPrograms[passIndex];
            return it->second.ShaderProgram;
        }

        if (mask != 0)
            PR_CORE_WARN("Variant mask {0} not compiled for shader '{1}', falling back to base", mask, m_Name);
        return m_Passes[passIndex].Program;
    }

#pragma endregion

#pragma region Native uniforms

    void PrismShader::SetMat4FromRenderThread(const std::string& name, const glm::mat4& value)
    {
        m_Shader->SetMat4FromRenderThread(name, value);
    }
    void PrismShader::SetInt(const std::string& name, int value)
    {
        m_Shader->SetInt(name, value);
    }
    void PrismShader::SetIntArray(const std::string& name, int* values, uint32_t size)
    {
        m_Shader->SetIntArray(name, values, size);
    }
    void PrismShader::SetFloat(const std::string& name, float value)
    {
        m_Shader->SetFloat(name, value);
    }
    void PrismShader::SetVec3(const std::string& name, const glm::vec3& value)
    {
        m_Shader->SetVec3(name, value);
    }
    void PrismShader::SetVec4(const std::string& name, const glm::vec4& value)
    {
        m_Shader->SetVec4(name, value);
    }
    void PrismShader::SetMat4(const std::string& name, const glm::mat4& value)
    {
        m_Shader->SetMat4(name, value);
    }

#pragma endregion

    // //////////////////////////////////////////////////
    // --- ShaderLibrary  ---
    // //////////////////////////////////////////////////

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
        PR_CORE_INFO("正在扫描 '{0}' 中的 .Shader 文件...", directory);
        uint32_t success = 0, failed = 0;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directory))
        {
            if (entry.path().extension() != ".Shader")
                continue;

            std::string path = entry.path().string();
            auto shader = Ref<PrismShader>(PrismShader::Create(path));

            if (shader->GetName().empty())
            {
                PR_CORE_ERROR("解析失败，跳过: {0}", path);
                failed++;
                continue;
            }

            auto& name = shader->GetName();
            if (m_Shaders.find(name) != m_Shaders.end())
            {
                PR_CORE_WARN("重复的Shader名称 '{0}'，来自: {1}", name, path);
                continue;
            }
            m_Shaders[name] = shader;
            success++;
        }
        PR_CORE_INFO("Shader加载完成: {0} 个成功, {1} 个失败", success, failed);

        // ====== 新 PSL 管线 ======
        if (!m_Shaders.empty())
        {
            PSL::DiagnosticCollector diag;
            auto& firstShader = m_Shaders.begin()->second;
            std::string testPath = firstShader->GetFilePath();

            PSL::SourceManager sm(testPath);
            if (sm.IsValid())
            {
                PSL::TokenStream stream(sm, &diag);
                PSL::Parser parser(stream, &diag);
                auto doc = parser.ParseShader();

                PR_CORE_INFO("[PSL测试] ========================================");
                PR_CORE_INFO("[PSL测试] 文件: {0}", testPath);
                PR_CORE_INFO("[PSL测试] Shader: {0}", doc.ShaderName);
                PR_CORE_INFO("[PSL测试] Properties ({0}):", doc.Properties.size());
                for (auto& p : doc.Properties)
                    PR_CORE_INFO("[PSL测试]   {0} (\"{1}\") type={2}",
                        p.Name, p.DisplayName, (int)p.Type);

                PR_CORE_INFO("[PSL测试] Passes ({0}):", doc.Passes.size());
                for (auto& pass : doc.Passes)
                {
                    auto& g = pass.Glsl;
                    PR_CORE_INFO("[PSL测试]   Pass '{0}': Shared={1}B  Vert={2}B  Frag={3}B  Attrs={4}  Varyings={5}  Pragmas={6}  Includes={7}",
                        pass.Name, g.SharedSource.size(), g.Vertex.Source.size(), g.Fragment.Source.size(),
                        g.Attributes.size(), g.Varyings.size(), g.Pragmas.size(), g.Includes.size());
                }
                PR_CORE_INFO("[PSL测试] ========================================");
            }
            else
            {
                PR_CORE_ERROR("[PSL测试] SourceManager 无法打开: {0}", testPath);
            }

            diag.PrintAll();
        }
    }

    const Ref<PrismShader>& ShaderLibrary::Get(const std::string& name) const
    {
        PR_CORE_ASSERT(m_Shaders.find(name) != m_Shaders.end());
        return m_Shaders.at(name);
    }

    const std::unordered_map<std::string, Prism::Ref<Prism::PrismShader>>& ShaderLibrary::GetAll() const
    {
        return m_Shaders;
    }

}
