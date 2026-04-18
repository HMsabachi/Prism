#pragma once
#include "Prism/Renderer/RendererAPI.h"

namespace Prism {

	enum class PRISM_API FramebufferFormat
	{
		None = 0,
		RGBA8 = 1,
		RGBA16F = 2
	};

	struct PRISM_API FramebufferSpecification
	{
		uint32_t Width = 1280;
		uint32_t Height = 720;
		glm::vec4 ClearColor;
		FramebufferFormat Format;
		uint32_t Samples = 1; // multisampling

		// SwapChainTarget = screen buffer (i.e. no framebuffer)
		bool SwapChainTarget = false;
	};

	class PRISM_API Framebuffer
	{
	public:
		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

		virtual const FramebufferSpecification& GetSpecification() const = 0;

		virtual ~Framebuffer() {}
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual void BindTexture(uint32_t slot = 0) const = 0;
		virtual void BindDepthTexture(uint32_t slot = 0) const = 0;

		virtual RendererID GetRendererID() const = 0;
		virtual RendererID GetColorAttachmentRendererID() const = 0;
		virtual RendererID GetDepthAttachmentRendererID() const = 0;
	};

	class PRISM_API FramebufferPool final
	{
	public:
		FramebufferPool(uint32_t maxFBs = 32);
		~FramebufferPool();

		std::weak_ptr<Framebuffer> AllocateBuffer();
		void Add(std::weak_ptr<Framebuffer> framebuffer);

		const std::vector<std::weak_ptr<Framebuffer>>& GetAll() const { return m_Pool; }

		inline static FramebufferPool* GetGlobal() { return s_Instance; }
	private:
		std::vector<std::weak_ptr<Framebuffer>> m_Pool;

		static FramebufferPool* s_Instance;
	};

}