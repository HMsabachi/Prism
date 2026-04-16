#pragma once

namespace Prism
{
	class ComputeShaderResult;
}

namespace Prism
{
	class ComputeShaderParser
	{
	public:
		static ComputeShaderResult Parse(const std::string& shaderCode);
	};
}