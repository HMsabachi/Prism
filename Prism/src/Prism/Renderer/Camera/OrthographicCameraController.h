#pragma once
#include "Prism/Core/Core.h"
#include "OrthographicCamera.h"

#include "Prism/Events/Event.h"
#include "Prism/Events/ApplicationEvent.h"
#include "Prism/Events/MouseEvent.h"


#include <glm/glm.hpp>

namespace Prism
{
	class PRISM_API OrthographicCameraController
	{
	public:
		OrthographicCameraController(const float aspectRatio, const bool rotation = false);

		void OnUpdata();
		void OnUpdate(const float deltaTime);
		void OnEvent(Event& e);

		OrthographicCamera& GetCamera() { return m_Camera; }
	private:
		void HandleCameraTransform(const float deltaTime);

		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		OrthographicCamera m_Camera;

		bool m_Rotation;
		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f;
		float m_CameraTranslationSpeed = 5.0f, m_CameraRotationSpeed = 180.0f;
	};
}