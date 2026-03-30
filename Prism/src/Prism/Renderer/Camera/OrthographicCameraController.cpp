#include "prpch.h"
#include "OrthographicCameraController.h"
#include "Prism/Core/Time.h"
#include "Prism/Core/Input.h"
#include "Prism/Core/KeyCodes.h"


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
		glm::vec3 direction(0.0f);
		if (Prism::Input::IsKeyPressed(PR_KEY_A))
			direction.x -= m_CameraTranslationSpeed * deltaTime;
		if (Prism::Input::IsKeyPressed(PR_KEY_D))
			direction.x += m_CameraTranslationSpeed * deltaTime;
		if (Prism::Input::IsKeyPressed(PR_KEY_W))
			direction.y += m_CameraTranslationSpeed * deltaTime;
		if (Prism::Input::IsKeyPressed(PR_KEY_S))
			direction.y -= m_CameraTranslationSpeed * deltaTime;
		if (m_Rotation)
		{
			if (Prism::Input::IsKeyPressed(PR_KEY_Q))
				m_CameraRotation += m_CameraRotationSpeed * deltaTime;
			if (Prism::Input::IsKeyPressed(PR_KEY_E))
				m_CameraRotation -= m_CameraRotationSpeed * deltaTime;
			m_Camera.SetRotation(m_CameraRotation);
		}
		direction = BaseDirectionToRealDirection(direction, m_CameraRotation);
		m_CameraPosition += direction;
		
		m_Camera.SetPosition(m_CameraPosition);
		
		m_CameraTranslationSpeed = m_ZoomLevel + m_CameraBaseTranslationSpeed;
	}
#pragma endregion

#pragma region 静态方法 Static Methods
	glm::vec3 OrthographicCameraController::BaseDirectionToRealDirection(glm::vec3& direction, float angle)
	{
		double radian = glm::radians(angle);
		glm::vec3 realDirection = glm::vec3(direction.x * cos(radian) - direction.y * sin(radian), direction.x * sin(radian) + direction.y * cos(radian), direction.z);
		return realDirection;
	}

#pragma endregion


	
}