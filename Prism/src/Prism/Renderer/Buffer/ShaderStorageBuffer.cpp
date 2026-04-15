#include "prpch.h"
#include "ShaderStorageBuffer.h"
#include "Platform/OpenGL/Buffer/OpenGLShaderStorageBuffer.h"

namespace Prism
{

	Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create(uint32_t size, BufferUsage usage /*= BufferUsage::Dynamic*/)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:   return nullptr;
		case RendererAPIType::OpenGL:  return CreateRef<OpenGLShaderStorageBuffer>(size, usage);
		}
	}

}