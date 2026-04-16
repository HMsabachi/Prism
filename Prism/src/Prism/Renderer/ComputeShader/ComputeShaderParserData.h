#pragma once


namespace Prism
{

	enum class PRISM_API ComputeShaderResourceType
	{
		None,
		RBuffer,
		WBuffer,
		RWBuffer,
		RWImage2D
	};

	struct PRISM_API ComputeShaderResource
	{
		ComputeShaderResourceType type;
		std::string name;
		std::string param;
		uint32_t binding;
	};

	struct PRISM_API KernelInfo
	{
		std::string name;
		std::string source;
		StrParse::Lines::iterator line;
		int numThreads[3] = { 1, 1, 1 };
	};

	struct PRISM_API ComputeShaderResult
	{
		std::string source;
		std::vector<KernelInfo> kernels;
		std::vector<ComputeShaderResource> resources;
	};
}
