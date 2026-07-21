#include "prpch.h"

#include "IndexBuffer.h"
#include "../Renderer.h"

#include "Platform/OpenGL/Buffer/OpenGLIndexBuffer.h"

namespace Prism {

	Ref<IndexBuffer> IndexBuffer::Create(void* data, uint32_t size)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    return nullptr;
		case RendererAPIType::OpenGL:  return Ref<OpenGLIndexBuffer>::Create(data, size);
		// case RendererAPIType::Vulkan: return Ref<VulkanIndexBuffer>::Create(data, size); // TODO
		}
		return nullptr;

	}
	Ref<IndexBuffer> IndexBuffer::Create(uint32_t size)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    return nullptr;
		case RendererAPIType::OpenGL:  return Ref<OpenGLIndexBuffer>::Create(size);
		// case RendererAPIType::Vulkan: return Ref<VulkanIndexBuffer>::Create(size); // TODO
		}
		PR_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}
