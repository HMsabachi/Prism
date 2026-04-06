#pragma once

#include "Prism/Renderer/Shader.h"
#include <Glad/glad.h>

namespace Prism {

	class PRISM_API OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filepath);
		OpenGLShader(const std::string& name, const std::string& source);

		virtual void Reload() override;

		virtual void Bind() override;

		virtual void UploadUniformBuffer(const UniformBufferBase& uniformBuffer) override;

		void SetFloat(const std::string& name, float value) override;
		void SetMat4(const std::string& name, const glm::mat4& value) override;

		const std::string& GetName() const override { return m_Name; }


	private:
		void ReadShaderFromFile(const std::string& filepath);
		void CompileAndUploadShader();

	private:
		void LoadTexture();

		static GLenum ShaderTypeFromString(const std::string& type);


		#pragma region 上传Uniform
		void UploadUniformInt(uint32_t location, int32_t value);
		void UploadUniformIntArray(uint32_t location, int32_t* values, int32_t count);
		void UploadUniformFloat(uint32_t location, float value);
		void UploadUniformFloat2(uint32_t location, const glm::vec2& value);
		void UploadUniformFloat3(uint32_t location, const glm::vec3& value);
		void UploadUniformFloat4(uint32_t location, const glm::vec4& value);
		void UploadUniformMat3(uint32_t location, const glm::mat3& values);
		void UploadUniformMat4(uint32_t location, const glm::mat4& values);
		void UploadUniformMat4Array(uint32_t location, const glm::mat4& values, uint32_t count);

		void UploadUniformInt(std::string name, int32_t value);
		void UploadUniformIntArray(std::string name, int32_t* values, int32_t count);
		void UploadUniformFloat(std::string name, float value);
		void UploadUniformFloat2(std::string name, const glm::vec2& value);
		void UploadUniformFloat3(std::string name, const glm::vec3& value);
		void UploadUniformFloat4(std::string name, const glm::vec4& value);
		void UploadUniformMat3(std::string name, const glm::mat3& values);
		void UploadUniformMat4(std::string name, const glm::mat4& values);
		void UploadUniformMat4Array(std::string name, const glm::mat4& values, uint32_t count);

		bool UniformLocationCache(std::string& name);
		#pragma endregion 上传Uniform

	private:
		RendererID m_RendererID;

		std::string m_Name, m_AssetsPath;
		std::string m_ShaderSource;

		std::unordered_map<std::string, int> m_UniformLocationCache;
	};

}