#include "prpch.h"
#include "Shader.h"

#include "Prism/ShaderCompiler/ShaderCompiler.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Prism
{

	Ref<Shader> Shader::Create(const PrismShaderCompiler::CompiledShader& shader,
		uint32_t passIndex,
		const std::vector<std::string>& keywords)
	{
		auto& compiler = ShaderCompiler::Get();
		switch (RendererAPI::Current())
		{
		case RendererAPIType::OpenGL:
		{
			auto out = compiler.GenerateGLSL(shader, passIndex, keywords);
			return Ref<Shader>(new OpenGLShader(out.VertexShader, out.FragmentShader));
		}
		// case RendererAPIType::Vulkan:
		// {
		// 	auto out = compiler.GenerateSPIRV(shader, passIndex, keywords);
		// 	return Ref<Shader>(new VulkanShader(out.SpirvVertex, out.SpirvFragment, out.Reflection));
		// }
		default:
			PR_CORE_ASSERT(false, "Unknown RendererAPI!"); return nullptr;
		}
	}

	Ref<Shader> Shader::Create(const void* computeSource)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::OpenGL:
			return Ref<Shader>(new OpenGLShader((const char*)computeSource));
		// case RendererAPIType::Vulkan:
		//     return Ref<Shader>(new VulkanShader(computeSource));
		default:
			PR_CORE_ASSERT(false, "Unknown RendererAPI!"); return nullptr;
		}
	}

}
