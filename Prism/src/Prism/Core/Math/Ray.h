#pragma once
#include "AABB.h"

namespace Prism
{
	struct PRISM_API Ray
	{
		glm::vec3 Origin, Direction;

		bool IntersectsAABB(const AABB& aabb, float& t) const;
		bool IntersectsTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t) const;
	};
}