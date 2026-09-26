#pragma once
#include "Prism/Renderer/Buffer/ShaderStorageBuffer.h"

#include <atomic>

namespace Prism
{
    class OpenGLShaderStorageBufferReadback : public ShaderStorageBufferReadback
    {
    public:
        OpenGLShaderStorageBufferReadback(size_t size);

        virtual bool IsDone() const override;
        virtual size_t GetSize() const override;
        virtual void GetData(void* data) const override;

        void RT_ReadFrom(RendererID bufferID, size_t offset);

    private:
        std::vector<uint8_t> m_Staging;
        std::atomic<bool> m_Done{ false };
    };

    class OpenGLShaderStorageBuffer : public ShaderStorageBuffer
    {

    public:
        OpenGLShaderStorageBuffer(size_t size, BufferUsage usage);

        virtual ~OpenGLShaderStorageBuffer() override;

        virtual void SetData(const void* data, size_t size, size_t offset = 0) override;
        virtual Ref<ShaderStorageBufferReadback> RequestReadback(size_t offset = 0, size_t size = 0) override;

        void RT_Init();

        RendererID GetRendererID() const;
        virtual size_t GetSize() const override;

    private:
        RendererID m_RendererID;
        size_t m_Size;
        BufferUsage m_Usage;
    };
}
