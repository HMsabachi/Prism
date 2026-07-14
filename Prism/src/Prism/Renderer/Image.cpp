#include "prpch.h"
#include "Image.h"

#include "Platform/OpenGL/OpenGLImage.h"

#include "Prism/Renderer/RendererAPI.h"

namespace Prism {

    Ref<Image2D> Image2D::Create(ImageFormat format, uint32_t width, uint32_t height, Buffer buffer)
    {
        switch (RendererAPI::Current())
        {
            case RendererAPIType::None:    return nullptr;
            case RendererAPIType::OpenGL:  return Ref<OpenGLImage2D>::Create(format, width, height, buffer);
        }
        PR_CORE_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

    Ref<Image2D> Image2D::Create(ImageFormat format, uint32_t width, uint32_t height, const void* data)
    {
        switch (RendererAPI::Current())
        {
            case RendererAPIType::None:    return nullptr;
            case RendererAPIType::OpenGL:  return Ref<OpenGLImage2D>::Create(format, width, height, data);
        }
        PR_CORE_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

}
