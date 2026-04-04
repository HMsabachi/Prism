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

	class PRISM_API Texture
	{
	public:
		virtual ~Texture() {}

		virtual RendererID GetRendererID() const = 0;
	};

	class PRISM_API Texture2D : public Texture
	{
	public:
		static Texture2D* Create(TextureFormat format, unsigned int width, unsigned int height);
		static Texture2D* Create(const std::string& path, bool srgb = false);

		virtual void Bind(unsigned int slot = 0) const = 0;

		virtual TextureFormat GetFormat() const = 0;
		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		virtual const std::string& GetPath() const = 0;
	};

	class PRISM_API TextureCube : public Texture
	{
	public:
		static TextureCube* Create(const std::string& path);

		virtual void Bind(unsigned int slot = 0) const = 0;

		virtual TextureFormat GetFormat() const = 0;
		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		virtual const std::string& GetPath() const = 0;
	};

}