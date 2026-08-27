#pragma once
#include "prpch.h"

namespace Prism
{
    class PRISM_API RenderCommandQueue
    {
    private:
        static constexpr size_t COMMAND_BUFFER_SIZE = 10 * 1024 * 1024; // 10MB buffer
        static constexpr size_t DATA_POOL_MAX_SIZE = 20 * 1024 * 1024; // 20MB buffer; 
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
        uint8_t m_CommandBuffer[COMMAND_BUFFER_SIZE];
        uint8_t m_DataPool[DATA_POOL_MAX_SIZE];

        uint8_t* m_CommandBufferPtr = nullptr;
        uint32_t m_CommandCount = 0;

        uint8_t* m_DataPoolPtr = nullptr;
        uint32_t m_DataPoolCapacity = 0;

        bool m_IsExecuting = false;
        uint32_t m_SubmitCount = 0;
    };
}
