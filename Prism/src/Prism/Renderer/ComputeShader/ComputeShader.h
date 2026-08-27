#pragma once
#include <PrismShaderCore/CompilerCompute.h>
#include "Prism/Renderer/RendererAPI.h"

#include <unordered_map>

namespace Prism
{
	class Texture2D;
	class TextureCube;

	class ComputeShader : public RefCounted
	{
	public:
		static Ref<ComputeShader> Create(const std::string& filePath);

		ComputeShader(const std::string& filePath);
		~ComputeShader();

		void Load();

		int32_t FindKernel(const std::string& name);

		void SetUniformBuffer(int32_t kernel, const std::string& name, Ref<UniformBuffer> ubo);
		void SetBuffer(int32_t kernel, const std::string& name, Ref<ShaderStorageBuffer> ssbo);
		void SetTexture(int32_t kernel, const std::string& name, Ref<Texture> tex);
		void SetImage(int32_t kernel, const std::string& name, Ref<Texture> tex, uint32_t level = 0);

		void Dispatch(int32_t kernel, uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ);

	private:
		int32_t FindRes(const std::string& name);
		bool IsLegalID(int32_t kernel);

		struct Kernel
		{
			Ref<Shader> shader;
			std::string name;
			uint32_t groupSizeX = 1;
			uint32_t groupSizeY = 1;
			uint32_t groupSizeZ = 1;
		};

		std::vector<Kernel> m_Kernels;
		std::vector<ComputeResourceBinding> m_Resources;
		std::unordered_map<std::string, int32_t> m_ResourcesMap;

		PrismShaderCompiler::CompiledComputeShader m_Compiled;

		std::string m_Name;
		std::string m_FilePath;

	public:
		static std::vector<Ref<ComputeShader>> s_AllComputeShader;
	};
}
