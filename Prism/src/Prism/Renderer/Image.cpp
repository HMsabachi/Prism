#include "prpch.h"
#include "Image.h"

#include "Platform/OpenGL/OpenGLImage.h"
#include "Platform/Vulkan/VulkanImage.h"

#include "Prism/Renderer/RendererAPI.h"

namespace Prism {

    Ref<Image2D> Image2D::Create(const ImageSpecification& specification, Buffer buffer)
    {
        switch (RendererAPI::Current())
        {
            case RendererAPIType::None:    return nullptr;
            case RendererAPIType::OpenGL:  return Ref<OpenGLImage2D>::Create(specification, std::move(buffer));
            case RendererAPIType::Vulkan:  return Ref<VulkanImage2D>::Create(specification, std::move(buffer));
        }
        PR_CORE_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

    Ref<Image2D> Image2D::Create(const ImageSpecification& specification, std::vector<Buffer>&& mips)
    {
        switch (RendererAPI::Current())
        {
            case RendererAPIType::None:    return nullptr;
            case RendererAPIType::OpenGL:  return Ref<OpenGLImage2D>::Create(specification, std::move(mips));
            case RendererAPIType::Vulkan:  return Ref<VulkanImage2D>::Create(specification, std::move(mips));
        }
        PR_CORE_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

    Ref<ImageCube> ImageCube::Create(const ImageSpecification& specification, Buffer buffer)
    {
        switch (RendererAPI::Current())
        {
            case RendererAPIType::None:    return nullptr;
            case RendererAPIType::OpenGL:  return Ref<OpenGLImageCube>::Create(specification, std::move(buffer));
            case RendererAPIType::Vulkan:  return Ref<VulkanImageCube>::Create(specification, std::move(buffer));
        }
        PR_CORE_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

}
