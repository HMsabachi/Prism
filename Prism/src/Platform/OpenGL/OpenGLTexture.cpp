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
		}
		return 0;
	}

	static int CalculateMipMapCount(int width, int height)
	{
		int levels = 1;
		while ((width | height) >> levels) {
			levels++;
		}
		return levels;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Texture2D
	//////////////////////////////////////////////////////////////////////////////////

	OpenGLTexture2D::OpenGLTexture2D(TextureFormat format, unsigned int width, unsigned int height, TextureWrap wrap)
		: m_Format(format), m_Width(width), m_Height(height), m_Wrap(wrap), m_RendererID(0)
	{
		PR_PROFILE_FUNCTION();

		auto self = this;
		PR_RENDER_1(self, {
			glGenTextures(1, &self->m_RendererID);
			glBindTexture(GL_TEXTURE_2D, self->m_RendererID);
			// 设置采样和环绕模式
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			GLenum wrap = self->m_Wrap == TextureWrap::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
			// 设置最大各向异性过滤
			glTextureParameterf(self->m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, RendererAPI::GetCapabilities().MaxAnisotropy);

			glTexImage2D(GL_TEXTURE_2D, 0, PrismToOpenGLTextureFormat(self->m_Format), self->m_Width, self->m_Height, 0, PrismToOpenGLTextureFormat(self->m_Format), GL_UNSIGNED_BYTE, nullptr);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
			});
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool srgb)
		: m_FilePath(path), m_RendererID(0)
	{
		PR_PROFILE_FUNCTION();

		int width, height, channels;
		PR_CORE_INFO("Loading texture {0}, srgb={1}", path, srgb);
		m_ImageData.Data = stbi_load(path.c_str(), &width, &height, &channels, srgb ? STBI_rgb : STBI_rgb_alpha);

		m_Width = width;
		m_Height = height;
		m_Format = TextureFormat::RGBA;

		PR_RENDER_S1(srgb, {
			// TODO: Consolidate properly
			if (srgb)
			{
				glCreateTextures(GL_TEXTURE_2D, 1, &self->m_RendererID);
				int levels = CalculateMipMapCount(self->m_Width, self->m_Height);
				PR_CORE_INFO("Creating srgb texture width {0} mips", levels);
				glTextureStorage2D(self->m_RendererID, levels, GL_SRGB8, self->m_Width, self->m_Height);
				glTextureParameteri(self->m_RendererID, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
				glTextureParameteri(self->m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glTextureSubImage2D(self->m_RendererID, 0, 0, 0, self->m_Width, self->m_Height, GL_RGB, GL_UNSIGNED_BYTE, self->m_ImageData.Data);
				glGenerateTextureMipmap(self->m_RendererID);
			}
			else
			{
				glGenTextures(1, &self->m_RendererID);
				glBindTexture(GL_TEXTURE_2D, self->m_RendererID);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				glTexImage2D(GL_TEXTURE_2D, 0, PrismToOpenGLTextureFormat(self->m_Format), self->m_Width, self->m_Height, 0, srgb ? GL_SRGB8 : PrismToOpenGLTextureFormat(self->m_Format), GL_UNSIGNED_BYTE, self->m_ImageData.Data);
				glGenerateMipmap(GL_TEXTURE_2D);

				glBindTexture(GL_TEXTURE_2D, 0);
			}
			stbi_image_free(self->m_ImageData.Data);
			});
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		PR_RENDER_S({
			glDeleteTextures(1, &self->m_RendererID);
			});
	}

	void OpenGLTexture2D::Bind(unsigned int slot) const
	{
		PR_RENDER_S1(slot, {
			glBindTextureUnit(slot, self->m_RendererID);
			});
	}
	void OpenGLTexture2D::Lock()
	{
		m_Locked = true;
	}

	void OpenGLTexture2D::Unlock()
	{
		m_Locked = false;
		PR_RENDER_S({
			glTextureSubImage2D(self->m_RendererID, 0, 0, 0, self->m_Width, self->m_Height, PrismToOpenGLTextureFormat(self->m_Format), GL_UNSIGNED_BYTE, self->m_ImageData.Data);
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

	//////////////////////////////////////////////////////////////////////////////////
	// TextureCube
	//////////////////////////////////////////////////////////////////////////////////

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

		unsigned int faceWidth = m_Width / 4;
		unsigned int faceHeight = m_Height / 3;
		PR_CORE_ASSERT(faceWidth == faceHeight, "Non-square faces!");

		std::array<unsigned char*, 6> faces;
		for (size_t i = 0; i < faces.size(); i++)
			faces[i] = new unsigned char[faceWidth * faceHeight * 3]; // 3 BPP

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

		PR_RENDER_S3(faces, faceWidth, faceHeight, {
			glGenTextures(1, &self->m_RendererID);
			glBindTexture(GL_TEXTURE_CUBE_MAP, self->m_RendererID);

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameterf(self->m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, RendererAPI::GetCapabilities().MaxAnisotropy);

			auto format = PrismToOpenGLTextureFormat(self->m_Format);
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

			stbi_image_free(self->m_ImageData);
			});
	}

	OpenGLTextureCube::~OpenGLTextureCube()
	{
		auto self = this;
		PR_RENDER_1(self, {
			glDeleteTextures(1, &self->m_RendererID);
			});
	}

	void OpenGLTextureCube::Bind(unsigned int slot) const
	{
		PR_RENDER_S1(slot, {
			glBindTextureUnit(slot, self->m_RendererID);
			});
	}

}