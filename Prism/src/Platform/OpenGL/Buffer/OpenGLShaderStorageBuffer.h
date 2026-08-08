#pragma once
#include "Prism/Renderer/Buffer/ShaderStorageBuffer.h"

namespace Prism
{
    class OpenGLShaderStorageBuffer : public ShaderStorageBuffer
    {

    public:
        OpenGLShaderStorageBuffer(size_t size, BufferUsage usage);

        virtual ~OpenGLShaderStorageBuffer() override;

        virtual void SetData(const void* data, size_t size, size_t offset = 0) override;
        virtual void GetData(void* data, size_t size, size_t offset = 0, bool sync = false) const override;

        RendererID GetRendererID() const;
        virtual size_t GetSize() const override;

    private:
        RendererID m_RendererID;
        size_t m_Size;
        BufferUsage m_Usage;
    };
}
