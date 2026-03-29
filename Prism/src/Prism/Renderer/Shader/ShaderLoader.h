#pragma once
#include "PrismShader.h"

namespace Prism
{
	class PRISM_API ShaderLibrary
	{
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
}