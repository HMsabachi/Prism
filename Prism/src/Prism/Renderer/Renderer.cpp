#include "prpch.h"
#include "Renderer.h"
#include "RenderCommand.h"

namespace Prism
{
	void Renderer::Init()
	{

	}

	void Renderer::Clear()
	{
		
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


	void Renderer::WaitAndRender()
	{
		s_Instance->m_CommandQueue.Execute();
	}

	Prism::Renderer* Renderer::s_Instance = new Renderer();

}