#include "prpch.h"
#include "Shader.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Prism
{

    Ref<Shader> Shader::Create(const void* vertexSource, const void* fragmentSource)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPIType::OpenGL:
            return Ref<Shader>(new OpenGLShader((const char*)vertexSource, (const char*)fragmentSource));
        // case RendererAPIType::Vulkan:
            // return Ref<Shader>(new VulkanShader(vertexSource, fragmentSource));
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
