#pragma once

#include "ISystem.h"
#include <glm/glm.hpp>

namespace Prism {

    class Scene;
    class Entity;

    struct DecomposedTransform
    {
        glm::vec3 Position;
        glm::vec3 Rotation;   // euler degrees
        glm::vec3 Scale;
    };

    class PRISM_API TransformSystem : public ISystem
    {
    public:
        explicit TransformSystem(Scene* scene);

        DecomposedTransform GetWorldDecomposed(Entity entity);
        glm::mat4 GetWorldTransformMatrix(Entity entity);

        glm::vec3 GetWorldPosition(Entity entity);
        glm::vec3 GetWorldRotation(Entity entity);
        glm::vec3 GetWorldScale(Entity entity);

        void SetWorldPosition(Entity entity, const glm::vec3& position);
        void SetWorldRotation(Entity entity, const glm::vec3& rotation);
        void SetWorldScale(Entity entity, const glm::vec3& scale);

        glm::vec3 GetLocalPosition(Entity entity);
        glm::vec3 GetLocalRotation(Entity entity);
        glm::vec3 GetLocalScale(Entity entity);

        void SetLocalPosition(Entity entity, const glm::vec3& position);
        void SetLocalRotation(Entity entity, const glm::vec3& rotation);
        void SetLocalScale(Entity entity, const glm::vec3& scale);

        void OnFixedUpdate(float dt) override;
        void OnPreUpdate(float dt) override;
        void OnRender(float dt) override;

    private:
        Scene* m_Scene;
    };

}
