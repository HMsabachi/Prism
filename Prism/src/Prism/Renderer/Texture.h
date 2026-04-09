#pragma once

#include "Prism/Core/Core.h"
#include "RendererAPI.h"

namespace Prism {

	enum class PRISM_API TextureFormat
	{
		None = 0,
		RGB = 1,
		RGBA = 2,
	};
	enum class PRISM_API TextureWrap
	{
		None = 0,
		Clamp = 1,
		Repeat = 2
	};

	class PRISM_API Texture
	{
	public:
		virtual ~Texture() {}

		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual RendererID GetRendererID() const = 0;

		static uint32_t GetBPP(TextureFormat format);
	};

	class PRISM_API Texture2D : public Texture
	{
	public:
		static Texture2D* Create(TextureFormat format, unsigned int width, unsigned int height, TextureWrap wrap = TextureWrap::Repeat);
		static Texture2D* Create(const std::string& path, bool srgb = false);

		virtual TextureFormat GetFormat() const = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void Lock() = 0;
		virtual void Unlock() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual Buffer GetWriteableBuffer() = 0;

		virtual const std::string& GetPath() const = 0;
	};

	class PRISM_API TextureCube : public Texture
	{
	public:
		static TextureCube* Create(const std::string& path);


		virtual TextureFormat GetFormat() const = 0;
		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		virtual const std::string& GetPath() const = 0;
	};

}