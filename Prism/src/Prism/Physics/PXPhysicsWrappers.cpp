#include "prpch.h"
#include "PXPhysicsWrappers.h"
#include "Physics.h"
#include "Prism/Renderer/Mesh.h"

#include <PhysX/extensions/PxDefaultCpuDispatcher.h>
#include <PhysX/extensions/PxRigidBodyExt.h>
#include <PhysX/pvd/PxPvdTransport.h>

#ifdef PR_DEBUG
#define PHYSX_DEBUGGER 1
#endif

namespace Prism {

    static physx::PxDefaultErrorCallback s_ErrorCallback;
    static physx::PxDefaultAllocator s_Allocator;
    static physx::PxFoundation* s_Foundation;
    static physx::PxPhysics* s_Physics;
    static physx::PxPvd* s_VisualDebugger;

    static physx::PxSimulationFilterShader s_FilterShader = physx::PxDefaultSimulationFilterShader;

    static ContactListener s_ContactListener;

    physx::PxScene* PXPhysicsWrappers::CreateScene(const SceneParams& sceneParams)
    {
        physx::PxSceneDesc sceneDesc(s_Physics->getTolerancesScale());

        sceneDesc.gravity = ToPhysXVector(sceneParams.Gravity);
        sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
        sceneDesc.filterShader = PrismFilterShader;
        sceneDesc.simulationEventCallback = &s_ContactListener;

        PR_CORE_ASSERT(sceneDesc.isValid());
        return s_Physics->createScene(sceneDesc);
    }

    physx::PxRigidActor* PXPhysicsWrappers::CreateActor(const RigidBodyComponent& rigidbody, const glm::mat4& transform)
    {
        physx::PxRigidActor* actor = nullptr;

        if (rigidbody.BodyType == RigidBodyComponent::Type::Static)
        {
            actor = s_Physics->createRigidStatic(ToPhysXTransform(transform));
        }
        else if (rigidbody.BodyType == RigidBodyComponent::Type::Dynamic)
        {
            physx::PxRigidDynamic* dynamicActor = s_Physics->createRigidDynamic(ToPhysXTransform(transform));

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

        return actor;
    }

    void PXPhysicsWrappers::SetCollisionFilters(const physx::PxRigidActor& actor, uint32_t actorGroup, uint32_t filters)
    {
        physx::PxFilterData filterData;
        filterData.word0 = actorGroup;
        filterData.word1 = filters;

        const physx::PxU32 numShapes = actor.getNbShapes();

        physx::PxShape** shapes = (physx::PxShape**)s_Allocator.allocate(sizeof(physx::PxShape*) * numShapes, "", "", 0);
        actor.getShapes(shapes, numShapes);

        for (physx::PxU32 i = 0; i < numShapes; i++)
            shapes[i]->setSimulationFilterData(filterData);

        s_Allocator.deallocate(shapes);
    }

    void PXPhysicsWrappers::AddBoxCollider(physx::PxRigidActor& actor, const physx::PxMaterial& material, const BoxColliderComponent& collider)
    {
        physx::PxBoxGeometry boxGeometry = physx::PxBoxGeometry(collider.Size.x / 2.0F, collider.Size.y / 2.0F, collider.Size.z / 2.0F);
        physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(actor, boxGeometry, material);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);
        shape->setLocalPose(ToPhysXTransform(glm::translate(glm::mat4(1.0F), collider.Offset)));
    }

    void PXPhysicsWrappers::AddSphereCollider(physx::PxRigidActor& actor, const physx::PxMaterial& material, const SphereColliderComponent& collider)
    {
        physx::PxSphereGeometry sphereGeometry = physx::PxSphereGeometry(collider.Radius);
        physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(actor, sphereGeometry, material);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);
    }

    void PXPhysicsWrappers::AddCapsuleCollider(physx::PxRigidActor& actor, const physx::PxMaterial& material, const CapsuleColliderComponent& collider)
    {
        physx::PxCapsuleGeometry capsuleGeometry = physx::PxCapsuleGeometry(collider.Radius, collider.Height / 2.0F);
        physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(actor, capsuleGeometry, material);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);
        shape->setLocalPose(physx::PxTransform(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1))));
    }

    void PXPhysicsWrappers::AddMeshCollider(physx::PxRigidActor& actor, const physx::PxMaterial& material, const MeshColliderComponent& collider)
    {
        // TODO: Possibly take a look at https://github.com/kmammou/v-hacd for computing convex meshes from triangle meshes...
        const auto& vertices = collider.CollisionMesh->GetStaticVertices();
        const auto& indices = collider.CollisionMesh->GetIndices();

        physx::PxConvexMeshDesc convexDesc;
        convexDesc.points.count = vertices.size();
        convexDesc.points.stride = sizeof(Vertex);
        convexDesc.points.data = vertices.data();
        convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

        physx::PxConvexMeshCookingResult::Enum result;
        physx::PxConvexMesh* mesh = PxCreateConvexMesh(
            physx::PxCookingParams(s_Physics->getTolerancesScale()),
            convexDesc,
            *PxGetStandaloneInsertionCallback(),
            &result
        );

        if (!mesh)
            PR_CORE_ASSERT(false);

        physx::PxConvexMeshGeometry triangleGeometry = physx::PxConvexMeshGeometry(mesh);
        triangleGeometry.meshFlags = physx::PxConvexMeshGeometryFlag::eTIGHT_BOUNDS;
        physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(actor, triangleGeometry, material);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);
    }

    physx::PxMaterial* PXPhysicsWrappers::CreateMaterial(const PhysicsMaterialComponent& material)
    {
        return s_Physics->createMaterial(material.StaticFriction, material.DynamicFriction, material.Bounciness);
    }

    void PXPhysicsWrappers::Initialize()
    {
        PR_CORE_ASSERT(!s_Foundation, "PXPhysicsWrappers::Initializer shouldn't be called more than once!");

        s_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_Allocator, s_ErrorCallback);
        PR_CORE_ASSERT(s_Foundation, "PxCreateFoundation Failed!");

        #if PHYSX_DEBUGGER
        s_VisualDebugger = PxCreatePvd(*s_Foundation);
        ConnectVisualDebugger();
        #endif

        s_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *s_Foundation, physx::PxTolerancesScale(), true, s_VisualDebugger);
        PR_CORE_ASSERT(s_Physics, "PxCreatePhysics Failed!");
    }

    void PXPhysicsWrappers::Shutdown()
    {
        s_Physics->release();
        s_Foundation->release();
    }

    void PXPhysicsWrappers::ConnectVisualDebugger()
    {
        #if PHYSX_DEBUGGER
        physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("localhost", 5425, 10);
        s_VisualDebugger->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
        #endif
    }

    void PXPhysicsWrappers::DisconnectVisualDebugger()
    {
        #if PHYSX_DEBUGGER
        if (s_VisualDebugger->isConnected(false))
            s_VisualDebugger->disconnect();
        #endif
    }

}
