#pragma once

#include "Prism/Scene/Entity.h"
#include "Prism/Core/Ref.h"
#include "Prism/Physics/Physics.h"

namespace physx
{
    class PxRigidActor;
}

namespace Prism {

    class PhysicsActor : public RefCounted
    {
    public:
        PhysicsActor(Entity entity);
        ~PhysicsActor();

        glm::vec3 GetPosition();
        glm::quat GetRotation();
        void Rotate(const glm::vec3& rotation);

        float GetMass() const;
        void SetMass(float mass);

        void AddForce(const glm::vec3& force, ForceMode forceMode);
        void AddTorque(const glm::vec3& torque, ForceMode forceMode);

        glm::vec3 GetLinearVelocity() const;
        void SetLinearVelocity(const glm::vec3& velocity);
        glm::vec3 GetAngularVelocity() const;
        void SetAngularVelocity(const glm::vec3& velocity);

        void SetLinearDrag(float drag) const;
        void SetAngularDrag(float drag) const;

        void SetLayer(uint32_t layerId);

        bool IsDynamic() const { return m_RigidBody.BodyType == RigidBodyComponent::Type::Dynamic; }

        Entity& GetEntity() { return m_Entity; }

    private:
        void Initialize();
        void Spawn();
        void SyncToPhysX();
        void SyncFromPhysX();

    private:
        Entity m_Entity;
        RigidBodyComponent& m_RigidBody;
        PhysicsMaterialComponent m_Material;

        physx::PxRigidActor* m_ActorInternal;

        friend class Physics;
        friend class PXPhysicsWrappers;
        friend class TransformSyncSystem;
    };

}
