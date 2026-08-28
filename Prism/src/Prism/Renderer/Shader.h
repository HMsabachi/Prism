#pragma once

#include "Prism/Core/Core.h"
#include "Prism/Renderer/RendererAPI.h"

#include <vector>
#include <string>

namespace PrismShaderCompiler { struct CompiledShader; struct CompiledComputeShader; }

namespace Prism
{
	class PRISM_API Shader : public RefCounted
	{
	public:
		virtual ~Shader() = default;

		static Ref<Shader> Create(const PrismShaderCompiler::CompiledShader& shader,
			uint32_t passIndex,
			const std::vector<std::string>& keywords = {});
		static Ref<Shader> Create(const PrismShaderCompiler::CompiledComputeShader& shader,
			uint32_t kernelIndex);
	};

}
