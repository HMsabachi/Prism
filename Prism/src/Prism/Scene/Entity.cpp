#include "prpch.h"
#include "Entity.h"
// Box2D
#include <box2d/box2d.h>
// PhysX
#include <PhysX/PxPhysicsAPI.h>
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
		if (HasComponent<RigidBodyComponent>())
		{
			auto& rb = GetComponent<RigidBodyComponent>();
			if (rb.RuntimeActor)
			{
				physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
				physx::PxTransform pose = actor->getGlobalPose();
				pose.p = physx::PxVec3(position.x, position.y, position.z);
				actor->setGlobalPose(pose);
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
		if (HasComponent<RigidBodyComponent>())
		{
			auto& rb = GetComponent<RigidBodyComponent>();
			if (rb.RuntimeActor)
			{
				physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
				glm::quat q = glm::quat(glm::radians(rotation));
				actor->setGlobalPose(physx::PxTransform(
					actor->getGlobalPose().p,
					physx::PxQuat(q.x, q.y, q.z, q.w)
				));
			}
		}
		tc.Rotation = glm::quat(glm::radians(rotation));
	}

	void Entity::SetScale(const glm::vec3& scale)
	{
		GetComponent<TransformComponent>().Scale = scale;
	}

}
