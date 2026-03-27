#include "prpch.h"
#include "OpenGLTexture.h"

#include <glad/glad.h>
#include <stb_image.h>

namespace Prism
{
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		:m_Path(path)
	{
		int width, height, channels;
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		PR_CORE_ASSERT(data, "Failed to load image {0}", path);
		m_Width = width;
		m_Height = height;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, GL_RGBA8, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);

	}
	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}
	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}
}