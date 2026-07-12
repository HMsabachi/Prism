#pragma once

namespace Prism
{
	struct ComputeShaderResult;
}

namespace Prism
{
	class ComputeShaderParser
	{
	public:
		static ComputeShaderResult Parse(const std::string& shaderCode);
	};
}
