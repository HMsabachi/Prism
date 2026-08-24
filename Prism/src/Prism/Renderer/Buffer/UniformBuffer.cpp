#include "prpch.h"
#include "UniformBuffer.h"
#include "../Renderer.h"

#include "Platform/OpenGL/OpenGLUniformBuffer.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"

namespace Prism {

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    return nullptr;
		case RendererAPIType::OpenGL:  return Ref<OpenGLUniformBuffer>::Create(size);
		case RendererAPIType::Vulkan:  return Ref<VulkanUniformBuffer>::Create(size);
		}
		return nullptr;
	}

}
