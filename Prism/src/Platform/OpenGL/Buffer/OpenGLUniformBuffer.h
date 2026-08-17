#pragma once
#include "Prism/Renderer/Buffer/UniformBuffer.h"

#include <Glad/glad.h>

namespace Prism
{

    class PRISM_API OpenGLUniformBuffer : public UniformBuffer
    {
    public:
        OpenGLUniformBuffer(uint32_t size);
        virtual ~OpenGLUniformBuffer();

        virtual void SetData(const Buffer& buffer) override;
        virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
        virtual void RT_SetData(const Buffer& buffer) override;
        virtual void RT_SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

        virtual uint32_t GetSize() const override { return m_Size; }
        RendererID GetRendererID() const { return m_RendererID; }

    private:
        RendererID m_RendererID = 0;
        uint32_t m_Size;
    };

}
