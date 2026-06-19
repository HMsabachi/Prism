#include "prpch.h"
#include "PXPhysicsWrappers.h"
#include "Physics.h"
#include "PhysicsLayer.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Core/Warning.h"

#include <glm/gtx/rotate_vector.hpp>

PR_WARNING_PUSH
PR_WARNING_DISABLE(26495)
#include <PhysX/extensions/PxDefaultCpuDispatcher.h>
#include <PhysX/extensions/PxRigidBodyExt.h>
PR_WARNING_POP

#ifdef PR_DEBUG
#define PHYSX_DEBUGGER 1
#endif

namespace Prism {

    static PhysicsErrorCallback s_ErrorCallback;
    static physx::PxDefaultAllocator s_Allocator;
    static physx::PxFoundation* s_Foundation;
    static physx::PxPhysics* s_Physics;
    static physx::PxOverlapHit s_OverlapBuffer[OVERLAP_MAX_COLLIDERS];

    static physx::PxSimulationFilterShader s_FilterShader = physx::PxDefaultSimulationFilterShader;

    static ContactListener s_ContactListener;

    void PhysicsErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
    {
        const char* errorMessage = NULL;

        switch (code)
        {
        case physx::PxErrorCode::eNO_ERROR:         errorMessage = "No Error"; break;
        case physx::PxErrorCode::eDEBUG_INFO:       errorMessage = "Info"; break;
        case physx::PxErrorCode::eDEBUG_WARNING:    errorMessage = "Warning"; break;
        case physx::PxErrorCode::eINVALID_PARAMETER: errorMessage = "Invalid Parameter"; break;
        case physx::PxErrorCode::eINVALID_OPERATION: errorMessage = "Invalid Operation"; break;
        case physx::PxErrorCode::eOUT_OF_MEMORY:    errorMessage = "Out Of Memory"; break;
        case physx::PxErrorCode::eINTERNAL_ERROR:   errorMessage = "Internal Error"; break;
        case physx::PxErrorCode::eABORT:            errorMessage = "Abort"; break;
        case physx::PxErrorCode::ePERF_WARNING:     errorMessage = "Performance Warning"; break;
        case physx::PxErrorCode::eMASK_ALL:         errorMessage = "Unknown Error"; break;
        }

        switch (code)
        {
        case physx::PxErrorCode::eNO_ERROR:
        case physx::PxErrorCode::eDEBUG_INFO:
            PR_CORE_INFO("[PhysX]: {0}: {1} at {2} ({3})", errorMessage, message, file, line);
            break;
        case physx::PxErrorCode::eDEBUG_WARNING:
        case physx::PxErrorCode::ePERF_WARNING:
            PR_CORE_WARN("[PhysX]: {0}: {1} at {2} ({3})", errorMessage, message, file, line);
            break;
        case physx::PxErrorCode::eINVALID_PARAMETER:
        case physx::PxErrorCode::eINVALID_OPERATION:
        case physx::PxErrorCode::eOUT_OF_MEMORY:
        case physx::PxErrorCode::eINTERNAL_ERROR:
            PR_CORE_ERROR("[PhysX]: {0}: {1} at {2} ({3})", errorMessage, message, file, line);
            break;
        case physx::PxErrorCode::eABORT:
        case physx::PxErrorCode::eMASK_ALL:
            PR_CORE_FATAL("[PhysX]: {0}: {1} at {2} ({3})", errorMessage, message, file, line);
            PR_CORE_ASSERT(false);
            break;
        }
    }

    physx::PxScene* PXPhysicsWrappers::CreateScene(const SceneParams& sceneParams)
    {
        physx::PxSceneDesc sceneDesc(s_Physics->getTolerancesScale());

        sceneDesc.gravity = { 0.0F, Physics::GetGravity(), 0.0F };
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

    void PXPhysicsWrappers::SetCollisionFilters(const physx::PxRigidActor& actor, uint32_t physicsLayer)
    {
        const PhysicsLayer& layerInfo = PhysicsLayerManager::GetLayer(physicsLayer);

        if (layerInfo.CollidesWith == 0)
            return;

        physx::PxFilterData filterData;
        filterData.word0 = layerInfo.BitValue;
        filterData.word1 = layerInfo.CollidesWith;

        const physx::PxU32 numShapes = actor.getNbShapes();

        physx::PxShape** shapes = (physx::PxShape**)s_Allocator.allocate(sizeof(physx::PxShape*) * numShapes, "", "", 0);
        actor.getShapes(shapes, numShapes);

        for (physx::PxU32 i = 0; i < numShapes; i++)
            shapes[i]->setSimulationFilterData(filterData);

        s_Allocator.deallocate(shapes);
    }

    void PXPhysicsWrappers::AddBoxCollider(physx::PxRigidActor& actor, const physx::PxMaterial& material, const BoxColliderComponent& collider, const glm::vec3& size)
    {
        glm::vec3 colliderSize = collider.Size;

        if (size.x != 0.0F) colliderSize.x *= size.x;
        if (size.y != 0.0F) colliderSize.y *= size.y;
        if (size.z != 0.0F) colliderSize.z *= size.z;

        physx::PxBoxGeometry boxGeometry = physx::PxBoxGeometry(colliderSize.x / 2.0F, colliderSize.y / 2.0F, colliderSize.z / 2.0F);
        physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(actor, boxGeometry, material);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);
        shape->setLocalPose(ToPhysXTransform(glm::translate(glm::mat4(1.0F), collider.Offset)));
    }

    void PXPhysicsWrappers::AddSphereCollider(physx::PxRigidActor& actor, const physx::PxMaterial& material, const SphereColliderComponent& collider, const glm::vec3& size)
    {
        float colliderRadius = collider.Radius;

        if (size.x != 0.0F) colliderRadius *= size.x;

        physx::PxSphereGeometry sphereGeometry = physx::PxSphereGeometry(colliderRadius);
        physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(actor, sphereGeometry, material);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);
    }

    void PXPhysicsWrappers::AddCapsuleCollider(physx::PxRigidActor& actor, const physx::PxMaterial& material, const CapsuleColliderComponent& collider, const glm::vec3& size)
    {
        float colliderRadius = collider.Radius;
        float colliderHeight = collider.Height;

        if (size.x != 0.0F)
            colliderRadius *= (size.x / 2.0F);

        if (size.y != 0.0F)
            colliderHeight *= size.y;

        physx::PxCapsuleGeometry capsuleGeometry = physx::PxCapsuleGeometry(colliderRadius, colliderHeight / 2.0F);
        physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(actor, capsuleGeometry, material);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);
        shape->setLocalPose(physx::PxTransform(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1))));
    }

    void PXPhysicsWrappers::AddMeshCollider(physx::PxRigidActor& actor, const physx::PxMaterial& material, MeshColliderComponent& collider, const glm::vec3& size)
    {
        physx::PxConvexMeshGeometry triangleGeometry = physx::PxConvexMeshGeometry(CreateConvexMesh(collider));
        triangleGeometry.meshFlags = physx::PxConvexMeshGeometryFlag::eTIGHT_BOUNDS;
        physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(actor, triangleGeometry, material);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);

        PX_UNUSED(size);
    }

    physx::PxConvexMesh* PXPhysicsWrappers::CreateConvexMesh(MeshColliderComponent& collider)
    {
        // TODO: Save cooked mesh once processed
        std::vector<Vertex> vertices = collider.CollisionMesh->GetStaticVertices();

        physx::PxConvexMeshDesc convexDesc;
        convexDesc.points.count = (uint32_t)vertices.size();
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

        if (!collider.ProcessedMesh)
        {
            // Based On: https://github.com/EpicGames/UnrealEngine/blob/08ee319f80ef47dbf0988e14b546b65214838ec4/Engine/Source/ThirdParty/PhysX3/NvCloth/samples/SampleBase/renderer/ConvexRenderMesh.cpp

            const uint32_t nbPolygons = mesh->getNbPolygons();
            const physx::PxVec3* convexVertices = mesh->getVertices();
            const physx::PxU8* convexIndices = mesh->getIndexBuffer();

            uint32_t nbVertices = 0;
            uint32_t nbFaces = 0;

            for (uint32_t i = 0; i < nbPolygons; i++)
            {
                physx::PxHullPolygon polygon;
                mesh->getPolygonData(i, polygon);
                nbVertices += polygon.mNbVerts;
                nbFaces += (polygon.mNbVerts - 2) * 3;
            }

            std::vector<Vertex> collisionVertices;
            std::vector<Index> collisionIndices;

            collisionVertices.resize(nbVertices);
            collisionIndices.resize(nbFaces / 3);

            uint32_t vertCounter = 0;
            uint32_t indexCounter = 0;
            for (uint32_t i = 0; i < nbPolygons; i++)
            {
                physx::PxHullPolygon polygon;
                mesh->getPolygonData(i, polygon);

                uint32_t vI0 = vertCounter;
                for (uint32_t vI = 0; vI < polygon.mNbVerts; vI++)
                {
                    collisionVertices[vertCounter].Position = glm::rotate(FromPhysXVector(convexVertices[convexIndices[polygon.mIndexBase + vI]]), glm::radians(90.0F), { 1, 0, 0});
                    vertCounter++;
                }

                for (uint32_t vI = 1; vI < uint32_t(polygon.mNbVerts) - 1; vI++)
                {
                    collisionIndices[indexCounter].V1 = uint32_t(vI0);
                    collisionIndices[indexCounter].V2 = uint32_t(vI0 + vI + 1);
                    collisionIndices[indexCounter].V3 = uint32_t(vI0 + vI);
                    indexCounter++;
                }
            }

            collider.ProcessedMesh = Ref<Mesh>::Create(collisionVertices, collisionIndices);
        }

        return mesh;
    }

    bool PXPhysicsWrappers::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit* hit)
    {
        physx::PxScene* scene = static_cast<physx::PxScene*>(Physics::GetPhysicsScene());
        physx::PxRaycastBuffer hitInfo;
        bool result = scene->raycast(ToPhysXVector(origin), ToPhysXVector(glm::normalize(direction)), maxDistance, hitInfo);

        if (result)
        {
            Entity& entity = *(Entity*)hitInfo.block.actor->userData;

            // NOTE: This should never be the case...
            if (!entity)
                PR_CORE_ASSERT(false, "Physics body with no Entity?");

            hit->EntityID = entity.GetUUID();
            hit->Position = FromPhysXVector(hitInfo.block.position);
            hit->Normal = FromPhysXVector(hitInfo.block.normal);
            hit->Distance = hitInfo.block.distance;
        }

        return result;
    }

    bool PXPhysicsWrappers::OverlapBox(const glm::vec3& origin, const glm::vec3& halfSize, std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t* count)
    {
        physx::PxScene* scene = static_cast<physx::PxScene*>(Physics::GetPhysicsScene());

        memset(s_OverlapBuffer, 0, sizeof(s_OverlapBuffer));
        physx::PxOverlapBuffer buf(s_OverlapBuffer, OVERLAP_MAX_COLLIDERS);
        physx::PxBoxGeometry geometry = physx::PxBoxGeometry(halfSize.x, halfSize.y, halfSize.z);
        physx::PxTransform pose = ToPhysXTransform(glm::translate(glm::mat4(1.0F), origin));

        bool result = scene->overlap(geometry, pose, buf);

        if (result)
        {
            uint32_t bodyCount = buf.nbTouches >= OVERLAP_MAX_COLLIDERS ? OVERLAP_MAX_COLLIDERS : buf.nbTouches;
            memcpy(buffer.data(), buf.touches, bodyCount * sizeof(physx::PxOverlapHit));
            *count = bodyCount;
        }

        return result;
    }

    bool PXPhysicsWrappers::OverlapCapsule(const glm::vec3& origin, float radius, float halfHeight, std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t* count)
    {
        physx::PxScene* scene = static_cast<physx::PxScene*>(Physics::GetPhysicsScene());

        memset(s_OverlapBuffer, 0, sizeof(s_OverlapBuffer));
        physx::PxOverlapBuffer buf(s_OverlapBuffer, OVERLAP_MAX_COLLIDERS);
        physx::PxCapsuleGeometry geometry = physx::PxCapsuleGeometry(radius, halfHeight);
        physx::PxTransform pose = ToPhysXTransform(glm::translate(glm::mat4(1.0F), origin));

        bool result = scene->overlap(geometry, pose, buf);

        if (result)
        {
            uint32_t bodyCount = buf.nbTouches >= OVERLAP_MAX_COLLIDERS ? OVERLAP_MAX_COLLIDERS : buf.nbTouches;
            memcpy(buffer.data(), buf.touches, bodyCount * sizeof(physx::PxOverlapHit));
            *count = bodyCount;
        }

        return result;
    }

    bool PXPhysicsWrappers::OverlapSphere(const glm::vec3& origin, float radius, std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t* count)
    {
        physx::PxScene* scene = static_cast<physx::PxScene*>(Physics::GetPhysicsScene());

        memset(s_OverlapBuffer, 0, sizeof(s_OverlapBuffer));
        physx::PxOverlapBuffer buf(s_OverlapBuffer, OVERLAP_MAX_COLLIDERS);
        physx::PxSphereGeometry geometry = physx::PxSphereGeometry(radius);
        physx::PxTransform pose = ToPhysXTransform(glm::translate(glm::mat4(1.0F), origin));

        bool result = scene->overlap(geometry, pose, buf);

        if (result)
        {
            uint32_t bodyCount = buf.nbTouches >= OVERLAP_MAX_COLLIDERS ? OVERLAP_MAX_COLLIDERS : buf.nbTouches;
            memcpy(buffer.data(), buf.touches, bodyCount * sizeof(physx::PxOverlapHit));
            *count = bodyCount;
        }

        return result;
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

        s_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *s_Foundation, physx::PxTolerancesScale(), true);
        PR_CORE_ASSERT(s_Physics, "PxCreatePhysics Failed!");
    }

    void PXPhysicsWrappers::Shutdown()
    {
        s_Physics->release();
        s_Foundation->release();
    }

}
