#pragma once

#include "Prism/Core.h"
#include <string>

namespace Prism
{
	class PRISM_API Shader
	{
	public:
		Shader(const std::string& VertexShader, const std::string& FragmentShader);
		
		~Shader();

		void Bind() const;
		void Unbind() const;
	private:
		static bool CompileShader(const std::string& shaderSource, unsigned int& ShaderID, unsigned int type);
		static bool LinkShaders(unsigned int& programID, unsigned int vertexShader, unsigned int fragmentShader);
	private:
		unsigned int m_RendererID;
	};
}

