#pragma once
#if 1
#include "Prism/Core/Core.h"
#include "Prism/Renderer/Shader.h"
#include "PrismShaderParser.h"

#include "ShaderProperty.h"


namespace Prism
{
	using ShaderReloadedCallback = std::function<void()>;
	class PRISM_API PrismShader
	{
	public:
		static Ref<PrismShader> Create(const std::string& source, const bool isFile = true);
		~PrismShader();

		Ref<Shader> GetOriginalShader() const;
	public:
		PrismShader(const std::string& source);
		void Reload();

	public:
		void Bind() const;
		void SetProperty(const Buffer& buffer);

		void AddShaderReloadedCallback(const ShaderReloadedCallback& callback);
		const std::string& GetFilePath() const {return m_FilePath; };
		const std::string& GetName() const { return m_ParseResult.ShaderName; }
		const ShaderProperty& GetProperty() const { return m_ShaderProperty; }
	#pragma region 设置原生uniform
	public:
		void SetMat4FromRenderThread(const std::string& name, const glm::mat4& value);
	#pragma endregion

		
	private:
		static std::string ReadFile(const std::string& filePath);
	private:
		std::string m_Name;
		std::string m_FilePath;
		Ref<Shader> m_Shader;
		ShaderProperty m_ShaderProperty;
		std::vector<ShaderReloadedCallback> m_ReloadedCallbacks;

	public:
		static std::vector<PrismShader*> s_AllShaders;
	private:


#pragma region 调试用
	public:
		const std::string GetSource() const { return m_ParseResult.Passes[0].VertexShaderCode + m_ParseResult.Passes[0].FragmentShaderCode; }
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