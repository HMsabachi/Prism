#pragma once
#include "Prism/Renderer/Framebuffer.h"

namespace Prism {

	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(uint32_t width, uint32_t height, FramebufferFormat format);
		virtual ~OpenGLFramebuffer();

		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void BindTexture(uint32_t slot = 0) const override;

		virtual RendererID GetRendererID() const { return m_RendererID; }
		virtual RendererID GetColorAttachmentRendererID() const { return m_ColorAttachment; }
		virtual RendererID GetDepthAttachmentRendererID() const { return m_DepthAttachment; }

		virtual uint32_t GetWidth() const { return m_Width; }
		virtual uint32_t GetHeight() const { return m_Height; }
		virtual FramebufferFormat GetFormat() const { return m_Format; }
	private:
		RendererID m_RendererID = 0;
		uint32_t m_Width, m_Height;
		FramebufferFormat m_Format;

		RendererID m_ColorAttachment, m_DepthAttachment;
	};

}