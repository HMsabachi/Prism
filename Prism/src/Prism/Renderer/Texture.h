#pragma once

#include "Prism/Core/Core.h"
#include "RendererAPI.h"

namespace Prism {

	enum class PRISM_API TextureFormat
	{
		None = 0,
		RGB = 1,
		RGBA = 2,
		Float16 = 3
	};
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

	class PRISM_API Texture : public RefCounted
	{
	public:
		virtual ~Texture() {}

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void BindImage(uint32_t slot, TextureAccess access, bool layered = true, uint32_t mipLevel = 0) const = 0;
		virtual uint32_t GetBinding() const = 0;

		virtual TextureFormat GetFormat() const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetMipLevelCount() const = 0;

		virtual RendererID GetRendererID() const = 0;

		static uint32_t GetBPP(TextureFormat format);
		static uint32_t CalculateMipMapCount(uint32_t width, uint32_t height);

		virtual bool operator==(const Texture& other) const = 0;
	};

	class PRISM_API Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap = TextureWrap::Clamp);
		static Ref<Texture2D> Create(const std::string& path, bool srgb = false);

		virtual void Lock() = 0;
		virtual void Unlock() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual Buffer GetWriteableBuffer() = 0;

		virtual bool Loaded() const = 0;

		virtual const std::string& GetPath() const = 0;
	};

	class PRISM_API TextureCube : public Texture
	{
	public:
		static Ref<TextureCube> Create(TextureFormat format, uint32_t width, uint32_t height);
		static Ref<TextureCube> Create(const std::string& path);

		virtual const std::string& GetPath() const = 0;

		virtual void GenerateMipMap() const = 0;

		virtual void CopyTo(Ref<TextureCube> destination) const = 0;
	};

}