#pragma once
#include "Prism/Renderer/Shader/Shader.h"
#include <string>
#include <glm/glm.hpp>

namespace Prism
{
	// TODO: 这里的PRESM_API仅供测试 该接口不应对外开放
	class PRISM_API OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& VertexShader, const std::string& FragmentShader);

		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;
	public:
		void UploadUniformInt(const std::string& name, int value) const;
		void UploadUniformIntArray(const std::string& name, int* values, uint32_t count) const;
		void UploadUniformFloat(const std::string& name, float value) const;
		void UploadUniformFloat2(const std::string& name, const glm::vec2& vector) const;
		void UploadUniformFloat3(const std::string& name, const glm::vec3& vector) const;
		void UploadUniformFloat4(const std::string& name, const glm::vec4& vector) const;


		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const;
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const;
	public:
		virtual void SetInt(const std::string& Name, int Value) const override;
		virtual void SetIntArray(const std::string& Name, int* Values, uint32_t Count) const override;
		virtual void SetFloat(const std::string& Name, float Value) const override;
		virtual void SetFloat2(const std::string& Name, const glm::vec2& Value) const override;
		virtual void SetFloat3(const std::string& Name, const glm::vec3& Value) const override;
		virtual void SetFloat4(const std::string& Name, const glm::vec4& Value) const override;
		virtual void SetMat3(const std::string& Name, const glm::mat3& Value) const override;
		virtual void SetMat4(const std::string& Name, const glm::mat4& Value) const override;
	private:
		static bool CompileShader(const std::string& shaderSource, unsigned int& ShaderID, unsigned int type);
		static bool LinkShaders(unsigned int& programID, unsigned int vertexShader, unsigned int fragmentShader);
	private:
		unsigned int m_RendererID;
	};
}