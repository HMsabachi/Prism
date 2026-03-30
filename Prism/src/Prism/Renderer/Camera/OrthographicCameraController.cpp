#include "prpch.h"
#include "OrthographicCameraController.h"
#include "Prism/Core/Time.h"
#include "Prism/Input.h"
#include "Prism/KeyCodes.h"


namespace Prism
{
	OrthographicCameraController::OrthographicCameraController(const float aspectRatio, const bool rotation)
		: m_AspectRatio(aspectRatio), m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel)
		, m_Rotation(rotation)
	{

	}
	void OrthographicCameraController::OnUpdata()
	{
		float deltaTime = Time::GetDeltaTime();
		OnUpdate(deltaTime);
	}
	void OrthographicCameraController::OnUpdate(const float deltaTime)
	{
		HandleCameraTransform(deltaTime);
	}
	void OrthographicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(PR_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(PR_BIND_EVENT_FN(OrthographicCameraController::OnWindowResize));
	}
	

#pragma region 事件处理 Event Handler
	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		m_ZoomLevel -= e.GetYOffset() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);;
		return false;
	}
	bool OrthographicCameraController::OnWindowResize(WindowResizeEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}
	void OrthographicCameraController::HandleCameraTransform(const float deltaTime)
	{
		if (Prism::Input::IsKeyPressed(PR_KEY_A))
			m_CameraPosition.x -= m_CameraTranslationSpeed * deltaTime;
		if (Prism::Input::IsKeyPressed(PR_KEY_D))
			m_CameraPosition.x += m_CameraTranslationSpeed * deltaTime;
		if (Prism::Input::IsKeyPressed(PR_KEY_W))
			m_CameraPosition.y += m_CameraTranslationSpeed * deltaTime;
		if (Prism::Input::IsKeyPressed(PR_KEY_S))
			m_CameraPosition.y -= m_CameraTranslationSpeed * deltaTime;
		if (m_Rotation)
		{
			if (Prism::Input::IsKeyPressed(PR_KEY_Q))
				m_CameraRotation += m_CameraRotationSpeed * deltaTime;
			if (Prism::Input::IsKeyPressed(PR_KEY_E))
				m_CameraRotation -= m_CameraRotationSpeed * deltaTime;
			m_Camera.SetRotation(m_CameraRotation);
		}
		
		m_Camera.SetPosition(m_CameraPosition);
		
		m_CameraTranslationSpeed = m_ZoomLevel;
	}
#pragma endregion

	
}