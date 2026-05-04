#include "prpch.h"
#include "PhysXManager.h"

namespace Prism
{
	void PhysXManager::Init()
	{
		PR_CORE_ASSERT(!s_PXFoundation, "PhysXManager::Init shouldn't be called more than once!");

		s_PXFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_PXAllocator, s_PXErrorCallback);
		PR_CORE_ASSERT(s_PXFoundation, "PxCreateFoundation Failed!");

		s_PXPhysicsFactory = PxCreatePhysics(PX_PHYSICS_VERSION, *s_PXFoundation, physx::PxTolerancesScale(), true);
		PR_CORE_ASSERT(s_PXPhysicsFactory, "PxCreatePhysics Failed!");
	}

	void PhysXManager::Shutdown()
	{
		s_PXPhysicsFactory->release();
		s_PXFoundation->release();
	}

	physx::PxSceneDesc PhysXManager::CreateSceneDesc()
	{
		return physx::PxSceneDesc(s_PXPhysicsFactory->getTolerancesScale());
	}

	physx::PxScene* PhysXManager::CreateScene(const physx::PxSceneDesc& sceneDesc)
	{
		return s_PXPhysicsFactory->createScene(sceneDesc);
	}

	PhysXErrorCallback PhysXManager::s_PXErrorCallback;
	PhysXAllocator PhysXManager::s_PXAllocator;
	physx::PxFoundation* PhysXManager::s_PXFoundation = nullptr;
	physx::PxPhysics* PhysXManager::s_PXPhysicsFactory = nullptr;
}
