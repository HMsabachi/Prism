#pragma once

#include "Prism/Renderer/Buffer/UniformBuffer.h"
#include <glm/glm.hpp>

namespace Prism
{
    static constexpr uint32_t PRISM_MAX_LIGHTS = 1;
    static constexpr uint32_t PRISM_MAX_CASCADES = 4;

    class PRISM_API FrameUniformBuffer
    {
    public:
        FrameUniformBuffer() = default;
        ~FrameUniformBuffer() = default;

        void Init();

        void SetViewProjection(const glm::mat4& vp);
        void SetView(const glm::mat4& view);
        void SetProjection(const glm::mat4& proj);
        void SetCameraPosition(const glm::vec3& pos);
        void SetTime(float t, float dt);
        void SetLight(uint32_t index, const glm::vec3& dir, const glm::vec3& radiance, float multiplier);
        void SetShadowMatrices(const glm::mat4* matrices, uint32_t count);
        void SetCascadeSplits(const glm::vec4& splits);
        void SetShadowParams(float bias, float normalBias, float cascadeCount, float softShadows);
        void SetShadowData(float lightSize, float maxShadowDistance, float shadowFade, float cascadeFading);
        void SetAspectRatio(float ratio);
        void SetResolution(const glm::vec2& res);

        void Upload();
        void Bind() const;

    private:
        Ref<UniformBuffer> m_Buffer;

        struct alignas(16) Data
        {
            glm::mat4 ViewProjection{ 1.0f };
            glm::mat4 InverseViewProjection{ 1.0f };
            glm::mat4 View{ 1.0f };
            glm::mat4 Projection{ 1.0f };

            glm::vec4 Time{ 0.0f };

            glm::vec3 CameraPosition{ 0.0f };
            float DeltaTime{ 0.0f }; 

            glm::vec2 Resolution{ 1280.0f, 720.0f };
            float AspectRatio{ 1.0f };
            float pad0{ 0.0f };

            struct Light
            {
                glm::vec3 Direction{};
                float pad1{};
                glm::vec3 Radiance{};
                float Multiplier{};
            } Lights[PRISM_MAX_LIGHTS];

            glm::mat4 ShadowMatrices[PRISM_MAX_CASCADES]{};
            glm::vec4 CascadeSplits{};
            glm::vec4 ShadowParams{};
            glm::vec4 ShadowData{};
        } m_Data;
    };
}
