#include "prpch.h"
#include "Texture.h"

#include "Prism/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Prism {

    uint32_t Texture::CalculateMipMapCount(uint32_t width, uint32_t height)
    {
        uint32_t levels = 1;
        while ((width | height) >> levels)
            levels++;

        return levels;
    }



    uint32_t Texture::GetBPP(ImageFormat format)
    {
        return Utils::GetImageFormatBPP(format);
    }

    Ref<Texture2D> Texture2D::Create(ImageFormat format, unsigned int width, unsigned int height, const void* data)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPIType::None: return nullptr;
        case RendererAPIType::OpenGL: return Ref<OpenGLTexture2D>::Create(format, width, height, data);
        // case RendererAPIType::Vulkan: return Ref<VulkanTexture2D>::Create(format, width, height, data); // TODO
        }
        return nullptr;
    }
    Ref<Texture2D> Texture2D::Create(const std::string& path, bool srgb)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPIType::None: return nullptr;
        case RendererAPIType::OpenGL: return Ref<OpenGLTexture2D>::Create(path, srgb);
        // case RendererAPIType::Vulkan: return Ref<VulkanTexture2D>::Create(path, srgb); // TODO
        }
        return nullptr;
    }

    Ref<TextureCube> TextureCube::Create(ImageFormat format, uint32_t width, uint32_t height, const void* data)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPIType::None: return nullptr;
        case RendererAPIType::OpenGL: return Ref<OpenGLTextureCube>::Create(format, width, height, data);
        // case RendererAPIType::Vulkan: return Ref<VulkanTextureCube>::Create(format, width, height, data); // TODO
        }
        return nullptr;
    }
}