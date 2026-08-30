#include "prpch.h"
#include "Ray.h"

namespace Prism
{

	Ray::Ray(const glm::vec3& origin, const glm::vec3& direction)
	{
		Origin = origin;
		Direction = direction;
	}

	Prism::Ray Ray::Zero()
	{
		return { {0.0f, 0.0f, 0.0f},{0.0f, 0.0f, 0.0f} };
	}

	bool Ray::IntersectsAABB(const AABB& aabb, float& t) const
	{
		glm::vec3 dirfrac = 1.0f / Direction;

		const glm::vec3& lb = aabb.Min;
		const glm::vec3& rt = aabb.Max;
		float t1 = (lb.x - Origin.x) * dirfrac.x;
		float t2 = (rt.x - Origin.x) * dirfrac.x;
		float t3 = (lb.y - Origin.y) * dirfrac.y;
		float t4 = (rt.y - Origin.y) * dirfrac.y;
		float t5 = (lb.z - Origin.z) * dirfrac.z;
		float t6 = (rt.z - Origin.z) * dirfrac.z;

		float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
		float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

		if (tmax < 0)
		{
			t = tmax;
			return false;
		}
		if (tmin > tmax)
		{
			t = tmax;
			return false;
		}
		t = tmin;
		return true;
	}

	bool Ray::IntersectsTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t) const
	{
		const glm::vec3 e1 = v1 - v0;
		const glm::vec3 e2 = v2 - v0;
		const glm::vec3 n = glm::cross(e1, e2);
		const float det = -glm::dot(Direction, n);
		const float invdet = 1.0f / det;

		const glm::vec3 ao = Origin - v0;
		const glm::vec3 dao = glm::cross(ao, Direction);
		const float u = glm::dot(e2, dao) * invdet;
		const float v = -glm::dot(e1, dao) * invdet;
		t = glm::dot(ao, n) * invdet;
		return (det >= 1e-6f && t >= 0.0f && u >= 0.0f && v >= 0.0f && u + v <= 1.0f);
	}

}