#pragma once

#include "Prism/Core/Core.h"
#include "Prism/Asset/Asset.h"
#include "Prism/Renderer/Image.h"
#include "RendererAPI.h"

namespace Prism {
    struct Buffer;
}

namespace Prism {

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

    struct TextureSpecification
    {
        ImageFormat Format = ImageFormat::RGBA8;
        uint32_t Width = 1;
        uint32_t Height = 1;
        TextureWrap SamplerWrap = TextureWrap::Repeat;
        TextureFilter SamplerFilter = TextureFilter::Linear;
        bool GenerateMips = true;
    };

    class PRISM_API Texture : public Asset
    {
    public:
        virtual ~Texture() {}

        virtual ImageFormat GetFormat() const = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual uint32_t GetMipLevelCount() const = 0;

        virtual TextureType GetType() const = 0;

        static uint32_t GetBPP(ImageFormat format);
    };

    class PRISM_API Texture2D : public Texture
    {
    public:
        static Ref<Texture2D> Create(const TextureSpecification& specification);
        static Ref<Texture2D> Create(const TextureSpecification& specification, Buffer imageData);
        static Ref<Texture2D> Create(const TextureSpecification& specification, const std::string& path);
        static Ref<Texture2D> Create(const std::string& path, TextureSpecification specification = TextureSpecification());

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
        static Ref<TextureCube> Create(const TextureSpecification& specification, Buffer imageData = Buffer());

        virtual const std::string& GetPath() const = 0;

        virtual Ref<ImageCube> GetImage() const = 0;

        virtual TextureType GetType() const override { return TextureType::TextureCube; }
    };

}
