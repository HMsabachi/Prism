#include "prpch.h"
#include "OpenGLShader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Prism
{
	OpenGLShader::OpenGLShader(const std::string& VertexShader, const std::string& FragmentShader)
	{
		// 编译着色器 Compile Shaders
		GLuint vertexShaderID, fragmentShaderID;
		bool isCompiled = CompileShader(VertexShader, vertexShaderID, GL_VERTEX_SHADER);
		PR_CORE_ASSERT(isCompiled, "Vertex Shader Compilation Failed!");
		isCompiled = CompileShader(FragmentShader, fragmentShaderID, GL_FRAGMENT_SHADER);
		PR_CORE_ASSERT(isCompiled, "Fragment Shader Compilation Failed!");

		// 创建着色器程序 Create Shader Program
		bool isLinked = LinkShaders(m_RendererID, vertexShaderID, fragmentShaderID);
		PR_CORE_ASSERT(isLinked, "Shader Program Linking Failed!");

		// 删除着色器 Delete Shaders
		glDetachShader(m_RendererID, vertexShaderID);
		glDetachShader(m_RendererID, fragmentShaderID);
	}
	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}
	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}
	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

#pragma region 上传Uniforms
	void OpenGLShader::UploadUniformInt(const std::string& name, int value) const
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}
	void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count) const
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, values);
	}
	void OpenGLShader::UploadUniformFloat(const std::string& name, float value) const
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}
	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& vector) const
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, vector.x, vector.y);
	}
	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& vector) const
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, vector.x, vector.y, vector.z);
	}
	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& vector) const
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, vector.x, vector.y, vector.z, vector.w);
	}
	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}
	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}
	void OpenGLShader::SetInt(const std::string& Name, int Value) const
	{
		UploadUniformInt(Name, Value);
	}
	void OpenGLShader::SetIntArray(const std::string& Name, int* Values, uint32_t Count) const
	{
		UploadUniformIntArray(Name, Values, Count);
	}

	void OpenGLShader::SetFloat(const std::string& Name, float Value) const
	{
		UploadUniformFloat(Name, Value);
	}

	void OpenGLShader::SetFloat2(const std::string& Name, const glm::vec2& Value) const
	{
		UploadUniformFloat2(Name, Value);
	}

	void OpenGLShader::SetFloat3(const std::string& Name, const glm::vec3& Value) const
	{
		UploadUniformFloat3(Name, Value);
	}

	void OpenGLShader::SetFloat4(const std::string& Name, const glm::vec4& Value) const
	{
		UploadUniformFloat4(Name, Value);
	}

	void OpenGLShader::SetMat3(const std::string& Name, const glm::mat3& Value) const
	{
		UploadUniformMat3(Name, Value);
	}

	void OpenGLShader::SetMat4(const std::string& Name, const glm::mat4& Value) const
	{
		UploadUniformMat4(Name, Value);
	}

#pragma endregion

#pragma region 私有方法 Private Methods
	bool OpenGLShader::CompileShader(const std::string& shaderSource, unsigned int& ShaderID, unsigned int type)
	{
		ShaderID = glCreateShader(type);
		const GLchar* source = shaderSource.c_str();
		glShaderSource(ShaderID, 1, &source, nullptr);
		glCompileShader(ShaderID);
		GLint isCompiled = 0;
		glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(ShaderID, maxLength, &maxLength, &infoLog[0]);
			PR_CORE_ERROR("{0}", infoLog.data());
			return false;
		}
		return true;
	}
	bool OpenGLShader::LinkShaders(unsigned int& programID, unsigned int vertexShader, unsigned int fragmentShader)
	{
		programID = glCreateProgram();
		glAttachShader(programID, vertexShader);
		glAttachShader(programID, fragmentShader);
		glLinkProgram(programID);
		GLint isLinked = 0;
		glGetProgramiv(programID, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(programID, maxLength, &maxLength, &infoLog[0]);
			PR_CORE_ERROR("{0}", infoLog.data());
			return false;
		}
		return true;
	}
#pragma endregion
}