#pragma once

#include "Prism/Renderer/RendererAPI.h"
#include "Prism/Renderer/Texture.h"

namespace Prism {

	class PRISM_API OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap);

		OpenGLTexture2D(const std::string& path, bool srgb);
		virtual ~OpenGLTexture2D();

		virtual void Bind(uint32_t slot = 0) const override;
		virtual void BindImage(uint32_t slot, TextureAccess access, bool layered = true, uint32_t mipLevel = 0) const override;
		virtual uint32_t GetBinding() const override { return m_BindSlot; }

		virtual TextureFormat GetFormat() const override { return m_Format; }
		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		// This function currently returns the expected number of mips based on image size,
		// not present mips in data
		virtual uint32_t GetMipLevelCount() const override;

		virtual const std::string& GetPath() const override { return m_FilePath; }

		virtual bool Loaded() const override { return m_Loaded; }

		virtual void Lock() override;
		virtual void Unlock() override;

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual Buffer GetWriteableBuffer() override;

		virtual RendererID GetRendererID() const override { return m_RendererID; }


	private:
		RendererID m_RendererID;
		mutable uint32_t m_BindSlot = 0;
		TextureFormat m_Format;
		TextureWrap m_Wrap = TextureWrap::Clamp;
		uint32_t m_Width, m_Height;

		Buffer m_ImageData;
		bool m_IsHDR = false;

		bool m_Locked = false;
		bool m_Loaded = false;

		std::string m_FilePath;
	};

	class PRISM_API OpenGLTextureCube : public TextureCube
	{
	public:
		OpenGLTextureCube(TextureFormat format, uint32_t width, uint32_t height);
		OpenGLTextureCube(const std::string& path);
		virtual ~OpenGLTextureCube();

		virtual void Bind(uint32_t slot = 0) const;
		virtual void BindImage(uint32_t slot, TextureAccess access, bool layered = true, uint32_t mipLevel = 0) const override;
		virtual uint32_t GetBinding() const override { return m_BindSlot; }

		virtual TextureFormat GetFormat() const { return m_Format; }
		virtual uint32_t GetWidth() const { return m_Width; }
		virtual uint32_t GetHeight() const { return m_Height; }
		// This function currently returns the expected number of mips based on image size,
		// not present mips in data
		virtual uint32_t GetMipLevelCount() const override;

		virtual void GenerateMipMap() const override;
		virtual void CopyTo(Ref<TextureCube> destination) const override;

		virtual const std::string& GetPath() const override { return m_FilePath; }

		virtual RendererID GetRendererID() const override { return m_RendererID; }
	private:
		RendererID m_RendererID;
		mutable uint32_t m_BindSlot = 0;
		TextureFormat m_Format;
		uint32_t m_Width, m_Height;

		byte* m_ImageData;

		std::string m_FilePath;
	};
}