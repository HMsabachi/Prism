#include "prpch.h"
#include "VertexInput.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLVertexInput.h"

namespace Prism {

    Ref<VertexInput> VertexInput::Create(const VertexInputSpecification& spec)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPIType::None:    return nullptr;
        case RendererAPIType::OpenGL:  return Ref<OpenGLVertexInput>::Create(spec);
        // case RendererAPIType::Vulkan: return Ref<VulkanVertexInput>::Create(spec); // TODO
        }
        PR_CORE_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

}
