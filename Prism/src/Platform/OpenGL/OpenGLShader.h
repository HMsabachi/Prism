#pragma once

#include "Prism/Renderer/Shader.h"
#include <string>

namespace Prism
{

	class PRISM_API OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& vertexSource, const std::string& fragmentSource);
		OpenGLShader(const char* computeSource);
		virtual ~OpenGLShader();

		RendererID GetRendererID() const { return m_RendererID; }

	private:
		void CompileAndUploadShader();

	private:
		RendererID m_RendererID = 0;
		std::string m_VertexSource;
		std::string m_FragmentSource;
		std::string m_ComputeSource;
		bool m_IsCompute = false;
	};

}
