#pragma once

#include "Prism/Core/Core.h"
#include "Prism/Asset/Asset.h"
#include "Prism/Renderer/Image.h"
#include "RendererAPI.h"

namespace Prism {
    struct Buffer;
}

namespace Prism {

    enum class PRISM_API TextureWrap
    {
        None = 0,
        Clamp = 1,
        Repeat = 2
    };
    enum class PRISM_API TextureAccess
    {
        ReadOnly = 0,
        WriteOnly = 1,
        ReadWrite = 2
    };
    enum class PRISM_API TextureType
    {
        None = 0,
        Texture2D,
        TextureCube
    };

    class PRISM_API Texture : public Asset
    {
    public:
        virtual ~Texture() {}

        virtual void Bind(uint32_t slot = 0) const = 0;
        virtual void BindImage(uint32_t slot, TextureAccess access, bool layered = true, uint32_t mipLevel = 0) const = 0;

        virtual ImageFormat GetFormat() const = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual uint32_t GetMipLevelCount() const = 0;

        virtual uint64_t GetHash() const = 0;

        virtual TextureType GetType() const = 0;

        static uint32_t GetBPP(ImageFormat format);
        static uint32_t CalculateMipMapCount(uint32_t width, uint32_t height);
    };

    class PRISM_API Texture2D : public Texture
    {
    public:
        static Ref<Texture2D> Create(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr);
        static Ref<Texture2D> Create(const std::string& path, bool srgb = false);

        virtual Ref<Image2D> GetImage() const = 0;

        virtual void Lock() = 0;
        virtual void Unlock() = 0;

        virtual Buffer GetWriteableBuffer() = 0;

        virtual bool Loaded() const = 0;

        virtual const std::string& GetPath() const = 0;

        virtual TextureType GetType() const override { return TextureType::Texture2D; }
    };

    class PRISM_API TextureCube : public Texture
    {
    public:
        static Ref<TextureCube> Create(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr);

        virtual const std::string& GetPath() const = 0;

        virtual void GenerateMipMap() const = 0;

        virtual void CopyTo(Ref<TextureCube> destination) const = 0;

        virtual Ref<ImageCube> GetImage() const = 0;

        virtual TextureType GetType() const override { return TextureType::TextureCube; }
    };

}
