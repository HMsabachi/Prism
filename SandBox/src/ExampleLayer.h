#pragma once
#include "Prism.h"
#include "imgui/imgui.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class ExampleLayer : public Prism::Layer
{
public:
	ExampleLayer();

	void OnImGuiRender() override;
	void OnUpdate() override;

	void OnEvent(Prism::Event& event) override;
#pragma region 事件处理 Event Handling
private:
	bool OnKeyPressedEvent(Prism::KeyPressedEvent& e);
#pragma endregion

private:
	Prism::ShaderLibrary m_ShaderLibrary;
	Prism::Ref<Prism::PrismShader> m_Shader, m_TextureShader;
	Prism::Ref<Prism::VertexArray> m_VertexArray;

	Prism::Ref<Prism::VertexArray> m_SquareVA;
	Prism::Ref<Prism::Texture2D> m_TestTexture;

	Prism::OrthographicCameraController m_CameraController;
	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};