#pragma once

#include "Prism/Renderer/Shader.h"
#include <span>
#include <string>

namespace Prism
{

	class PRISM_API OpenGLShader : public Shader
	{
	public:
		OpenGLShader(std::span<const uint8_t> vertexSource, std::span<const uint8_t> fragmentSource);
		OpenGLShader(std::span<const uint8_t> computeSource);
		virtual ~OpenGLShader();

		RendererID GetRendererID() const { return m_RendererID; }

        void RT_Bind() const;

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
