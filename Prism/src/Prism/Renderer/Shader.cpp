#include "prpch.h"
#include "Shader.h"

#include "Prism/ShaderCompiler/ShaderCompiler.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Vulkan/VulkanShader.h"

namespace Prism
{

    Ref<Shader> Shader::Create(const PrismShaderCompiler::CompiledShader& shader,
        uint32_t passIndex,
        const std::vector<std::string>& keywords)
    {
        auto& compiler = ShaderCompiler::Get();
        switch (RendererAPI::Current())
        {
        case RendererAPIType::OpenGL:
        {
            auto out = compiler.GenerateGLSL(shader, passIndex, keywords);
            return Ref<Shader>(new OpenGLShader(out.VertexShader, out.FragmentShader));
        }
        case RendererAPIType::Vulkan:
        {
            auto out = compiler.GenerateSPIRV(shader, passIndex, keywords);
            return Ref<Shader>(new VulkanShader(out.SpirvVertex, out.SpirvFragment, out.Reflection));
        }
        default:
            PR_CORE_ASSERT(false, "Unknown RendererAPI!"); return nullptr;
        }
    }

    Ref<Shader> Shader::Create(const PrismShaderCompiler::CompiledComputeShader& shader,
        uint32_t kernelIndex)
    {
        auto& compiler = ShaderCompiler::Get();
        switch (RendererAPI::Current())
        {
        case RendererAPIType::OpenGL:
        {
            auto out = compiler.GenerateComputeGLSL(shader, kernelIndex);
            for (auto& w : out.Warnings)
                PR_CORE_WARN("CSL kernel '{}': {}", shader.Kernels[kernelIndex].Name, w);
            for (auto& e : out.Errors)
                PR_CORE_ERROR("CSL kernel '{}': {}", shader.Kernels[kernelIndex].Name, e);
            if (out.Errors.empty() && !out.Source.empty())
                return Ref<Shader>(new OpenGLShader((const char*)out.Source.c_str()));
            break;
        }
        case RendererAPIType::Vulkan:
        {
            auto out = compiler.GenerateComputeSPIRV(shader, kernelIndex);
            for (auto& w : out.Warnings)
                PR_CORE_WARN("CSL kernel '{}': {}", shader.Kernels[kernelIndex].Name, w);
            for (auto& e : out.Errors)
                PR_CORE_ERROR("CSL kernel '{}': {}", shader.Kernels[kernelIndex].Name, e);
            if (out.Errors.empty() && !out.Spirv.empty())
                return Ref<Shader>(new VulkanShader(out.Spirv, out.Reflection));
            break;
        }
        default:
            PR_CORE_ASSERT(false, "Unknown RendererAPI!");
        }
        return nullptr;
    }

}
