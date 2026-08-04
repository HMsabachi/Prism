#include "prpch.h"
#include "ShaderCompiler.h"

#include "Prism/Utilities/Utilities.h"
#include "Prism/Asset/AssetManager.h"
#include "Prism/Renderer/Shader/PrismShader.h"

#include <PrismShaderCore/Log.h>
#include <PrismShaderCore/Generator/IRGenerator.h>

namespace Prism
{

    void ShaderCompiler_LogCallback(PrismShaderCompiler::LogLevel lv, const std::string& msg)
    {
        using namespace PrismShaderCompiler;
        switch (lv)
        {
        case LogLevel::Error:   PR_CORE_ERROR("{}", msg);   break;
        case LogLevel::Warning: PR_CORE_WARN("{}", msg);    break;
        case LogLevel::Info:    PR_CORE_INFO("{}", msg);    break;
        case LogLevel::Debug:   PR_CORE_TRACE("{}", msg);    break;
        case LogLevel::Fatal:   PR_CORE_TRACE("{}", msg);    break;
        default: break;
        }
    }

    void ShaderCompiler::Init()
    {
        using namespace PrismShaderCompiler;

        CompilerConfig config;
        config.IncludeRoot = "Assets/Shaders/Include";
        config.ReadFile = &Prism::File::ReadFile;
        config.OnLog = &ShaderCompiler_LogCallback;
        config.ResolveUsePass = [](const std::string& shaderName) -> CompiledShader {
            return AssetManager::GetShaderLibrary()->OnResolveUsePass(shaderName);
        };

        s_Instance = new PrismShaderCompiler::ShaderCompiler(config);
    }

    PrismShaderCompiler::ShaderCompiler& ShaderCompiler::Get()
    {
        return *s_Instance;
    }

} // namespace Prism
