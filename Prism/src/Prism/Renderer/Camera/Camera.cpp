#include "prpch.h"
#include "Camera.h"

#include "Prism/Core/Input.h"
#include "Prism/Events/MouseEvent.h"

#include <glfw/glfw3.h>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#define M_PI 3.14159f

namespace Prism {

	Camera::Camera(const glm::mat4& projectionMatrix)
		: m_ProjectionMatrix(projectionMatrix)
	{
	}

	Camera::Camera() = default;
	Camera::~Camera() = default;

}