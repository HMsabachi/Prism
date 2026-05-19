#include "prpch.h"
#include "Physics3D.h"
#include "Prism/Scene/Components.h"
#include "Prism/Renderer/Mesh.h"
#include "Scripting/ScriptEngine.h"

#include <PhysX/PxPhysicsAPI.h>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <PhysX/extensions/PxDefaultCpuDispatcher.h>
#include <PhysX/extensions/PxSimpleFactory.h>
#include <PhysX/extensions/PxRigidBodyExt.h>
#include <PhysX/pvd/PxPvdTransport.h>
#include <PhysX/cooking/PxCooking.h>

#define PHYSX_DEBUGGER 1

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

	static PhysXAllocator s_PhysicsAllocator;
	static PhysXErrorCallback s_PhysicsErrorCallback;

	static std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition(const glm::mat4& transform)
	{
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose(transform, scale, orientation, translation, skew, perspective);

		return { translation, orientation, scale };
	}

	class PhysXContactListener : public physx::PxSimulationEventCallback
	{
	public:
		void onConstraintBreak(physx::PxConstraintInfo*, physx::PxU32) override {}
		void onWake(physx::PxActor**, physx::PxU32) override {}
		void onSleep(physx::PxActor**, physx::PxU32) override {}
		void onTrigger(physx::PxTriggerPair*, physx::PxU32) override {}
		void onAdvance(const physx::PxRigidBody* const*, const physx::PxTransform*, const physx::PxU32) override {}

		void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override
		{
			for (physx::PxU32 i = 0; i < nbPairs; i++)
			{
				if (pairs[i].flags & physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)
				{
					Entity* a = (Entity*)(pairHeader.actors[0]->userData);
					Entity* b = (Entity*)(pairHeader.actors[1]->userData);

					if (a && a->HasComponent<ScriptComponent>() && ScriptEngine::ModuleExists(a->GetComponent<ScriptComponent>().ModuleName))
						ScriptEngine::OnCollisionBegin(*a);

					if (b && b->HasComponent<ScriptComponent>() && ScriptEngine::ModuleExists(b->GetComponent<ScriptComponent>().ModuleName))
						ScriptEngine::OnCollisionBegin(*b);
				}

				if (pairs[i].flags & physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)
				{
					Entity* a = (Entity*)(pairHeader.actors[0]->userData);
					Entity* b = (Entity*)(pairHeader.actors[1]->userData);

					if (a && a->HasComponent<ScriptComponent>() && ScriptEngine::ModuleExists(a->GetComponent<ScriptComponent>().ModuleName))
						ScriptEngine::OnCollisionEnd(*a);

					if (b && b->HasComponent<ScriptComponent>() && ScriptEngine::ModuleExists(b->GetComponent<ScriptComponent>().ModuleName))
						ScriptEngine::OnCollisionEnd(*b);
				}
			}
		}
	};

	static PhysXContactListener s_PhysXContactListener;

	static physx::PxFilterFlags HazelFilterShader(
		physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
		physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
	{
		if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
			return physx::PxFilterFlag::eDEFAULT;
		}

		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

		if ((filterData0.word0 & filterData1.word1) || (filterData1.word0 & filterData0.word1))
		{
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
		}

		return physx::PxFilterFlag::eDEFAULT;
	}

	void Physics3D::Init()
	{
		PR_CORE_ASSERT(!s_PXFoundation, "Physics3D::Init shouldn't be called more than once!");

			s_PXAllocator = &s_PhysicsAllocator;
			s_PXErrorCallback = &s_PhysicsErrorCallback;

			s_PXFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *s_PXAllocator, *s_PXErrorCallback);
		PR_CORE_ASSERT(s_PXFoundation, "PxCreateFoundation Failed!");

#if PHYSX_DEBUGGER
		s_PXPvd = PxCreatePvd(*s_PXFoundation);
		ConnectToPhysXDebugger();
#endif

		s_PXPhysicsFactory = PxCreatePhysics(PX_PHYSICS_VERSION, *s_PXFoundation, physx::PxTolerancesScale(), true, s_PXPvd);
		PR_CORE_ASSERT(s_PXPhysicsFactory, "PxCreatePhysics Failed!");
	}

	void Physics3D::Shutdown()
	{
		DisconnectFromPhysXDebugger();
		s_PXPhysicsFactory->release();
		s_PXFoundation->release();
	}

	void Physics3D::ConnectToPhysXDebugger()
	{
#if PHYSX_DEBUGGER
		if (!s_PXPvd)
			return;
		physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("localhost", 5425, 10);
		s_PXPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
#endif
	}

	void Physics3D::DisconnectFromPhysXDebugger()
	{
#if PHYSX_DEBUGGER
		if (s_PXPvd && s_PXPvd->isConnected(false))
			s_PXPvd->disconnect();
#endif
	}

	physx::PxSceneDesc Physics3D::CreateSceneDesc()
	{
		physx::PxSceneDesc sceneDesc(s_PXPhysicsFactory->getTolerancesScale());

		physx::PxDefaultCpuDispatcher* cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
		if (!cpuDispatcher)
			PR_CORE_ASSERT(false, "Failed to create PhysX CPU dispatcher!");
		sceneDesc.cpuDispatcher = cpuDispatcher;

		sceneDesc.filterShader = HazelFilterShader;
		sceneDesc.simulationEventCallback = &s_PhysXContactListener;

		return sceneDesc;
	}

	physx::PxScene* Physics3D::CreateScene(const physx::PxSceneDesc& sceneDesc)
	{
		return s_PXPhysicsFactory->createScene(sceneDesc);
	}

	physx::PxRigidActor* Physics3D::CreateAndAddActor(physx::PxScene* scene, const RigidBodyComponent& rigidbody, const glm::mat4& transform)
	{
		physx::PxRigidActor* actor = nullptr;

		if (rigidbody.BodyType == RigidBodyComponent::Type::Static)
		{
			actor = s_PXPhysicsFactory->createRigidStatic(CreatePose(transform));
		}
		else if (rigidbody.BodyType == RigidBodyComponent::Type::Dynamic)
		{
			physx::PxRigidDynamic* dynamicActor = s_PXPhysicsFactory->createRigidDynamic(CreatePose(transform));

			dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, rigidbody.IsKinematic);
			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, rigidbody.LockPositionX);
			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, rigidbody.LockPositionY);
			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, rigidbody.LockPositionZ);
			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, rigidbody.LockRotationX);
			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, rigidbody.LockRotationY);
			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, rigidbody.LockRotationZ);

			physx::PxRigidBodyExt::updateMassAndInertia(*dynamicActor, rigidbody.Mass);

			actor = dynamicActor;
		}

		scene->addActor(*actor);

		return actor;
	}

	physx::PxMaterial* Physics3D::CreateMaterial(float staticFriction, float dynamicFriction, float restitution)
	{
		return s_PXPhysicsFactory->createMaterial(staticFriction, dynamicFriction, restitution);
	}

	physx::PxTransform Physics3D::CreatePose(const glm::mat4& transform)
	{
		auto [translation, rotationQuat, scale] = GetTransformDecomposition(transform);
		glm::vec3 rotation = glm::eulerAngles(rotationQuat);

		physx::PxTransform physxTransform(physx::PxVec3(translation.x, translation.y, translation.z));
		physxTransform.rotate(physx::PxVec3(rotation.x, rotation.y, rotation.z));
		return physxTransform;
	}

	void Physics3D::SetCollisionFilters(physx::PxRigidActor* actor, uint32_t filterGroup, uint32_t filterMask)
	{
		physx::PxFilterData filterData;
		filterData.word0 = filterGroup;
		filterData.word1 = filterMask;

		const physx::PxU32 numShapes = actor->getNbShapes();
		physx::PxShape** shapes = (physx::PxShape**)s_PXAllocator->allocate(sizeof(physx::PxShape*) * numShapes, "", "", 0);
		actor->getShapes(shapes, numShapes);
		for (physx::PxU32 i = 0; i < numShapes; i++)
		{
			physx::PxShape* shape = shapes[i];
			shape->setSimulationFilterData(filterData);
		}
		s_PXAllocator->deallocate(shapes);
	}

	physx::PxConvexMesh* Physics3D::CreateConvexMeshCollider(const Ref<Mesh>& mesh)
	{
		const auto& vertices = mesh->GetStaticVertices();
		if (vertices.empty())
		{
			PR_CORE_ERROR("Physics3D::CreateConvexMeshCollider: mesh has no vertices!");
			return nullptr;
		}

		physx::PxConvexMeshDesc convexDesc;
		convexDesc.points.count = (physx::PxU32)vertices.size();
		convexDesc.points.stride = sizeof(Vertex);
		convexDesc.points.data = vertices.data();
		convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

		physx::PxConvexMeshCookingResult::Enum result;
		physx::PxConvexMesh* convexMesh = PxCreateConvexMesh(
			physx::PxCookingParams(physx::PxTolerancesScale()),
			convexDesc,
			*PxGetStandaloneInsertionCallback(),
			&result
		);

		if (!convexMesh)
		{
			PR_CORE_ERROR("Physics3D::CreateConvexMeshCollider failed! Cooking result: {0}", (int)result);
			return nullptr;
		}

		return convexMesh;
	}

	physx::PxTriangleMesh* Physics3D::CreateTriangleMeshCollider(const Ref<Mesh>& mesh)
	{
		const auto& vertices = mesh->GetStaticVertices();
		const auto& indices = mesh->GetIndices();

		if (vertices.empty() || indices.empty())
		{
			PR_CORE_ERROR("Physics3D::CreateTriangleMeshCollider: mesh has no vertices or indices!");
			return nullptr;
		}

		physx::PxTriangleMeshDesc meshDesc;
		meshDesc.points.count = (physx::PxU32)vertices.size();
		meshDesc.points.stride = sizeof(Vertex);
		meshDesc.points.data = vertices.data();
		meshDesc.triangles.count = (physx::PxU32)indices.size();
		meshDesc.triangles.stride = sizeof(Index);
		meshDesc.triangles.data = indices.data();

		physx::PxTriangleMeshCookingResult::Enum result;
		physx::PxTriangleMesh* triangleMesh = PxCreateTriangleMesh(
			physx::PxCookingParams(physx::PxTolerancesScale()),
			meshDesc,
			*PxGetStandaloneInsertionCallback(),
			&result
		);

		if (!triangleMesh)
		{
			PR_CORE_ERROR("Physics3D::CreateTriangleMeshCollider failed! Cooking result: {0}", (int)result);
			return nullptr;
		}

		return triangleMesh;
	}

	physx::PxErrorCallback* Physics3D::s_PXErrorCallback = nullptr;
	physx::PxAllocatorCallback* Physics3D::s_PXAllocator = nullptr;
	physx::PxFoundation* Physics3D::s_PXFoundation = nullptr;
	physx::PxPhysics* Physics3D::s_PXPhysicsFactory = nullptr;
	physx::PxPvd* Physics3D::s_PXPvd = nullptr;
}
