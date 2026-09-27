#include "prpch.h"
#include "Texture.h"

#include "Prism/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Platform/Vulkan/VulkanTexture.h"

namespace Prism {

    uint32_t Texture::GetBPP(ImageFormat format)
    {
        return Utils::GetImageFormatBPP(format);
    }

    Ref<Texture2D> Texture2D::Create(const TextureSpecification& specification)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPIType::None: return nullptr;
        case RendererAPIType::OpenGL: return Ref<OpenGLTexture2D>::Create(specification);
        case RendererAPIType::Vulkan: return Ref<VulkanTexture2D>::Create(specification);
        }
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const TextureSpecification& specification, Buffer imageData)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPIType::None: return nullptr;
        case RendererAPIType::OpenGL: return Ref<OpenGLTexture2D>::Create(specification, std::move(imageData));
        case RendererAPIType::Vulkan: return Ref<VulkanTexture2D>::Create(specification, std::move(imageData));
        }
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const TextureSpecification& specification, const std::string& path)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPIType::None: return nullptr;
        case RendererAPIType::OpenGL: return Ref<OpenGLTexture2D>::Create(specification, path);
        case RendererAPIType::Vulkan: return Ref<VulkanTexture2D>::Create(specification, path);
        }
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const std::string& path, TextureSpecification specification)
    {
        return Create(specification, path);
    }

    Ref<TextureCube> TextureCube::Create(const TextureSpecification& specification, Buffer imageData)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPIType::None: return nullptr;
        case RendererAPIType::OpenGL: return Ref<OpenGLTextureCube>::Create(specification, std::move(imageData));
        case RendererAPIType::Vulkan: return Ref<VulkanTextureCube>::Create(specification, std::move(imageData));
        }
        return nullptr;
    }
}