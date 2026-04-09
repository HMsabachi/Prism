#include "prpch.h"
#include "PrismShader.h"
#include "Prism/Utilities/Utilities.h"
#if 1

namespace Prism
{
	std::vector<PrismShader*> PrismShader::s_AllShaders;

	Ref<PrismShader> PrismShader::Create(const std::string& source, const bool isFile)
	{
		std::string src;
		if (isFile)
			src = ReadFile(source);
		else
			src = source;
		Ref<PrismShader> shader = CreateRef<PrismShader>(src);
		if (isFile)	shader->m_FilePath = source;
		return shader;
	}

	PrismShader::PrismShader(const std::string& source)
	{
		PrismShaderParser parser;
		m_ParseResult = parser.Parse(source);
		m_Name = m_ParseResult.ShaderName;
		m_Shader.reset(Shader::Create(m_Name, m_ParseResult.Passes[0].VertexShaderCode, m_ParseResult.Passes[0].FragmentShaderCode));
		m_ShaderProperty.Init(m_ParseResult.Properties);
		SetProperty(m_ShaderProperty.GetDefaultValueBuffer());

		s_AllShaders.push_back(this);
	}
	PrismShader::~PrismShader()
	{

	}
	
	void PrismShader::Reload()
	{
		auto source = ReadFile(m_FilePath);
		PrismShaderParser parser;
		m_ParseResult = parser.Parse(source);
		m_Name = m_ParseResult.ShaderName;
		m_Shader.reset(Shader::Create(m_Name, m_ParseResult.Passes[0].VertexShaderCode, m_ParseResult.Passes[0].FragmentShaderCode));
		m_ShaderProperty.Init(m_ParseResult.Properties);
		SetProperty(m_ShaderProperty.GetDefaultValueBuffer());
		for (const auto& callback : m_ReloadedCallbacks)
			callback();
	}

	void PrismShader::Bind() const
	{
		m_Shader->Bind();
	}

	void PrismShader::SetProperty(const Buffer& buffer)
	{
		m_Shader->SetProperty(m_ShaderProperty.GetDeclaration(), buffer);
	}

	void PrismShader::AddShaderReloadedCallback(const ShaderReloadedCallback& callback)
	{
		m_ReloadedCallbacks.push_back(callback);
	}

	std::string PrismShader::ReadFile(const std::string& filePath)
	{
		std::string result;
		std::ifstream in(filePath, std::ios::in, std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else
		{
			PR_CORE_WARN("Could not open file '{0}'", filePath);
		}
		return result;
	}

	Prism::Ref<Prism::Shader> PrismShader::GetOriginalShader() const
	{
		return m_Shader;
	}

	

	std::string PrismShader::ToString() const
	{
		const PrismShader& shader = *this;
		std::string result = fmt::format("PrismShader(Name: \"{}\", Path: \"{}\")\n",
			shader.GetName(), shader.GetFilePath());

		result += "Properties:\n";

		return result;
	}

}
#endif