#include "prpch.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Shader/PrismShader.h"

namespace Prism
{
	Renderer* Renderer::s_Instance = new Renderer();
	RendererAPIType RendererAPI::s_CurrentRendererAPI = RendererAPIType::OpenGL;

	void Renderer::Init()
	{
		s_Instance->m_ShaderLibrary = std::make_unique<ShaderLibrary>();
		PR_RENDER({ RendererAPI::Init(); });

		Renderer::GetShaderLibrary()->Load("Assets/Shaders/PrismPBR_Static.glsl");
		Renderer::GetShaderLibrary()->Load("Assets/Shaders/PrismPBR_Anim.glsl");
	}

	Renderer::Renderer()
	{

	}

	Renderer::~Renderer()
	{

	}

	void Renderer::Clear()
	{
		PR_RENDER({
			RendererAPI::Clear(0.0f, 0.0f, 0.0f, 1.0f);
		});
	}

	void Renderer::Clear(float r, float g, float b, float a /*= 1.0f*/)
	{
		PR_RENDER_4(r, g, b, a, {
			RendererAPI::Clear(r, g, b, a);
		});
	}

	void Renderer::SetClearColor(float r, float g, float b, float a)
	{

	}

	void Renderer::ClearMagenta()
	{
		Clear(1, 0, 1);
	}

	void Renderer::DrawIndexed(unsigned int count, bool depthTest)
	{
		PR_RENDER_2(count, depthTest, {
			RendererAPI::DrawIndexed(count, depthTest);
		});
	}

	void Renderer::WaitAndRender()
	{
		s_Instance->m_CommandQueue.Execute();
	}
	const Prism::Scope<Prism::ShaderLibrary>& Renderer::GetShaderLibrary()
	{
		return Get().m_ShaderLibrary;
	}



}