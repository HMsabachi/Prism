#include "prpch.h"
#include "RenderCommandQueue.h"

#define PR_RENDER_TRACE(...) PR_CORE_TRACE(__VA_ARGS__)
namespace Prism
{
    constexpr static size_t AlignUp(size_t size, size_t alignment) {
        return (size + alignment - 1) & ~(alignment - 1);
    }

    RenderCommandQueue::RenderCommandQueue()
    {
        // m_CommandBuffer = new uint8_t[COMMAND_BUFFER_SIZE]; 
        m_CommandBufferPtr = m_CommandBuffer;
        memset(m_CommandBuffer, 0, COMMAND_BUFFER_SIZE);

        // m_DataPool = new uint8_t[DATA_POOL_MAX_SIZE];
        m_DataPoolPtr = m_DataPool;
        memset(m_DataPool, 0, DATA_POOL_MAX_SIZE);
    }

    RenderCommandQueue::~RenderCommandQueue()
    {
        // TODO: 渲染队列释放内存顺序有问题
        //delete[] m_CommandBuffer;
    }

    void* RenderCommandQueue::Allocate(RenderCommandFn fn, uint32_t size)
    {
        m_SubmitCount++;
        //if (m_IsExecuting);
            //PR_CORE_WARN("RenderCommandQueue: 在执行命令队列时分配新了的命令!");
        // TODO: 对齐 alignment
        *(RenderCommandFn*)m_CommandBufferPtr = fn;
        m_CommandBufferPtr += sizeof(RenderCommandFn);

        *(uint32_t*)m_CommandBufferPtr = size;
        m_CommandBufferPtr += sizeof(uint32_t);

        void* memory = m_CommandBufferPtr;
        m_CommandBufferPtr += size;

        m_CommandCount++;
        return memory;
    }

    void* RenderCommandQueue::DataAllocate(const void* data, size_t size)
    {
        if (size > DATA_POOL_MAX_SIZE) return nullptr;
        size_t alignedSize = AlignUp(size, 16);
        if (m_DataPoolPtr + alignedSize > m_DataPool + DATA_POOL_MAX_SIZE)
            m_DataPoolPtr = m_DataPool;
        byte* ptr = m_DataPoolPtr;
        m_DataPoolPtr += alignedSize;
        if (data)
            std::memcpy(ptr, data, size);
        m_DataPoolCapacity = (uint32_t)((m_DataPoolPtr - m_DataPool) / (1024 * 1024));
        return ptr;
    }

    void RenderCommandQueue::Execute()
    {
        PR_PROFILE_FUNCTION();
        //PR_RENDER_TRACE("RendererCommandQueue begin");
        m_IsExecuting = true;

        //PR_RENDER_TRACE("RenderCommandQueue::Execute -- {0} commands, {1} bytes", m_CommandCount, (m_CommandBufferPtr - m_CommandBuffer));
        byte* buffer = m_CommandBuffer;

        for (uint32_t i = 0; i < m_CommandCount; i++)
        {
            RenderCommandFn function = *(RenderCommandFn*)buffer;
            buffer += sizeof(RenderCommandFn);

            uint32_t size = *(uint32_t*)buffer;
            buffer += sizeof(uint32_t);
            function(buffer);
            buffer += size;
        }

        m_CommandBufferPtr = m_CommandBuffer;
        m_CommandCount = 0;
        m_IsExecuting = false;
        //PR_RENDER_TRACE("RendererCommandQueue end");

    }
    

}
