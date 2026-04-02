#include "prpch.h"
#include "Renderer2D.h"
#include "Legacy/Renderer_Legacy.h"
#include "VertexArray.h"
#include "Shader/PrismShader.h"
#include "glm/ext/matrix_transform.inl"
#include "Renderer.h"

namespace Prism
{
	struct Renderer2DStorage
	{
		Ref<VertexArray> QuadVertexArray;
		Ref<PrismShader> FlatColorShader;
		Ref<PrismShader> TextureShader;
		Ref<Texture2D> WhiteTexture;
	};

	static Renderer2DStorage* s_Data;

	void Renderer2D::Init()
	{
		PR_PROFILE_FUNCTION();

		s_Data = new Renderer2DStorage();
		// 创建VertexArray 2
		s_Data->QuadVertexArray = Prism::VertexArray::Create();
		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};
		Ref<VertexBuffer> squareVB;
		squareVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position", VertexSemantic::Position },
			{ ShaderDataType::Float2, "a_TexCoord", VertexSemantic::TexCoord0}
			});
		s_Data->QuadVertexArray->AddVertexBuffer(squareVB);
		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Ref<IndexBuffer> squareIB;
		squareIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		s_Data->QuadVertexArray->SetIndexBuffer(squareIB);

		std::string flatColorShader = "Assets/Shaders/FlatColor.glsl";
		s_Data->FlatColorShader = PrismShader::Create(flatColorShader, true);
		s_Data->TextureShader = PrismShader::Create("Assets/Shaders/Texture.glsl", true);
		PR_CORE_TRACE("Shader Source 着色器代码: {0}", s_Data->FlatColorShader->GetSource());
		
		s_Data->WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
	}

	void Renderer2D::Shutdown()
	{
		PR_PROFILE_FUNCTION();

		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		PR_PROFILE_FUNCTION();

		Renderer_Legacy::UpdateGlobalUniform(camera);
	}

	void Renderer2D::EndScene()
	{
		PR_PROFILE_FUNCTION();

	}

	#pragma region Primitives 基本元素 
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position))
			* glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));
		PR_RENDER_II(transform, color, {
			DrawQuadWithMatrix(transform, color);
			};)
		
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const float tilingFactor, const glm::vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const float tilingFactor, const glm::vec4& tintColor)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position))
			* glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));
		PR_RENDER_III(transform, texture, tilingFactor, {
			DrawQuadWithMatrix(transform, texture, tilingFactor);
			})
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const float rotation, const glm::vec4& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float rotation, const glm::vec4& color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position))
			* glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));
		PR_RENDER_II(transform, color, {
			DrawQuadWithMatrix(transform, color);
			};)
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const float rotation, const Ref<Texture2D>& texture, const float tilingFactor, const glm::vec4& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float rotation, const Ref<Texture2D>& texture, const float tilingFactor, const glm::vec4& tintColor)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position))
			* glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));
		PR_RENDER_III(transform, texture, tilingFactor, {
			DrawQuadWithMatrix(transform, texture, tilingFactor);
			})
	}

	void Renderer2D::DrawQuadWithMatrix(const glm::mat4& transform, const glm::vec4& color)
	{
		s_Data->FlatColorShader->GetOriginalShader()->Bind();
		s_Data->FlatColorShader->GetOriginalShader()->SetMat4("Prism_Model", transform);
		s_Data->FlatColorShader->GetOriginalShader()->SetFloat4("_MainColor", color);
		s_Data->QuadVertexArray->Bind();
		s_Data->WhiteTexture->Bind(0);
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

	void Renderer2D::DrawQuadWithMatrix(const glm::mat4& transform, const Ref<Texture2D>& texture, const float tilingFactor, const glm::vec4& tintColor)
	{
		s_Data->TextureShader->GetOriginalShader()->Bind();
		s_Data->TextureShader->GetOriginalShader()->SetMat4("Prism_Model", transform);
		s_Data->TextureShader->GetOriginalShader()->SetInt("_MainTex", 0);
		s_Data->TextureShader->GetOriginalShader()->SetFloat4("_MainColor", tintColor);
		s_Data->TextureShader->GetOriginalShader()->SetFloat("_TillingFactor", tilingFactor);

		s_Data->QuadVertexArray->Bind();
		texture->Bind(0);
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

	Prism::PrismGlobalsUBO Renderer2D::s_GlobalUBO;

#pragma endregion 
}