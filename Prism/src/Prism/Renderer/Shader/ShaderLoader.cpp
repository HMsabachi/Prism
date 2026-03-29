#include "prpch.h"
#include "ShaderLoader.h"

namespace Prism
{
	void ShaderLibrary::Add(const std::string& name, const Ref<PrismShader>& shader)
	{
		PR_CORE_ASSERT(!Exists(name), "Shader already exists!");
		m_Shaders[name] = shader;
	}
	void ShaderLibrary::Add(const Ref<PrismShader>& shader)
	{
		auto& name = shader->GetName();
		Add(name, shader);
	}

	Ref<PrismShader> ShaderLibrary::Load(const std::string& filePath)
	{
		auto shader = PrismShader::Create(filePath);
		Add(shader);
		return shader;
	}

	Ref<PrismShader> ShaderLibrary::Load(const std::string& name, const std::string& filePath)
	{
		auto shader = PrismShader::Create(filePath);
		Add(name, shader);
		return shader;
	}
	Ref<PrismShader> ShaderLibrary::Get(const std::string& name)
	{
		PR_CORE_ASSERT(Exists(name), "Shader does not exist!");
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exists(const std::string& name)
	{
		return m_Shaders.find(name)!= m_Shaders.end();
	}
}