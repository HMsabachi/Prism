#include "prpch.h"
#include "UniformBuffer.h"
#include "../Renderer.h"

#include "Platform/OpenGL/Buffer/OpenGLUniformBuffer.h"

namespace Prism {

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    return nullptr;
		case RendererAPIType::OpenGL:  return Ref<OpenGLUniformBuffer>::Create(size);
		// case RendererAPIType::Vulkan: return Ref<VulkanUniformBuffer>::Create(size); // TODO
		}
		return nullptr;
	}

}
