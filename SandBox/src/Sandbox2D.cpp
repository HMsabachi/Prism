#include "Sandbox2D.h"

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "chrono"

template<typename fn>
class Timer
{
public:
	Timer(const char* name, fn&& Func)
		:m_Name(name), m_Func(Func), m_Stopped(false)
	{
		m_StartTimePoint = std::chrono::high_resolution_clock::now();
	}
	~Timer()
	{
		if (!m_Stopped)
			Stop();
	}
	void Stop()
	{
		auto endTimepoint = std::chrono::high_resolution_clock::now();
		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimePoint).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

		m_Stopped = true;

		float duration = (end - start) * 0.001f;
		m_Func({ m_Name, duration });
	}
private:
	const char* m_Name;
	fn m_Func;
	std::chrono::time_point<std::chrono::steady_clock> m_StartTimePoint;
	bool m_Stopped;

};

#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult) {m_ProfileResult.push_back(profileResult);})

Sandbox2D::Sandbox2D()
	:Layer("Sandbox2D"), m_CameraController(1920.0f / 1080.0f, true)
{

}

void Sandbox2D::OnAttach()
{
	Layer::OnAttach();
	m_Texture = Prism::Texture2D::Create("Assets/Textures/TestImage.png");
}

void Sandbox2D::OnDetach()
{
	Layer::OnDetach();
}

void Sandbox2D::OnUpdate()
{
	
	PROFILE_SCOPE("Sandbox2D::OnUpdate");
	Layer::OnUpdate();
	{
		PROFILE_SCOPE("CameraController::OnUpdate");
		m_CameraController.OnUpdata();
	}

	{
		PROFILE_SCOPE("Renderer Draw");
		Prism::Renderer2D::BeginScene(m_CameraController.GetCamera());

		Prism::Renderer2D::DrawQuad({ -0.3f, -0.3f }, { 1.0f, 1.0f }, m_SquareColor);

		Prism::Renderer2D::DrawQuad({ 0.5f, 0.5f }, { 0.5f, 0.5f }, m_Texture);
		Prism::Renderer2D::EndScene();
	}
	/*glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f));
	m_Shader->GetOriginalShader()->Bind();
	m_Shader->GetOriginalShader()->SetFloat3("_MainColor", m_SquareColor);
	Prism::Renderer::Submit(m_Shader->GetOriginalShader(), m_SquareVA, scale);*/

}

void Sandbox2D::OnEvent(Prism::Event& event)
{
	Layer::OnEvent(event);
	Prism::EventDispatcher dispatcher(event);
	m_CameraController.OnEvent(event);
}

void Sandbox2D::OnImGuiRender()
{
	Layer::OnImGuiRender();
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));

	for (auto& result : m_ProfileResult)
	{
		char label[50];
		strcpy(label, "%.3fms ");
		strcat(label, result.Name);
		ImGui::Text(label, result.Time);
	}
	m_ProfileResult.clear();

	ImGui::End();
}
