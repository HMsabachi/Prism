#include "prpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Shader/PrismShader.h"
#include "glm/ext/matrix_transform.inl"

namespace Prism
{
	struct Renderer2DStorage
	{
		Ref<VertexArray> QuadVertexArray;
		Ref<PrismShader> FlatColorShader;
	};

	static Renderer2DStorage* s_Data;

	void Renderer2D::Init()
	{
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
	}

	void Renderer2D::Shutdown()
	{
		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		Renderer::UpdateGlobalUniform(camera);
	}

	void Renderer2D::EndScene()
	{

	}

	#pragma region Primitives 基本元素 
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		s_Data->FlatColorShader->GetOriginalShader()->Bind();
		s_Data->FlatColorShader->GetOriginalShader()->SetFloat4("_MainColor", color);
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position))
			* glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));
		s_Data->FlatColorShader->GetOriginalShader()->SetMat4("Prism_Model", transform);

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
		
	}
	#pragma endregion 
}