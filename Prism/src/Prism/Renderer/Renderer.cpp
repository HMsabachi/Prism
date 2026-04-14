#include "prpch.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Shader/PrismShader.h"

#include "Camera/Camera.h"

#include <glad/glad.h>

namespace Prism
{
	Renderer* Renderer::s_Instance = new Renderer();
	RendererAPIType RendererAPI::s_CurrentRendererAPI = RendererAPIType::OpenGL;

	void Renderer::Init()
	{
		s_Instance->m_ShaderLibrary = std::make_unique<ShaderLibrary>();
		Renderer::Submit([]() { RendererAPI::Init(); });

		Renderer::GetShaderLibrary()->Load("Assets/Shaders/PrismPBR_Static.Shader");
		Renderer::GetShaderLibrary()->Load("Assets/Shaders/PrismPBR_Anim.Shader");
	}
	Renderer::Renderer()
	{
	}
	Renderer::~Renderer()
	{
	}
	void Renderer::Clear()
	{
		Renderer::Submit([]() {
			RendererAPI::Clear(0.0f, 0.0f, 0.0f, 1.0f);
		});
	}
	void Renderer::Clear(float r, float g, float b, float a /*= 1.0f*/)
	{
		Renderer::Submit([=]() {
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
		Renderer::Submit([=]() {
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
		const glm::vec4& clearColor = renderPass->GetSpecification().TargetFramebuffer->GetSpecification().ClearColor;
		Renderer::Submit([=]() {
			RendererAPI::Clear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
			});
		UpdateGlobalsUBO(*m_ActiveCamera);
	}

	void Renderer::IEndRenderPass()
	{
		PR_CORE_ASSERT(m_ActiveRenderPass, "没有活动的渲染通道！您是否调用了两次 Renderer::EndRenderPass？");
		m_ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();
		m_ActiveRenderPass = nullptr;
	}

	

	void Renderer::SubmitMeshI(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<MaterialInstance>& overrideMaterial)
	{
		if (overrideMaterial)
		{
			overrideMaterial->Bind();
		}
		else
		{
			// Bind mesh material here
		}

		// TODO: 解决这个问题
		mesh->m_VertexArray->Bind();
		// TODO: 替换为渲染 API 调用
		Renderer::Submit([=]()
			{
				for (Submesh& submesh : mesh->m_Submeshes)
				{
					if (mesh->m_IsAnimated)
					{
						for (size_t i = 0; i < mesh->m_BoneTransforms.size(); i++)
						{
							std::string uniformName = std::string("u_BoneTransforms[") + std::to_string(i) + std::string("]");
							mesh->m_MeshShader->SetMat4FromRenderThread(uniformName, mesh->m_BoneTransforms[i]);
						}
					}

					glDrawElementsBaseVertex(GL_TRIANGLES, submesh.IndexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * submesh.BaseIndex), submesh.BaseVertex);
				}
			});
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
		Renderer::Submit([this](){
			Prism::GlobalUniforms::UpdateGlobalUniform(m_GlobalsUBO);
			});
	}

}