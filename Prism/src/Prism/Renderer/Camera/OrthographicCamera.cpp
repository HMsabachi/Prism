#include "prpch.h"
#include "OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>
namespace Prism
{
	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		: m_ProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f))
	{
		RecalculateViewMatrix();
	}
	bool OrthographicCamera::OnWindowResize(float width, float height)
	{
		m_AspectRatio = width / height;
		RecalculateProjectionMatrix();
		return true;
	}
	void OrthographicCamera::RecalculateViewMatrix()
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) * 
			glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));
		m_ViewMatrix = glm::inverse(transform);

		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
	void OrthographicCamera::RecalculateProjectionMatrix()
	{
		m_ProjectionMatrix = glm::ortho(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel, -1.0f, 1.0f);

		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
}