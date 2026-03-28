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
		void UploadUniformInt(const std::string& name, int value);
		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& vector);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& vector);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& vector);


		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
	private:
		static bool CompileShader(const std::string& shaderSource, unsigned int& ShaderID, unsigned int type);
		static bool LinkShaders(unsigned int& programID, unsigned int vertexShader, unsigned int fragmentShader);
	private:
		unsigned int m_RendererID;
	};
}