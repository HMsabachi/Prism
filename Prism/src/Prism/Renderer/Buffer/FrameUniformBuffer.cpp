#include "prpch.h"
#include "FrameUniformBuffer.h"
#include "Prism/ShaderCompiler/PrismBindings.h"

namespace Prism
{
    void FrameUniformBuffer::Init()
    {
        m_Buffer = UniformBuffer::Create(Prism::Config::BINDING_FRAME, sizeof(Data));
    }

    void FrameUniformBuffer::SetViewProjection(const glm::mat4& vp)
    {
        m_Data.ViewProjection = vp;
        m_Data.InverseViewProjection = glm::inverse(vp);
    }

    void FrameUniformBuffer::SetView(const glm::mat4& view)
    {
        m_Data.View = view;
    }

    void FrameUniformBuffer::SetProjection(const glm::mat4& proj)
    {
        m_Data.Projection = proj;
    }

    void FrameUniformBuffer::SetCameraPosition(const glm::vec3& pos)
    {
        m_Data.CameraPosition = pos;
    }

    void FrameUniformBuffer::SetTime(float t, float dt)
    {
        m_Data.Time = glm::vec4(t * 0.2f, t, t * 2, t * 3);
        m_Data.DeltaTime = dt;
    }

    void FrameUniformBuffer::SetLight(uint32_t index, const glm::vec3& dir, const glm::vec3& radiance, float multiplier)
    {
        if (index >= PRISM_MAX_LIGHTS) return;
        m_Data.Lights[index].Direction = dir;
        m_Data.Lights[index].Radiance = radiance;
        m_Data.Lights[index].Multiplier = multiplier;
    }

    void FrameUniformBuffer::SetShadowMatrices(const glm::mat4* matrices, uint32_t count)
    {
        if (count > PRISM_MAX_CASCADES) count = PRISM_MAX_CASCADES;
        for (uint32_t i = 0; i < count; i++)
            m_Data.ShadowMatrices[i] = matrices[i];
    }

    void FrameUniformBuffer::SetCascadeSplits(const glm::vec4& splits)
    {
        m_Data.CascadeSplits = splits;
    }

    void FrameUniformBuffer::SetShadowParams(const glm::vec4& params)
    {
        m_Data.ShadowParams = params;
    }

    void FrameUniformBuffer::SetAspectRatio(float ratio)
    {
        m_Data.AspectRatio = ratio;
    }

    void FrameUniformBuffer::SetResolution(const glm::vec2& res)
    {
        m_Data.Resolution = res;
    }

    void FrameUniformBuffer::Upload()
    {
        m_Buffer->SetData(&m_Data, sizeof(Data));
    }

    void FrameUniformBuffer::Bind() const
    {
        m_Buffer->Bind();
    }
}
