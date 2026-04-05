#pragma once
#if 1
#include "Prism/Core/Core.h"
#include "Prism/Renderer/Shader.h"
#include "PrismShaderParser.h"
#include "PrismShaderProperty.h"



namespace Prism
{
	
	class PRISM_API PrismShader
	{
	public:
		static Ref<PrismShader> Create(const std::string& source, const bool isFile = true);
		~PrismShader();

		Ref<Shader> GetOriginalShader() const { return m_Shader; }
	public:
		PrismShader(const std::string& source);

	public:
		

		const std::string& GetFilePath() const {return m_FilePath; };
		const std::string& GetName() const { return m_ParseResult.ShaderName; }
		const std::string& GetSource() const { return m_ParseResult.Passes[0].VertexShaderCode + m_ParseResult.Passes[0].FragmentShaderCode; }
		const ShaderData::propertyGroup& GetProperties() const { return m_Properties; }
		ShaderData::propertyGroup& GetProperties() { return m_Properties; }

	private:
		void HandleProperty();
	private:
		Ref<Shader> m_Shader;
		static std::string ReadFile(const std::string& filePath);
	private:
		ShaderData::propertyGroup m_Properties;
		std::string m_FilePath;
		ParseResult m_ParseResult;
	private:
		static std::vector<PrismShader> s_AllPrismShader;

	public:
		std::string ToString() const;
	};

	inline std::string format_as(const PrismShader& shader)
	{
		return shader.ToString();
	}
}



#endif