#include "prpch.h"
#include "Transform.h"

namespace Prism {

    Transform::Transform()
    {
        RecalculateMatrix();
    }

    Transform::Transform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
        : m_Translation(translation), m_Rotation(rotation), m_Scale(scale)
    {
        RecalculateMatrix();
    }

    const glm::mat4& Transform::GetMatrix()
    {
        if (m_Changed)
            RecalculateMatrix();

        return m_Matrix;
    }

    void Transform::RecalculateMatrix()
    {
        glm::quat rotation = glm::quat(glm::radians(m_Rotation));

        m_Up = glm::rotate(rotation, glm::vec3(0.0F, 1.0F, 0.0F));
        m_Right = glm::rotate(rotation, glm::vec3(1.0F, 0.0F, 0.0F));
        m_Forward = glm::rotate(rotation, glm::vec3(0.0F, 0.0F, -1.0F));

        m_Matrix = glm::translate(glm::mat4(1.0F), m_Translation)
            * glm::toMat4(rotation)
            * glm::scale(glm::mat4(1.0F), m_Scale);

        m_Changed = false;
    }

}
