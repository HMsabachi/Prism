#pragma once
#include "Prism/Core/Core.h"
#include <PhysX/PxPhysicsAPI.h>

namespace Prism
{
	struct PhysXAllocator : public physx::PxAllocatorCallback
	{
		void* allocate(size_t size, const char*, const char*, int) override
		{
			return ::malloc(size);
		}

		void deallocate(void* ptr) override
		{
			::free(ptr);
		}
	};

	struct PhysXErrorCallback : public physx::PxErrorCallback
	{
		void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override
		{
			PR_CORE_ERROR("PhysX: {0} ({1}:{2})", message, file, line);
		}
	};

	class PhysXManager
	{
	public:
		static void Init();
		static void Shutdown();

		static physx::PxSceneDesc CreateSceneDesc();
		static physx::PxScene* CreateScene(const physx::PxSceneDesc& sceneDesc);

	private:
		static PhysXErrorCallback s_PXErrorCallback;
		static PhysXAllocator s_PXAllocator;
		static physx::PxFoundation* s_PXFoundation;
		static physx::PxPhysics* s_PXPhysicsFactory;
	};
}
