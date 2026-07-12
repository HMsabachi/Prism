#pragma once

#include "Prism/Renderer/Buffer/UniformBuffer.h"
#include <glm/glm.hpp>

namespace Prism
{
    static constexpr uint32_t PRISM_MAX_BONES = 128;

    class PRISM_API ObjectUniformBuffer
    {
    public:
        ObjectUniformBuffer() = default;
        ~ObjectUniformBuffer() = default;

        void Init();

        void SetModel(const glm::mat4& model);
        void SetBones(const glm::mat4* bones, uint32_t count);
        void SetShadowPassIndex(int index) { m_Data.Reserved.x = static_cast<float>(index); }
        void Upload();
        void Bind() const;

        RendererID GetID() const { return m_Buffer->GetRendererID(); }

    private:
        Ref<UniformBuffer> m_Buffer;
        bool m_BonesDirty = false;
        struct alignas(16) Data
        {
            glm::mat4 Model{ 1.0f };
            glm::mat4 PrevModel{ 1.0f };
            glm::vec4 Reserved{ 0.0f };
            glm::mat4 Bones[PRISM_MAX_BONES]{};
        } m_Data;
    };
}
