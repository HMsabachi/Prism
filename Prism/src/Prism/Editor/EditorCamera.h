#pragma once
#include "Prism/Renderer/Camera/Camera.h"


namespace Prism
{
    class Event;
    class MouseScrolledEvent;
}

namespace Prism
{
    class PRISM_API EditorCamera : public Camera
    {
    public:
        EditorCamera() = default;
        EditorCamera(const glm::mat4& projectionMatrix);

        void Focus();
        void OnUpdate(float ts);
        void OnEvent(Event& e);

        inline float GetDistance() const { return m_Distance; }
        inline void SetDistance(float distance) { m_Distance = distance; }

        inline void SetViewportSize(uint32_t width, uint32_t height) { m_ViewportWidth = width > 0 ? width : 1; m_ViewportHeight = height > 0 ? height : 1; }

        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        glm::mat4 GetViewProjection() const { return m_ProjectionMatrix * m_ViewMatrix; }

        glm::vec3 GetUpDirection();
        glm::vec3 GetRightDirection();
        glm::vec3 GetForwardDirection();
        const glm::vec3& GetPosition() const { return m_Position; }
        glm::quat GetOrientation() const;

        float GetPitch() const { return m_Pitch; }
        float GetYaw() const { return m_Yaw; }
    private:
        void UpdateCameraView();

        bool OnMouseScroll(MouseScrolledEvent& e);

        void MousePan(const glm::vec2& delta);
        void MouseRotate(const glm::vec2& delta);
        void MouseZoom(float delta);

        glm::vec3 CalculatePosition();

        std::pair<float, float> PanSpeed() const;
        float RotationSpeed() const;
        float ZoomSpeed() const;
    private:
        glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
        glm::vec3 m_Position = glm::vec3(0.0f), m_Rotation = glm::vec3(0.0f), m_FocalPoint = glm::vec3(0.0f);

        bool m_Panning = false, m_Rotating = false;
        glm::vec2 m_InitialMousePosition = glm::vec2(0.0f);
        glm::vec3 m_InitialFocalPoint = glm::vec3(0.0f), m_InitialRotation = glm::vec3(0.0f);

        float m_Distance = 10.0f;
        float m_Pitch = 0.0f, m_Yaw = 0.0f;

        uint32_t m_ViewportWidth = 1280, m_ViewportHeight = 720;
    };
}
