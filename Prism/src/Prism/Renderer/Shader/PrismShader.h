#pragma once
#if 1
#include "Prism/Core/Core.h"
#include "Prism/Renderer/Shader.h"
#include "PrismShaderParser.h"
#include "PrismShaderProperty.h"
#include "Platform/OpenGL/Shader/OpenGLShaderUniform.h"


namespace Prism
{
	class OpenGLShader;
	
	class PRISM_API PrismShader
	{
	public:
		static Ref<PrismShader> Create(const std::string& source, const bool isFile = true);
		~PrismShader();

		Ref<Shader> GetOriginalShader() const;
	public:
		PrismShader(const std::string& source);

	public:
		void bind() const;
		void SetPropertiesToShader(Buffer buffer);
		const std::string& GetFilePath() const {return m_FilePath; };
		const std::string& GetName() const { return m_ParseResult.ShaderName; }
		
		const ShaderData::propertyGroup& GetProperties() const { return m_Properties; }
		ShaderData::propertyGroup& GetProperties() { return m_Properties; }
	public:
		

	private:
		void HandleProperty();
		void HandlePropertyUniformBuffer();
		void InitPropertyBuffer();
	private:
		static std::string ReadFile(const std::string& filePath);
	private:
		std::string m_Name;
		ShaderData::propertyGroup m_Properties;
		std::string m_FilePath;
		Ref<OpenGLShader> m_Shader;
	private:
		static std::vector<PrismShader> s_AllPrismShader;
	private:
		Buffer m_PropertyBuffer;
		Scope<OpenGLShaderUniformBufferDeclaration> m_PropertiesDeclaration;

#pragma region 调试用
	public:
		const std::string& GetSource() const { return m_ParseResult.Passes[0].VertexShaderCode + m_ParseResult.Passes[0].FragmentShaderCode; }
	private:
		ParseResult m_ParseResult;
	public:
		std::string ToString() const;
	};

	inline std::string format_as(const PrismShader& shader)
	{
		return shader.ToString();
	}
#pragma endregion

}



#endif