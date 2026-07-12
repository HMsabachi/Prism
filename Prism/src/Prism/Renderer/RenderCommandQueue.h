#pragma once
#include "prpch.h"

namespace Prism
{
    class PRISM_API RenderCommandQueue
    {
    private:
        static const size_t COMMAND_BUFFER_SIZE;
        static const size_t DATA_POOL_MAX_SIZE; 
    public:
        typedef void(*RenderCommandFn)(void*);

        RenderCommandQueue();
        ~RenderCommandQueue();

        void* Allocate(RenderCommandFn fn, uint32_t size);
        void Execute();

        void* DataAllocate(const void* data, size_t size);

        // Debug
        void ResetSubmitCount() { m_SubmitCount = 0; }
        uint32_t GetSubmitCount() const { return m_SubmitCount; }
        uint32_t GetDataPoolCapacity() const { return m_DataPoolCapacity; }
    private:
        uint8_t* m_CommandBuffer;
        uint8_t* m_CommandBufferPtr;
        uint32_t m_CommandCount;

        uint8_t* m_DataPool;
        uint8_t* m_DataPoolPtr;
        uint32_t m_DataPoolCapacity;

        bool m_IsExecuting = false;
        uint32_t m_SubmitCount;
    };
}
