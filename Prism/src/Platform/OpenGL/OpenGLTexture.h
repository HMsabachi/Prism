#pragma once

#include "Prism/Renderer/Texture.h"

typedef unsigned int GLenum;

namespace Prism
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D (const uint32_t width, const uint32_t height);
		OpenGLTexture2D (const std::string& path);
		virtual ~OpenGLTexture2D();

		virtual int GetWidth() const override { return m_Width; }
		virtual int GetHeight() const override { return m_Height; }
		virtual void Bind(uint32_t slot = 0) const override;
		virtual void SetData(void* data, uint32_t size) const override;
	private:
		std::string m_Path;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
	};
}