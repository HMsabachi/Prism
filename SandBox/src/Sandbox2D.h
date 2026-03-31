#pragma once
#include <Prism.h>

class Sandbox2D : public Prism::Layer
{
public:
	Sandbox2D();

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate() override;
	virtual void OnEvent(Prism::Event& event) override;
	virtual void OnImGuiRender() override;

private:
	Prism::OrthographicCameraController m_CameraController;
	Prism::ShaderLibrary m_ShaderLibrary;
	// Temp 临时
	Prism::Ref<Prism::PrismShader> m_Shader;
	Prism::Ref<Prism::Texture2D> m_Texture;
	Prism::Ref<Prism::VertexArray> m_SquareVA;

	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};