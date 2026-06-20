#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Prism {

    class PRISM_API Transform
    {
    public:
        Transform();
        Transform(const glm::vec3& translation, const glm::vec3& rotation = glm::vec3(0.0F), const glm::vec3& scale = glm::vec3(1.0F));

        void Translate(const glm::vec3& translation) { m_Translation += translation; m_HasChanged = true; m_PhysicsDirty = true; }
        void Rotate(const glm::vec3& rotation) { m_Rotation += rotation; m_HasChanged = true; m_PhysicsDirty = true; }

        void SetPosition(const glm::vec3& position) { m_Translation = position; m_HasChanged = true; m_PhysicsDirty = true; }
        void SetRotation(const glm::vec3& rotation) { m_Rotation = rotation; m_HasChanged = true; m_PhysicsDirty = true; }
        void SetRotation(const glm::quat& rotation) { m_Rotation = glm::degrees(glm::eulerAngles(rotation)); m_HasChanged = true; m_PhysicsDirty = true; }
        void SetScale(const glm::vec3& scale) { m_Scale = scale; m_HasChanged = true; m_PhysicsDirty = true; }

        const glm::vec3& GetPosition() const { return m_Translation; }
        const glm::vec3& GetRotation() const { return m_Rotation; }
        const glm::vec3& GetScale() const { return m_Scale; }

        const glm::vec3& GetUpDirection() const { return m_Up; }
        const glm::vec3& GetRightDirection() const { return m_Right; }
        const glm::vec3& GetForwardDirection() const { return m_Forward; }

        bool IsChanged() const { return m_HasChanged; }

        bool IsPhysicsDirty() const { return m_PhysicsDirty; }
        void MarkPhysicsClean() { m_PhysicsDirty = false; }

        const glm::mat4& GetMatrix();
        float* Data() { return glm::value_ptr(m_Matrix); }

    private:
        void RecalculateMatrix();

    private:
        glm::vec3 m_Translation = glm::vec3(0.0F);
        glm::vec3 m_Rotation = glm::vec3(0.0F);
        glm::vec3 m_Scale = glm::vec3(1.0F);

        glm::vec3 m_Forward = glm::vec3(0.0F, 0.0F, -1.0F);
        glm::vec3 m_Right = glm::vec3(1.0F, 0.0F, 0.0F);
        glm::vec3 m_Up = glm::vec3(0.0F, 1.0F, 0.0F);

        glm::mat4 m_Matrix = glm::mat4(1.0F);
        bool m_HasChanged = false;
        bool m_PhysicsDirty = false;
    };

}
