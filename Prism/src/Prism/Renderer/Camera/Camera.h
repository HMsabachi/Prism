#pragma once

#include <glm/glm.hpp>
#include "Prism/Core/Core.h"

namespace Prism {

	class PRISM_API Camera
	{
	public:
		Camera();
		Camera(const glm::mat4& projectionMatrix);

		virtual ~Camera();

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		void SetProjectionMatrix(const glm::mat4& projectionMatrix) { m_ProjectionMatrix = projectionMatrix; }

		float GetExposure() const { return m_Exposure; }
		float& GetExposure() { return m_Exposure; }
	protected:
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
		float m_Exposure = 0.8f; // 默认曝光度
	};

}