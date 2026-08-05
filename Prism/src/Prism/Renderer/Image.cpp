#include "prpch.h"
#include "Image.h"

#include "Platform/OpenGL/OpenGLImage.h"

#include "Prism/Renderer/RendererAPI.h"

namespace Prism {

    Ref<Image2D> Image2D::Create(ImageFormat format, uint32_t width, uint32_t height, Buffer buffer, uint32_t samples)
    {
        switch (RendererAPI::Current())
        {
            case RendererAPIType::None:    return nullptr;
            case RendererAPIType::OpenGL:  return Ref<OpenGLImage2D>::Create(format, width, height, buffer, samples);
            // case RendererAPIType::Vulkan: return Ref<VulkanImage2D>::Create(format, width, height, buffer, samples); // TODO
        }
        PR_CORE_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

    Ref<Image2D> Image2D::Create(ImageFormat format, uint32_t width, uint32_t height, const void* data, uint32_t samples)
    {
        switch (RendererAPI::Current())
        {
            case RendererAPIType::None:    return nullptr;
            case RendererAPIType::OpenGL:  return Ref<OpenGLImage2D>::Create(format, width, height, data, samples);
            // case RendererAPIType::Vulkan: return Ref<VulkanImage2D>::Create(format, width, height, data, samples); // TODO
        }
        PR_CORE_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

    Ref<ImageCube> ImageCube::Create(ImageFormat format, uint32_t width, uint32_t height, const void* data)
    {
        switch (RendererAPI::Current())
        {
            case RendererAPIType::None:    return nullptr;
            case RendererAPIType::OpenGL:  return Ref<OpenGLImageCube>::Create(format, width, height, data);
            // case RendererAPIType::Vulkan: return Ref<VulkanImageCube>::Create(format, width, height, data); // TODO
        }
        PR_CORE_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

}
