#pragma once

namespace Prism
{
	class Shader;
	class ShaderStorageBuffer;
	class Texture2D;
	class TextureCube;
	enum class ComputeShaderResourceType;
}

namespace Prism
{
	class PRISM_API ComputeShader
	{
#pragma region 数据结构
		struct PRISM_API Kernel
		{
			Ref<Shader> shader;
			std::string name;
			uint32_t groupSizeX;
			uint32_t groupSizeY;
			uint32_t groupSizeZ;
		};
		struct PRISM_API Resource
		{
			ComputeShaderResourceType type;
			std::string name;
			uint32_t binding;
			bool layered = true;
			uint32_t level = 0;
			std::weak_ptr<Texture2D> texture2D;
			std::weak_ptr<TextureCube> textureCube;
			std::weak_ptr<ShaderStorageBuffer> ssbo;
		};
#pragma endregion
	public:
		static Ref<ComputeShader> Create(const std::string& filePath);
	public:
		ComputeShader(const std::string& filePath);
		~ComputeShader();

		void Load();
	public:
		int32_t FindKernel(const std::string& name);
		void SetBuffer(int32_t kernel, const std::string& name, Ref<ShaderStorageBuffer>& ssbo);
		void SetImage2D(int32_t kernel, const std::string& name, Ref<Texture2D>& tex, uint32_t level = 0, bool layered = true);
		void SetImageCube(int32_t kernel, const std::string& name, Ref<TextureCube>& tex, uint32_t level = 0, bool layered = true);
		void SetTexture2D(int32_t kernel, const std::string& name, Ref<Texture2D>& tex);
		void SetTextureCube(int32_t kernel, const std::string& name, Ref<TextureCube>& tex);

		void SetInt(int32_t kernel, const std::string& name, int32_t value);

		void SetFloat(int32_t kernel, const std::string& name, float value);

		void Dispatch(int32_t kernel, uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ);
	
	private:
		int32_t FindRes(const std::string& name);
		bool IsLegalID(int32_t kernel);
	private:
		std::vector<Kernel> m_Kernels;
		std::vector<Resource> m_Resources;
		std::unordered_map<std::string, int32_t> m_ResourcesMap;

		std::string m_Name;
		std::string m_FilePath;

	public:
		static std::vector<Ref<ComputeShader>> s_AllComputeShader;
	};
}