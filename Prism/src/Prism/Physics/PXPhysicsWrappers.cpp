#include "prpch.h"
#include "PXPhysicsWrappers.h"
#include "Physics.h"
#include "PhysicsLayer.h"
#include "PhysicsUtil.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Core/Warning.h"

#include <functional>
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
    static physx::PxPvd* s_PVD = nullptr;
    static physx::PxPvdTransport* s_PvdTransport = nullptr;
    static physx::PxPhysics* s_Physics;
    static physx::PxOverlapHit s_OverlapBuffer[OVERLAP_MAX_COLLIDERS];

    static physx::PxSimulationFilterShader s_FilterShader = physx::PxDefaultSimulationFilterShader;

    static ContactListener s_ContactListener;

    static std::function<void(Entity)> s_OnCollisionBegin;
    static std::function<void(Entity)> s_OnCollisionEnd;
    static std::function<void(Entity)> s_OnTriggerBegin;
    static std::function<void(Entity)> s_OnTriggerEnd;

    void SetContactCallbacks(const std::function<void(Entity)>& onBegin, const std::function<void(Entity)>& onEnd)
    {
        s_OnCollisionBegin = onBegin;
        s_OnCollisionEnd = onEnd;
    }

    void SetContactTriggerCallbacks(const std::function<void(Entity)>& onBegin, const std::function<void(Entity)>& onEnd)
    {
        s_OnTriggerBegin = onBegin;
        s_OnTriggerEnd = onEnd;
    }

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

    void ContactListener::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
    {
        PX_UNUSED(constraints);
        PX_UNUSED(count);
    }

    void ContactListener::onWake(physx::PxActor** actors, physx::PxU32 count)
    {
        for (uint32_t i = 0; i < count; i++)
        {
            physx::PxActor& actor = *actors[i];
            Entity& entity = *(Entity*)actor.userData;

            PR_CORE_INFO("PhysX Actor waking up: ID: {0}, Name: {1}", entity.GetUUID(), entity.GetComponent<TagComponent>().Tag);
        }
    }

    void ContactListener::onSleep(physx::PxActor** actors, physx::PxU32 count)
    {
        for (uint32_t i = 0; i < count; i++)
        {
            physx::PxActor& actor = *actors[i];
            Entity& entity = *(Entity*)actor.userData;

            PR_CORE_INFO("PhysX Actor going to sleep: ID: {0}, Name: {1}", entity.GetUUID(), entity.GetComponent<TagComponent>().Tag);
        }
    }

    void ContactListener::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
    {
        Entity& a = *(Entity*)pairHeader.actors[0]->userData;
        Entity& b = *(Entity*)pairHeader.actors[1]->userData;

        if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)
        {
            if (s_OnCollisionBegin) s_OnCollisionBegin(a);
            if (s_OnCollisionBegin) s_OnCollisionBegin(b);
        }
        else if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)
        {
            if (s_OnCollisionEnd) s_OnCollisionEnd(a);
            if (s_OnCollisionEnd) s_OnCollisionEnd(b);
        }
    }

    void ContactListener::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
    {
        Entity& a = *(Entity*)pairs->triggerActor->userData;
        Entity& b = *(Entity*)pairs->otherActor->userData;

        if (pairs->status & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
        {
            if (s_OnTriggerBegin) s_OnTriggerBegin(a);
            if (s_OnTriggerBegin) s_OnTriggerBegin(b);
        }
        else if (pairs->status & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
        {
            if (s_OnTriggerEnd) s_OnTriggerEnd(a);
            if (s_OnTriggerEnd) s_OnTriggerEnd(b);
        }

        PX_UNUSED(count);
    }

    void ContactListener::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)
    {
        PX_UNUSED(bodyBuffer);
        PX_UNUSED(poseBuffer);
        PX_UNUSED(count);
    }

    static physx::PxBroadPhaseType::Enum PrismToPhysXBroadphaseType(BroadphaseType type)
    {
        switch (type)
        {
        case BroadphaseType::SweepAndPrune: return physx::PxBroadPhaseType::eSAP;
        case BroadphaseType::MultiBoxPrune: return physx::PxBroadPhaseType::eMBP;
        case BroadphaseType::AutomaticBoxPrune: return physx::PxBroadPhaseType::eABP;
        }

        return physx::PxBroadPhaseType::eABP;
    }

    physx::PxScene* PXPhysicsWrappers::CreateScene()
    {
        physx::PxSceneDesc sceneDesc(s_Physics->getTolerancesScale());

        const PhysicsSettings& settings = Physics::GetSettings();

        sceneDesc.gravity = ToPhysXVector(settings.Gravity);
        sceneDesc.broadPhaseType = PrismToPhysXBroadphaseType(settings.BroadphaseAlgorithm);
        sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
        sceneDesc.filterShader = PrismFilterShader;
        sceneDesc.simulationEventCallback = &s_ContactListener;

        PR_CORE_ASSERT(sceneDesc.isValid());
        return s_Physics->createScene(sceneDesc);
    }

    physx::PxRigidActor* PXPhysicsWrappers::CreateActor(const RigidBodyComponent& rigidbody, const Transform& transform)
    {
        physx::PxRigidActor* actor = nullptr;

        const PhysicsSettings& settings = Physics::GetSettings();

        if (rigidbody.BodyType == RigidBodyComponent::Type::Static)
        {
            actor = s_Physics->createRigidStatic(ToPhysXTransform(transform));
        }
        else if (rigidbody.BodyType == RigidBodyComponent::Type::Dynamic)
        {
            physx::PxRigidDynamic* dynamicActor = s_Physics->createRigidDynamic(ToPhysXTransform(transform));

            dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, rigidbody.IsKinematic);

            dynamicActor->setLinearDamping(rigidbody.LinearDrag);
            dynamicActor->setAngularDamping(rigidbody.AngularDrag);

            dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, rigidbody.LockPositionX);
            dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, rigidbody.LockPositionY);
            dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, rigidbody.LockPositionZ);
            dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, rigidbody.LockRotationX);
            dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, rigidbody.LockRotationY);
            dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, rigidbody.LockRotationZ);

            dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, rigidbody.DisableGravity);

            dynamicActor->setSolverIterationCounts(settings.SolverIterations, settings.SolverVelocityIterations);

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
        glm::vec3 meshScale = size;
        if (meshScale.x == 0.0F && meshScale.y == 0.0F && meshScale.z == 0.0F)
            meshScale = glm::vec3(1.0F);

        // Triangle meshes cannot be simulation shapes on non-kinematic dynamic actors
        bool forceConvex = false;
        if (!collider.IsConvex && !collider.IsTrigger)
        {
            physx::PxRigidDynamic* dynamic = actor.is<physx::PxRigidDynamic>();
            if (dynamic && !(dynamic->getRigidBodyFlags() & physx::PxRigidBodyFlag::eKINEMATIC))
                forceConvex = true;
        }

        if (collider.IsConvex || forceConvex)
        {
            std::vector<physx::PxConvexMesh*> meshes = CreateConvexMesh(collider);

            for (auto mesh : meshes)
            {
                physx::PxConvexMeshGeometry convexGeometry = physx::PxConvexMeshGeometry(mesh, physx::PxMeshScale(ToPhysXVector(meshScale)));
                convexGeometry.meshFlags = physx::PxConvexMeshGeometryFlag::eTIGHT_BOUNDS;
                physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(actor, convexGeometry, material);
                if (!shape) { PR_CORE_ERROR("Failed to create convex mesh shape"); continue; }
                shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
                shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);
            }
        }
        else
        {
            std::vector<physx::PxTriangleMesh*> meshes = CreateTriangleMesh(collider);

            for (auto mesh : meshes)
            {
                physx::PxTriangleMeshGeometry triGeometry = physx::PxTriangleMeshGeometry(mesh, physx::PxMeshScale(ToPhysXVector(meshScale)));
                physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(actor, triGeometry, material);
                if (!shape) { PR_CORE_ERROR("Failed to create triangle mesh shape"); continue; }
                shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
                shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);
            }
        }
    }

    std::vector<physx::PxTriangleMesh*> PXPhysicsWrappers::CreateTriangleMesh(MeshColliderComponent& collider, bool invalidateOld)
    {
        std::vector<physx::PxTriangleMesh*> meshes;

        if (invalidateOld)
            ConvexMeshSerializer::DeleteIfSerializedAndInvalidated(collider.CollisionMesh->GetFilePath());

        if (!ConvexMeshSerializer::IsSerialized(collider.CollisionMesh->GetFilePath()))
        {
            const std::vector<Vertex>& vertices = collider.CollisionMesh->GetStaticVertices();
            const std::vector<Index>& indices = collider.CollisionMesh->GetIndices();

            std::vector<glm::vec3> vertexPositions;
            for (const auto& vertex : vertices)
                vertexPositions.push_back(vertex.Position);

            for (const auto& submesh : collider.CollisionMesh->GetSubmeshes())
            {
                physx::PxTriangleMeshDesc triDesc;
                triDesc.points.count = submesh.VertexCount;
                triDesc.points.stride = sizeof(glm::vec3);
                triDesc.points.data = &vertexPositions[submesh.BaseVertex];
                triDesc.triangles.count = submesh.IndexCount / 3;
                triDesc.triangles.data = &indices[submesh.BaseIndex / 3];
                triDesc.triangles.stride = sizeof(Index);

                physx::PxDefaultMemoryOutputStream buf;
                physx::PxCookingParams params(s_Physics->getTolerancesScale());
                if (!PxCookTriangleMesh(params, triDesc, buf))
                {
                    PR_CORE_ERROR("Failed to cook triangle mesh: {0}", submesh.MeshName);
                    continue;
                }

                ConvexMeshSerializer::SerializeMesh(collider.CollisionMesh->GetFilePath(), buf, submesh.MeshName);
                physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
                meshes.push_back(s_Physics->createTriangleMesh(input));
            }
        }
        else
        {
            std::vector<physx::PxDefaultMemoryInputData> serializedMeshes = ConvexMeshSerializer::DeserializeMesh(collider.CollisionMesh->GetFilePath());

            for (auto& meshData : serializedMeshes)
            {
                physx::PxTriangleMesh* m = s_Physics->createTriangleMesh(meshData);
                if (m)
                    meshes.push_back(m);
            }

            ConvexMeshSerializer::CleanupDataBuffers();

            if (meshes.empty() && !serializedMeshes.empty())
            {
                PR_CORE_WARN("Cached triangle mesh invalid for {0}, rebuilding", collider.CollisionMesh->GetFilePath());
                ConvexMeshSerializer::DeleteIfSerializedAndInvalidated(collider.CollisionMesh->GetFilePath());
                return CreateTriangleMesh(collider, false);
            }
        }

        if (collider.ProcessedMeshes.size() <= 0)
        {
            for (auto mesh : meshes)
            {
                const uint32_t nbVerts = mesh->getNbVertices();
                const physx::PxVec3* triVertices = mesh->getVertices();
                const uint32_t nbTriangles = mesh->getNbTriangles();
                const physx::PxU16* tris = (const physx::PxU16*)mesh->getTriangles();

                std::vector<Vertex> verts;
                std::vector<Index> inds;

                for (uint32_t v = 0; v < nbVerts; v++)
                {
                    Vertex v1;
                    v1.Position = glm::rotate(FromPhysXVector(triVertices[v]), glm::radians(90.0F), glm::vec3(1, 0, 0));
                    verts.push_back(v1);
                }

                for (uint32_t tri = 0; tri < nbTriangles; tri++)
                {
                    Index index;
                    index.V1 = tris[3 * tri + 0];
                    index.V2 = tris[3 * tri + 1];
                    index.V3 = tris[3 * tri + 2];
                    inds.push_back(index);
                }

                collider.ProcessedMeshes.push_back(Ref<Mesh>::Create(verts, inds));
            }
        }

        return meshes;
    }

    std::vector<physx::PxConvexMesh*> PXPhysicsWrappers::CreateConvexMesh(MeshColliderComponent& collider, bool invalidateOld)
    {
        std::vector<physx::PxConvexMesh*> meshes;

        if (invalidateOld)
            ConvexMeshSerializer::DeleteIfSerializedAndInvalidated(collider.CollisionMesh->GetFilePath());

        if (!ConvexMeshSerializer::IsSerialized(collider.CollisionMesh->GetFilePath()))
        {
            const std::vector<Vertex>& vertices = collider.CollisionMesh->GetStaticVertices();
            const std::vector<Index>& indices = collider.CollisionMesh->GetIndices();
            std::vector<glm::vec3> vertexPositions;
            for (const auto& vertex : vertices)
                vertexPositions.push_back(vertex.Position);

            bool anySubmeshCooked = false;

            for (const auto& submesh : collider.CollisionMesh->GetSubmeshes())
            {
                physx::PxConvexMeshDesc convexDesc;
                convexDesc.points.count = submesh.VertexCount;
                convexDesc.points.stride = sizeof(glm::vec3);
                convexDesc.points.data = &vertexPositions[submesh.BaseVertex];
                convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX | physx::PxConvexFlag::eCHECK_ZERO_AREA_TRIANGLES | physx::PxConvexFlag::eSHIFT_VERTICES;

                physx::PxDefaultMemoryOutputStream buf;
                physx::PxCookingParams params(s_Physics->getTolerancesScale());
                physx::PxConvexMeshCookingResult::Enum result{};
                if (!PxCookConvexMesh(params, convexDesc, buf, &result))
                {
                    PR_CORE_WARN("Failed to cook convex submesh: {0}, will fallback to whole mesh", submesh.MeshName);
                    continue;
                }

                anySubmeshCooked = true;
                ConvexMeshSerializer::SerializeMesh(collider.CollisionMesh->GetFilePath(), buf, submesh.MeshName);
                physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
                meshes.push_back(s_Physics->createConvexMesh(input));
            }

            // Fallback: cook all vertices as a single convex hull
            if (!anySubmeshCooked)
            {
                PR_CORE_INFO("All submeshes failed, cooking whole mesh as single convex hull");

                physx::PxConvexMeshDesc convexDesc;
                convexDesc.points.count = (uint32_t)vertexPositions.size();
                convexDesc.points.stride = sizeof(glm::vec3);
                convexDesc.points.data = vertexPositions.data();
                convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

                physx::PxDefaultMemoryOutputStream buf;
                physx::PxCookingParams params(s_Physics->getTolerancesScale());
                physx::PxConvexMeshCookingResult::Enum result{};
                if (PxCookConvexMesh(params, convexDesc, buf, &result))
                {
                    ConvexMeshSerializer::SerializeMesh(collider.CollisionMesh->GetFilePath(), buf, "whole");
                    physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
                    meshes.push_back(s_Physics->createConvexMesh(input));
                }
                else
                {
                    PR_CORE_ERROR("Failed to cook whole convex mesh for {0}", collider.CollisionMesh->GetFilePath());
                }
            }
        }
        else
        {
            std::vector<physx::PxDefaultMemoryInputData> serializedMeshes = ConvexMeshSerializer::DeserializeMesh(collider.CollisionMesh->GetFilePath());

            for (auto& meshData : serializedMeshes)
            {
                physx::PxConvexMesh* m = s_Physics->createConvexMesh(meshData);
                if (m)
                    meshes.push_back(m);
            }

            ConvexMeshSerializer::CleanupDataBuffers();

            // Cached data was invalid (e.g. triangle mesh stored as convex), rebuild
            if (meshes.empty() && !serializedMeshes.empty())
            {
                PR_CORE_WARN("Cached convex mesh invalid for {0}, rebuilding", collider.CollisionMesh->GetFilePath());
                ConvexMeshSerializer::DeleteIfSerializedAndInvalidated(collider.CollisionMesh->GetFilePath());
                return CreateConvexMesh(collider, false);
            }
        }

        if (collider.ProcessedMeshes.size() <= 0)
        {
            for (auto mesh : meshes)
            {
                const uint32_t nbPolygons = mesh->getNbPolygons();
                const physx::PxVec3* convexVertices = mesh->getVertices();
                const physx::PxU8* convexIndices = mesh->getIndexBuffer();

                uint32_t vertCounter = 0;

                std::vector<Vertex> collisionVertices;
                std::vector<Index> collisionIndices;

                for (uint32_t i = 0; i < nbPolygons; i++)
                {
                    physx::PxHullPolygon polygon;
                    mesh->getPolygonData(i, polygon);

                    uint32_t vI0 = vertCounter;

                    for (uint32_t vI = 0; vI < polygon.mNbVerts; vI++)
                    {
                        Vertex v;
                        v.Position = glm::rotate(FromPhysXVector(convexVertices[convexIndices[polygon.mIndexBase + vI]]), glm::radians(90.0F), glm::vec3(1, 0, 0));
                        collisionVertices.push_back(v);
                        vertCounter++;
                    }

                    for (uint32_t vI = 1; vI < uint32_t(polygon.mNbVerts) - 1; vI++)
                    {
                        Index index;
                        index.V1 = uint32_t(vI0);
                        index.V2 = uint32_t(vI0 + vI + 1);
                        index.V3 = uint32_t(vI0 + vI);
                        collisionIndices.push_back(index);
                    }
                }

                collider.ProcessedMeshes.push_back(Ref<Mesh>::Create(collisionVertices, collisionIndices));
            }
        }

        return meshes;
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
            PR_CORE_ASSERT(entity, "Physics body with no Entity?");

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

        s_PVD = PxCreatePvd(*s_Foundation);
        if (s_PVD)
        {
            s_PvdTransport = physx::PxDefaultPvdSocketTransportCreate("localhost", 5425, 10);
            s_PVD->connect(*s_PvdTransport, physx::PxPvdInstrumentationFlag::eALL);
        }

        physx::PxTolerancesScale scale = physx::PxTolerancesScale();
        scale.length = 10;
        s_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *s_Foundation, scale, true, s_PVD);
        PR_CORE_ASSERT(s_Physics, "PxCreatePhysics Failed!");
    }

    void PXPhysicsWrappers::Shutdown()
    {
        if (s_PVD)
        {
            s_PVD->disconnect();
            s_PVD = nullptr;
        }
        s_PvdTransport = nullptr;
        s_Physics->release();
        s_Physics = nullptr;
        s_Foundation->release();
        s_Foundation = nullptr;
    }

}
