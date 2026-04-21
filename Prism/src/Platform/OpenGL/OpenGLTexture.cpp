#include "prpch.h"
#include "OpenGLTexture.h"

#include "Prism/Renderer/RendererAPI.h"
#include "Prism/Renderer/Renderer.h"

#include <glad/glad.h>
#include "stb_image.h"

namespace Prism {

	static GLenum PrismToOpenGLTextureFormat(TextureFormat format)
	{
		switch (format)
		{
		case Prism::TextureFormat::RGB:     return GL_RGB;
		case Prism::TextureFormat::RGBA:    return GL_RGBA;
		case Prism::TextureFormat::Float16: return GL_RGBA16F;
		}
		PR_CORE_ASSERT(false, "Unknown texture format!");
		return 0;
	}
	static GLenum PrismToOpenGLTextureAccess(TextureAccess access)
	{
		switch (access)
		{
		case Prism::TextureAccess::ReadOnly:  return GL_READ_ONLY;
		case Prism::TextureAccess::WriteOnly: return GL_WRITE_ONLY;
		case Prism::TextureAccess::ReadWrite: return GL_READ_WRITE;
		}
		PR_CORE_ASSERT(false, "Unknown texture format!");
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Texture2D
	//////////////////////////////////////////////////////////////////////////////////

	OpenGLTexture2D::OpenGLTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap)
		: m_Format(format), m_Width(width), m_Height(height), m_Wrap(wrap), m_RendererID(0)
	{
		PR_PROFILE_FUNCTION();

		Ref<OpenGLTexture2D> instance = this;
		Renderer::Submit([instance]() mutable
			{
			glGenTextures(1, &instance->m_RendererID);
			glBindTexture(GL_TEXTURE_2D, instance->m_RendererID);
			// 设置采样和环绕模式
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			GLenum wrap = instance->m_Wrap == TextureWrap::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
			// 设置最大各向异性过滤
			glTextureParameterf(instance->m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, RendererAPI::GetCapabilities().MaxAnisotropy);

			glTexImage2D(GL_TEXTURE_2D, 0, PrismToOpenGLTextureFormat(instance->m_Format), instance->m_Width, instance->m_Height, 0, PrismToOpenGLTextureFormat(instance->m_Format), GL_UNSIGNED_BYTE, nullptr);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
			});
		m_ImageData.Allocate(width * height * Texture::GetBPP(m_Format));
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool srgb)
		: m_FilePath(path), m_RendererID(0)
	{
		PR_PROFILE_FUNCTION();

		int width, height, channels;
		if (stbi_is_hdr(path.c_str()))
		{
			PR_CORE_INFO("Loading HDR texture {0}, srgb={1}", path, srgb);
			m_ImageData.Data = (byte*)stbi_loadf(path.c_str(), &width, &height, &channels, 0);
			m_IsHDR = true;
			m_Format = TextureFormat::Float16;
		}
		else
		{
			PR_CORE_INFO("Loading texture {0}, srgb={1}", path, srgb);
			m_ImageData.Data = stbi_load(path.c_str(), &width, &height, &channels, srgb ? STBI_rgb : STBI_rgb_alpha);
			PR_CORE_ASSERT(m_ImageData.Data, "Could not read image!");
			m_Format = TextureFormat::RGBA;
		}

		if (!m_ImageData.Data)
			return;

		m_Loaded = true;

		m_Width = width;
		m_Height = height;

		Ref<OpenGLTexture2D> instance = this;
		Renderer::Submit([instance, srgb]() mutable
			{
			// TODO: Consolidate properly
			if (srgb)
			{
				glCreateTextures(GL_TEXTURE_2D, 1, &instance->m_RendererID);
				int levels = Texture::CalculateMipMapCount(instance->m_Width, instance->m_Height);
				glTextureStorage2D(instance->m_RendererID, levels, GL_SRGB8, instance->m_Width, instance->m_Height);
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glTextureSubImage2D(instance->m_RendererID, 0, 0, 0, instance->m_Width, instance->m_Height, GL_RGB, GL_UNSIGNED_BYTE, instance->m_ImageData.Data);
				glGenerateTextureMipmap(instance->m_RendererID);
			}
			else
			{
				glGenTextures(1, &instance->m_RendererID);
				glBindTexture(GL_TEXTURE_2D, instance->m_RendererID);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

				GLenum internalFormat = PrismToOpenGLTextureFormat(instance->m_Format);
				GLenum format = srgb ? GL_SRGB8 : (instance->m_IsHDR ? GL_RGB : PrismToOpenGLTextureFormat(instance->m_Format)); // HDR = GL_RGB for now
				GLenum type = internalFormat == GL_RGBA16F ? GL_FLOAT : GL_UNSIGNED_BYTE;
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, instance->m_Width, instance->m_Height, 0, format, type, instance->m_ImageData.Data);
				glGenerateMipmap(GL_TEXTURE_2D);

				glBindTexture(GL_TEXTURE_2D, 0);
			}
			stbi_image_free(instance->m_ImageData.Data);
			});
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		GLuint rendererID = m_RendererID;
		Renderer::Submit([rendererID]() {
			glDeleteTextures(1, &rendererID);
		});
	}

	void OpenGLTexture2D::Bind(unsigned int slot) const
	{
		Ref<const OpenGLTexture2D> instance = this;
		Renderer::Submit([instance, slot]()
		{
				instance->m_BindSlot = slot;
			glBindTextureUnit(slot, instance->m_RendererID);
		});
	}

	void OpenGLTexture2D::BindImage(uint32_t slot, TextureAccess access, bool layered, uint32_t mipLevel) const
	{
		Ref<const OpenGLTexture2D> instance = this;
		Renderer::Submit([=]()
		{
			instance->m_BindSlot = slot;
			glBindImageTexture(slot, instance->m_RendererID, mipLevel, layered, 0, PrismToOpenGLTextureAccess(access), PrismToOpenGLTextureFormat(instance->m_Format));
		});
	}

	void OpenGLTexture2D::Lock()
	{
		m_Locked = true;
	}

	void OpenGLTexture2D::Unlock()
	{
		m_Locked = false;
		Ref<OpenGLTexture2D> instance = this;
		Renderer::Submit([instance]() {
			glTextureSubImage2D(instance->m_RendererID, 0, 0, 0, instance->m_Width, instance->m_Height, PrismToOpenGLTextureFormat(instance->m_Format), GL_UNSIGNED_BYTE, instance->m_ImageData.Data);
		});
	}

	void OpenGLTexture2D::Resize(uint32_t width, uint32_t height)
	{
		PR_CORE_ASSERT(m_Locked, "Texture must be locked!");

		m_ImageData.Allocate(width * height * Texture::GetBPP(m_Format));
#if PR_DEBUG
		m_ImageData.ZeroInitialize();
#endif
	}

	Buffer OpenGLTexture2D::GetWriteableBuffer()
	{
		PR_CORE_ASSERT(m_Locked, "Texture must be locked!");
		return m_ImageData;
	}


	uint32_t OpenGLTexture2D::GetMipLevelCount() const
	{
		return Texture::CalculateMipMapCount(m_Width, m_Height);
	}

	//////////////////////////////////////////////////////////////////////////////////
	// TextureCube
	//////////////////////////////////////////////////////////////////////////////////

	OpenGLTextureCube::OpenGLTextureCube(TextureFormat format, uint32_t width, uint32_t height)
		:m_Format(format), m_Width(width), m_Height(height), m_RendererID(0)
	{

		uint32_t levels = Texture::CalculateMipMapCount(width, height);

		Ref<OpenGLTextureCube> instance = this;
		Renderer::Submit([instance, levels]() mutable
		{
			glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &instance->m_RendererID);
			glTextureStorage2D(instance->m_RendererID, levels, PrismToOpenGLTextureFormat(instance->m_Format), instance->m_Width, instance->m_Height);
			glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			// glTextureParameterf(m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, 16);
			});
	}

	OpenGLTextureCube::OpenGLTextureCube(const std::string& path)
		: m_FilePath(path) , m_RendererID(0)
	{
		PR_PROFILE_FUNCTION();

		int width, height, channels;
		stbi_set_flip_vertically_on_load(false);
		m_ImageData = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb);

		m_Width = width;
		m_Height = height;
		m_Format = TextureFormat::RGB;

		uint32_t faceWidth = m_Width / 4;
		uint32_t faceHeight = m_Height / 3;
		PR_CORE_ASSERT(faceWidth == faceHeight, "Non-square faces!");

		std::array<uint8_t*, 6> faces;
		for (size_t i = 0; i < faces.size(); i++)
			faces[i] = new uint8_t[faceWidth * faceHeight * 3]; // 3 BPP

		int faceIndex = 0;

		for (size_t i = 0; i < 4; i++)
		{
			for (size_t y = 0; y < faceHeight; y++)
			{
				size_t yOffset = y + faceHeight;
				for (size_t x = 0; x < faceWidth; x++)
				{
					size_t xOffset = x + i * faceWidth;
					faces[faceIndex][(x + y * faceWidth) * 3 + 0] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 0];
					faces[faceIndex][(x + y * faceWidth) * 3 + 1] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 1];
					faces[faceIndex][(x + y * faceWidth) * 3 + 2] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 2];
				}
			}
			faceIndex++;
		}

		for (size_t i = 0; i < 3; i++)
		{
			// Skip the middle one
			if (i == 1)
				continue;

			for (size_t y = 0; y < faceHeight; y++)
			{
				size_t yOffset = y + i * faceHeight;
				for (size_t x = 0; x < faceWidth; x++)
				{
					size_t xOffset = x + faceWidth;
					faces[faceIndex][(x + y * faceWidth) * 3 + 0] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 0];
					faces[faceIndex][(x + y * faceWidth) * 3 + 1] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 1];
					faces[faceIndex][(x + y * faceWidth) * 3 + 2] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 2];
				}
			}
			faceIndex++;
		}

		Ref<OpenGLTextureCube> instance = this;
		Renderer::Submit([instance, faceWidth, faceHeight, faces]() mutable
		{
			glGenTextures(1, &instance->m_RendererID);
			glBindTexture(GL_TEXTURE_CUBE_MAP, instance->m_RendererID);

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTextureParameterf(instance->m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, RendererAPI::GetCapabilities().MaxAnisotropy);

			auto format = PrismToOpenGLTextureFormat(instance->m_Format);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[2]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[0]);

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[4]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[5]);

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[1]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[3]);

			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

			glBindTexture(GL_TEXTURE_2D, 0);

			for (size_t i = 0; i < faces.size(); i++)
				delete[] faces[i];

			stbi_image_free(instance->m_ImageData);
			});
	}

	OpenGLTextureCube::~OpenGLTextureCube()
	{
		GLuint rendererID = m_RendererID;
		Renderer::Submit([rendererID]() mutable {
			glDeleteTextures(1, &rendererID);
		});
	}

	void OpenGLTextureCube::Bind(unsigned int slot) const
	{
		Ref<const OpenGLTextureCube> instance = this;
		Renderer::Submit([instance, slot]() mutable {
			instance->m_BindSlot = slot;
			glBindTextureUnit(slot, instance->m_RendererID);
			});
	}

	void OpenGLTextureCube::BindImage(uint32_t slot, TextureAccess access, bool layered /*= true*/, uint32_t mipLevel) const
	{
		Ref<const OpenGLTextureCube> instance = this;
		Renderer::Submit([=]() mutable
		{
			instance->m_BindSlot = slot;
			glBindImageTexture(slot, instance->m_RendererID, mipLevel, layered, 0, PrismToOpenGLTextureAccess(access), PrismToOpenGLTextureFormat(instance->m_Format));
		});
	}

	uint32_t OpenGLTextureCube::GetMipLevelCount() const
	{
		return Texture::CalculateMipMapCount(m_Width, m_Height);
	}

	void OpenGLTextureCube::GenerateMipMap() const
	{
		Ref<const OpenGLTextureCube> instance = this;
		Renderer::Submit([instance]() mutable
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP, instance->m_RendererID);
			glGenerateTextureMipmap(instance->m_RendererID);
		});
	}

	void OpenGLTextureCube::CopyTo(Ref<TextureCube> destination) const
	{
		Ref<const OpenGLTextureCube> instance = this;
		Renderer::Submit([instance, destination]() mutable
		{
			glCopyImageSubData(instance->m_RendererID, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0, destination->GetRendererID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0, instance->m_Width, instance->m_Height, 6);
		});
	}

}