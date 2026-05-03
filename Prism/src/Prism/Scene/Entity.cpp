#include "prpch.h"
#include "Entity.h"
// Box2D
#include <box2d/box2d.h>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/matrix_decompose.hpp"

namespace Prism
{

	void Entity::SetPosition(const glm::vec3& position)
	{
		auto& tc = GetComponent<TransformComponent>();
		if (HasComponent<RigidBody2DComponent>())
		{
			auto& rb2d = GetComponent<RigidBody2DComponent>();
			if (rb2d.RuntimeBody)
			{
				b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
				body->SetTransform(b2Vec2(position.x, position.y), body->GetAngle());
			}
		}
		tc.Position = position;
	}

	void Entity::SetRotation(const glm::vec3& rotation)
	{
		auto& tc = GetComponent<TransformComponent>();
		if (HasComponent<RigidBody2DComponent>())
		{
			auto& rb2d = GetComponent<RigidBody2DComponent>();
			if (rb2d.RuntimeBody)
			{
				b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
				body->SetTransform(body->GetPosition(), rotation.z);
			}
		}
		tc.Rotation = glm::quat(glm::radians(rotation));
	}

	void Entity::SetScale(const glm::vec3& scale)
	{
		GetComponent<TransformComponent>().Scale = scale;
	}

}
