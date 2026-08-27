#include "prpch.h"
#include "ShaderStorageBuffer.h"
#include "Platform/OpenGL/OpenGLShaderStorageBuffer.h"
#include "Platform/Vulkan/VulkanShaderStorageBuffer.h"

namespace Prism
{

	Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create(uint32_t size, BufferUsage usage /*= BufferUsage::Dynamic*/)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:   return nullptr;
		case RendererAPIType::OpenGL:  return Ref<OpenGLShaderStorageBuffer>::Create(size, usage);
		case RendererAPIType::Vulkan: return Ref<VulkanShaderStorageBuffer>::Create(size, usage);
		}
        return nullptr;
	}

}
