#include "prpch.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Shader/PrismShader.h"

#include "Camera/Camera.h"

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

	void Renderer::DrawIndexed(uint32_t count, bool depthTest)
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


	// 渲染流程

	void Renderer::IBeginScene(const Camera& camera)
	{
		m_ActiveCamera = &camera;
	}
	void Renderer::IEndScene()
	{

	}



	void Renderer::IBeginRenderPass(const Ref<RenderPass>& renderPass)
	{
		// TODO: 将所有这些转换为渲染命令缓冲区
		m_ActiveRenderPass = renderPass;

		renderPass->GetSpecification().TargetFramebuffer->Bind();

		UpdateGlobalsUBO(*m_ActiveCamera);
	}

	void Renderer::IEndRenderPass()
	{
		PR_CORE_ASSERT(m_ActiveRenderPass, "没有活动的渲染通道！您是否调用了两次 Renderer::EndRenderPass？");
		m_ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();
		m_ActiveRenderPass = nullptr;
	}

	

	void Renderer::SubmitMeshI(const Ref<Mesh>& mesh)
	{

	}


	void Renderer::UpdateGlobalsUBO(const Camera& camera)
	{
		const auto& framebufferSpec = m_ActiveRenderPass->GetSpecification().TargetFramebuffer->GetSpecification();
		m_GlobalsUBO.AspectRatio = (float)framebufferSpec.Width / (float)framebufferSpec.Height;
		m_GlobalsUBO.CameraPosition = camera.GetPosition();
		m_GlobalsUBO.DeltaTime = Prism::Time::GetDeltaTime();
		m_GlobalsUBO.Projection = camera.GetProjectionMatrix();
		m_GlobalsUBO.View = camera.GetViewMatrix();
		m_GlobalsUBO.ViewProjection = m_GlobalsUBO.Projection * m_GlobalsUBO.View;
		m_GlobalsUBO.InverseViewProjection = inverse(m_GlobalsUBO.ViewProjection);
		float time = Prism::Time::GetTime();
		m_GlobalsUBO.Time = glm::vec4(time * 0.2f, time, time * 2, time * 3);
		PR_RENDER_S({
			Prism::GlobalUniforms::UpdateGlobalUniform(self->m_GlobalsUBO);
			});
	}

}