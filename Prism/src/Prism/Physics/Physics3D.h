#pragma once
#include "Prism/Core/Core.h"

#include <PhysX/PxPhysicsAPI.h>
#include <functional>

namespace Prism
{
	struct RigidBodyComponent;
	class Mesh;
	class Entity;

	enum class ForceMode : uint16_t
	{
		Force = 0,
		Impulse,
		VelocityChange,
		Acceleration
	};

	enum class FilterGroup : uint32_t
	{
		Static   = BIT(0),
		Dynamic  = BIT(1),
		Kinematic = BIT(2),
		All      = Static | Dynamic | Kinematic
	};

	class PRISM_API Physics3D
	{
	public:
		static void Init();
		static void Shutdown();

		static physx::PxSceneDesc CreateSceneDesc();
		static physx::PxScene* CreateScene(const physx::PxSceneDesc& sceneDesc);
		static physx::PxRigidActor* CreateAndAddActor(physx::PxScene* scene, const RigidBodyComponent& rigidbody, const glm::mat4& transform);
		static physx::PxMaterial* CreateMaterial(float staticFriction, float dynamicFriction, float restitution);

		static physx::PxTransform CreatePose(const glm::mat4& transform);

		static physx::PxPhysics* GetFactory() { return s_PXPhysicsFactory; }

		static void SetCollisionFilters(physx::PxRigidActor* actor, uint32_t filterGroup, uint32_t filterMask);

		static void SetCollisionScene(class Scene* scene);

		using CollisionCallback = std::function<void(Entity)>;
		static void SetCollisionCallbacks(CollisionCallback begin, CollisionCallback end);

		static void ConnectToPhysXDebugger();
		static void DisconnectFromPhysXDebugger();

		static physx::PxConvexMesh* CreateConvexMeshCollider(const Ref<Mesh>& mesh);
		static physx::PxTriangleMesh* CreateTriangleMeshCollider(const Ref<Mesh>& mesh);

	private:
		static physx::PxErrorCallback* s_PXErrorCallback;
		static physx::PxAllocatorCallback* s_PXAllocator;
		static physx::PxFoundation* s_PXFoundation;
		static physx::PxPhysics* s_PXPhysicsFactory;
		static physx::PxPvd* s_PXPvd;
	};
}
