#pragma once
#include "PrismShader.h"

namespace Prism
{
	class PRISM_API ShaderLibrary
	{
	private:
		friend std::string format_as(const ShaderLibrary& library);
	public:
		void Add(const std::string& name, const Ref<PrismShader>& shader);
		void Add(const Ref<PrismShader>& shader);
		Ref<PrismShader> Load(const std::string& filePath);
		Ref<PrismShader> Load(const std::string& name, const std::string& filePath);
		
		Ref<PrismShader> Get(const std::string& name);

		bool Exists(const std::string& name);
	private:
		std::unordered_map<std::string, Ref<PrismShader>> m_Shaders;
	};

	inline std::string format_as(const ShaderLibrary& library)
	{
		std::string result = "Shader Library:\n";
		for (auto& [name, shader] : library.m_Shaders)
		{
			result += "\t" + name + " : " + shader->GetFilePath() + "\n";
		}
		return result;
	}
}