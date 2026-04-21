#include "prpch.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Shader/PrismShader.h"
#include "Shader/GlobalUniforms.h"

#include "SceneRenderer.h"
#include "RendererAPI.h"
#include "Renderer2D.h"

#include "Camera/Camera.h"

#include <glad/glad.h>

namespace Prism
{
	RendererAPIType RendererAPI::s_CurrentRendererAPI = RendererAPIType::OpenGL;

	struct RendererData
	{
		Ref<RenderPass> m_ActiveRenderPass;
		RenderCommandQueue m_CommandQueue;
		Ref<ShaderLibrary> m_ShaderLibrary;
		Ref<VertexArray> m_FullscreenQuadVertexArray;
	};

	static RendererData s_Data;

	void Renderer::Init()
	{
		s_Data.m_ShaderLibrary = Ref<ShaderLibrary>::Create();
		Renderer::Submit([]() { RendererAPI::Init(); });

		Renderer::GetShaderLibrary()->Load("Assets/Shaders/PrismPBR_Static.Shader");
		Renderer::GetShaderLibrary()->Load("Assets/Shaders/PrismPBR_Anim.Shader");

		GlobalUniforms::Init();
		SceneRenderer::Init();

		// Create fullscreen quad
		float x = -1;
		float y = -1;
		float width = 2, height = 2;
		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};

		QuadVertex* data = new QuadVertex[4];

		data[0].Position = glm::vec3(x, y, 0.1f);
		data[0].TexCoord = glm::vec2(0, 0);

		data[1].Position = glm::vec3(x + width, y, 0.1f);
		data[1].TexCoord = glm::vec2(1, 0);

		data[2].Position = glm::vec3(x + width, y + height, 0.1f);
		data[2].TexCoord = glm::vec2(1, 1);

		data[3].Position = glm::vec3(x, y + height, 0.1f);
		data[3].TexCoord = glm::vec2(0, 1);

		s_Data.m_FullscreenQuadVertexArray = VertexArray::Create();
		auto quadVB = VertexBuffer::Create(data, 4 * sizeof(QuadVertex));
		quadVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" , VertexSemantic::Position},
			{ ShaderDataType::Float2, "a_TexCoord" , VertexSemantic::TexCoord0}
			});

		uint32_t indices[6] = { 0, 1, 2, 2, 3, 0, };
		auto quadIB = IndexBuffer::Create(indices, 6 * sizeof(uint32_t));

		s_Data.m_FullscreenQuadVertexArray->AddVertexBuffer(quadVB);
		s_Data.m_FullscreenQuadVertexArray->SetIndexBuffer(quadIB);

		Renderer2D::Init();
	}

	Ref<ShaderLibrary> Renderer::GetShaderLibrary()
	{
		return s_Data.m_ShaderLibrary;
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

	void Renderer::DrawIndexed(uint32_t count, PrimitiveType type, bool depthTest)
	{
		Renderer::Submit([=]() {
			RendererAPI::DrawIndexed(count, type, depthTest);
		});
	}
	void Renderer::SetLineThickness(float thickness)
	{
		Renderer::Submit([=]() {
			RendererAPI::SetLineThickness(thickness);
		});
	}

	void Renderer::MemoryBarriers(RendererAPI::BarrierFlags flags)
	{
		Renderer::Submit([=]() {
			RendererAPI::MemoryBarriers(flags);
		});
	}

	void Renderer::WaitAndRender()
	{
		s_Data.m_CommandQueue.Execute();
	}

	void Renderer::BeginRenderPass(Ref<RenderPass> renderPass, bool clear)
	{
		PR_CORE_ASSERT(renderPass, "渲染通道不能为空！");

		s_Data.m_ActiveRenderPass = renderPass;
		renderPass->GetSpecification().TargetFramebuffer->Bind();
		if (clear)
		{
			const glm::vec4& clearColor = renderPass->GetSpecification().TargetFramebuffer->GetSpecification().ClearColor;
			Renderer::Submit([=]() {
				RendererAPI::Clear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
			});
		}
	}

	void Renderer::EndRenderPass()
	{
		PR_CORE_ASSERT(s_Data.m_ActiveRenderPass, "没有活动的渲染通道！您是否调用了两次 Renderer::EndRenderPass？");
		s_Data.m_ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();
		s_Data.m_ActiveRenderPass = nullptr;
	}

	

	void Renderer::SubmitQuad(Ref<MaterialInstance> material, const glm::mat4& transform)
	{
		bool depthTest = true;
		if (material)
		{
			material->Bind();
			auto shader = material->GetShader();
			shader->SetMat4("Prism_Model", transform);
		}
		s_Data.m_FullscreenQuadVertexArray->Bind();
		Renderer::DrawIndexed(6, PrimitiveType::Triangles, depthTest);
	}
	void Renderer::SubmitFullscreenQuad(Ref<MaterialInstance> material)
	{
		bool depthTest = true;
		if (material)
			material->Bind();
		s_Data.m_FullscreenQuadVertexArray->Bind();
		Renderer::DrawIndexed(6, PrimitiveType::Triangles, depthTest);
	}
	void Renderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform, Ref<MaterialInstance> overrideMaterial)
	{
		// auto material = overrideMaterial ? overrideMaterial : mesh->GetMaterialInstance();
		// auto shader = material->GetShader();
		mesh->m_VertexArray->Bind();
		auto& materials = mesh->GetMaterials();
		for (Submesh& submesh : mesh->m_Submeshes)
		{
			auto material = materials[submesh.MaterialIndex];
			auto shader = material->GetShader();
			material->Bind();
			if (mesh->m_IsAnimated)
			{
				for (size_t i = 0; i < mesh->m_BoneTransforms.size(); i++)
				{
					std::string uniformName = std::string("u_BoneTransforms[") + std::to_string(i) + std::string("]");
					mesh->m_MeshShader->SetMat4(uniformName, mesh->m_BoneTransforms[i]);
				}
			}
			shader->SetMat4("Prism_Model", transform * submesh.Transform);
			Renderer::Submit([submesh, material]() {
				glDrawElementsBaseVertex(GL_TRIANGLES, submesh.IndexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * submesh.BaseIndex), submesh.BaseVertex);
			});
		}
	}

	void Renderer::DrawAABB(Ref<Mesh> mesh, const glm::mat4& transform, const glm::vec4& color)
	{
		for (Submesh& submesh : mesh->m_Submeshes)
		{
			auto& aabb = submesh.BoundingBox;
			auto aabbTransform = transform * submesh.Transform;
			DrawAABB(aabb, aabbTransform);
		}
	}

	void Renderer::DrawAABB(const AABB& aabb, const glm::mat4& transform, const glm::vec4& color /*= glm::vec4(1.0f)*/)
	{
		glm::vec4 min = { aabb.Min.x, aabb.Min.y, aabb.Min.z, 1.0f };
		glm::vec4 max = { aabb.Max.x, aabb.Max.y, aabb.Max.z, 1.0f };

		glm::vec4 corners[8] =
		{
			transform * glm::vec4 { aabb.Min.x, aabb.Min.y, aabb.Max.z, 1.0f },
			transform * glm::vec4 { aabb.Min.x, aabb.Max.y, aabb.Max.z, 1.0f },
			transform * glm::vec4 { aabb.Max.x, aabb.Max.y, aabb.Max.z, 1.0f },
			transform * glm::vec4 { aabb.Max.x, aabb.Min.y, aabb.Max.z, 1.0f },

			transform * glm::vec4 { aabb.Min.x, aabb.Min.y, aabb.Min.z, 1.0f },
			transform * glm::vec4 { aabb.Min.x, aabb.Max.y, aabb.Min.z, 1.0f },
			transform * glm::vec4 { aabb.Max.x, aabb.Max.y, aabb.Min.z, 1.0f },
			transform * glm::vec4 { aabb.Max.x, aabb.Min.y, aabb.Min.z, 1.0f }
		};

		for (uint32_t i = 0; i < 4; i++)
			Renderer2D::DrawLine(corners[i], corners[(i + 1) % 4], color);

		for (uint32_t i = 0; i < 4; i++)
			Renderer2D::DrawLine(corners[i + 4], corners[((i + 1) % 4) + 4], color);

		for (uint32_t i = 0; i < 4; i++)
			Renderer2D::DrawLine(corners[i], corners[i + 4], color);
	}

	RenderCommandQueue& Renderer::GetRenderCommandQueue()
	{
		return s_Data.m_CommandQueue;
	}

}