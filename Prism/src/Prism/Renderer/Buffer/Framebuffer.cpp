#include "prpch.h"
#include "Framebuffer.h"

#include "Platform/OpenGL/Buffer/OpenGLFramebuffer.h"

namespace Prism 
{

	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		Ref<Framebuffer> result = nullptr;

		switch (RendererAPI::Current())
		{	
		case RendererAPIType::None:		return nullptr;
		case RendererAPIType::OpenGL:	result = CreateRef<OpenGLFramebuffer>(spec);
		}
		FramebufferPool::GetGlobal()->Add(result);
		return result;
	}

	FramebufferPool* FramebufferPool::s_Instance = new FramebufferPool;

	FramebufferPool::FramebufferPool(uint32_t maxFBs /* = 32 */)
	{

	}

	FramebufferPool::~FramebufferPool()
	{

	}

	std::weak_ptr<Framebuffer> FramebufferPool::AllocateBuffer()
	{
		// m_Pool.push_back();
		return std::weak_ptr<Framebuffer>();
	}

	void FramebufferPool::Add(std::weak_ptr<Framebuffer> framebuffer)
	{
		m_Pool.push_back(framebuffer);
	}

}