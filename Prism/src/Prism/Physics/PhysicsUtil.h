#pragma once

#include <PhysX/PxPhysicsAPI.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <functional>

namespace Prism {

    class Entity;
    class Scene;

    void SetContactCallbacks(const std::function<void(Entity)>& onBegin, const std::function<void(Entity)>& onEnd);

    physx::PxTransform ToPhysXTransform(const glm::mat4& matrix);
    physx::PxMat44 ToPhysXMatrix(const glm::mat4& matrix);
    physx::PxVec3 ToPhysXVector(const glm::vec3& vector);
    physx::PxVec4 ToPhysXVector(const glm::vec4& vector);
    physx::PxQuat ToPhysXQuat(const glm::quat& quat);

    glm::mat4 FromPhysXTransform(const physx::PxTransform& transform);
    glm::mat4 FromPhysXMatrix(const physx::PxMat44& matrix);
    glm::vec3 FromPhysXVector(const physx::PxVec3& vector);
    glm::vec4 FromPhysXVector(const physx::PxVec4& vector);
    glm::quat FromPhysXQuat(const physx::PxQuat& quat);

    physx::PxFilterFlags PrismFilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1,
        physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize);

    class ContactListener : public physx::PxSimulationEventCallback
    {
    public:
        virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override;
        virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override;
        virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override;
        virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
        virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
        virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override;
    };

}
