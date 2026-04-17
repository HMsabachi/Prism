#pragma once
#if 1
#include "Prism/Core/Core.h"
#include "Prism/Renderer/Shader.h"
#include "PrismShaderParser.h"

#include "ShaderProperty.h"
#include "ShaderCommand.h"


namespace Prism
{
	using ShaderReloadedCallback = std::function<void()>;
	class PRISM_API PrismShader
	{
	public:
		static Ref<PrismShader> Create(const std::string& path);
		static Ref<PrismShader> CreateFromString(const std::string& source);

		Ref<Shader> GetOriginalShader() const;
	public:
		PrismShader(const std::string& source);
		PrismShader();
		~PrismShader();
		void Reload();
		void Load(const std::string& source);

	public:
		void Bind() const;
		void SetProperty(const Buffer& buffer);

		void AddShaderReloadedCallback(const ShaderReloadedCallback& callback);
		const std::string& GetFilePath() const {return m_FilePath; };
		const std::string& GetName() const { return m_Name; }
		const ShaderProperty& GetProperty() const { return m_ShaderProperty; }
		const ShaderCommand& GetShaderCommand() const { return m_ShaderCommand; }
	#pragma region 设置原生uniform
	public:
		void SetMat4FromRenderThread(const std::string& name, const glm::mat4& value);
		void SetInt(const std::string& name, int value) const;
		void SetFloat(const std::string& name, float value) const;
		void SetVec3(const std::string& name, const glm::vec3& value) const;
		void SetVec4(const std::string& name, const glm::vec4& value) const;
		void SetMat4(const std::string& name, const glm::mat4& value) const;
	#pragma endregion

	private:
		std::string m_Name;
		std::string m_FilePath;
		Ref<Shader> m_Shader;
		ShaderProperty m_ShaderProperty;
		ShaderCommand m_ShaderCommand;
		std::vector<ShaderReloadedCallback> m_ReloadedCallbacks;

	public:
		static std::vector<Ref<PrismShader>> s_AllShaders;
	#pragma region 调试用
	public:
		const std::string GetSource() const { return m_ParseResult.Passes[0].VertexShaderCode + m_ParseResult.Passes[0].FragmentShaderCode; }
	private:
		ParseResult m_ParseResult;
	public:
		std::string ToString() const;
	
#pragma endregion
	};
	inline std::string format_as(const Prism::PrismShader& shader)
	{
		return shader.ToString();
	}

	// 目前这个类只是一个简单的容器，负责存储和管理所有的Shader实例，提供添加、加载和获取Shader的功能。未来可能会扩展为更复杂的资源管理系统。
	class PRISM_API ShaderLibrary
	{
	public:
		ShaderLibrary();
		~ShaderLibrary();

		void Add(const Ref<PrismShader>& shader);
		void Load(const std::string& path);
		void Load(const std::string& name, const std::string& path);

		Ref<PrismShader>& Get(const std::string& name);
	private:
		std::unordered_map<std::string, Ref<PrismShader>> m_Shaders;
	};

}
	

#endif