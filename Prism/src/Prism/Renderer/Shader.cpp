#include "prpch.h"
#include "Shader.h"

#include "PrismShaderCore/Generator/ReflectionGenerator.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Prism
{

    Ref<Shader> Shader::Create(const void* vertexSource, const void* fragmentSource,
        const PrismShaderCompiler::PassReflection& reflection)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPIType::OpenGL:
            return Ref<Shader>(new OpenGLShader((const char*)vertexSource, (const char*)fragmentSource, reflection));
        // case RendererAPIType::Vulkan:
            // return Ref<Shader>(new VulkanShader(vertexSource, fragmentSource, reflection));
        default:
            PR_CORE_ASSERT(false, "Unknown RendererAPI!"); return nullptr;
        }
    }

    Ref<Shader> Shader::Create(const void* computeSource)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPIType::OpenGL:
            return Ref<Shader>(new OpenGLShader((const char*)computeSource));
        // case RendererAPIType::Vulkan:
        //     return Ref<Shader>(new VulkanShader(computeSource));
        default:
            PR_CORE_ASSERT(false, "Unknown RendererAPI!"); return nullptr;
        }
        auto* result = new OpenGLShader((const char*)computeSource);
        return result;
    }

}
