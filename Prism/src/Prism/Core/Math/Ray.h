#pragma once
#include "AABB.h"

namespace Prism
{
	struct PRISM_API Ray
	{
		glm::vec3 Origin, Direction;
		Ray(const glm::vec3& origin, const glm::vec3& direction);

		static Ray Zero();
		bool IntersectsAABB(const AABB& aabb, float& t) const;
		bool IntersectsTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t) const;
	};
}