#include "prpch.h"
#include "RendererContext.h"

#include "Prism/Renderer/RendererAPI.h"

#include "Platform/OpenGL/OpenGLContext.h"

namespace Prism {

    Ref<RendererContext> RendererContext::Create(GLFWwindow* windowHandle)
    {
        switch (RendererAPI::Current())
        {
            case RendererAPIType::None:    return nullptr;
            case RendererAPIType::OpenGL:  return Ref<OpenGLContext>::Create(windowHandle);
            // case RendererAPIType::Vulkan: return Ref<VulkanContext>::Create(windowHandle); // TODO
        }
        PR_CORE_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

}
