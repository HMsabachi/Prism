#pragma once
#include "Prism/Renderer/Buffer/IndexBuffer.h"
#include "Prism/Core/Buffer.h"

namespace Prism
{

    class PRISM_API OpenGLIndexBuffer : public IndexBuffer
    {
    public:
        OpenGLIndexBuffer(uint32_t size);
        OpenGLIndexBuffer(void* data, uint32_t size);
        virtual ~OpenGLIndexBuffer();

        virtual void SetData(void* data, uint32_t size, uint32_t offset = 0);
        virtual void Bind() const;

        virtual uint32_t GetSize() const { return m_Size; }
        virtual uint32_t GetCount() const { return m_Size / sizeof(uint32_t); }

        void RT_Bind() const;
        RendererID GetRendererID() const { return m_RendererID; }
    private:
        RendererID m_RendererID = 0;
        uint32_t m_Size;

        Buffer m_LocalData;
    };

}
