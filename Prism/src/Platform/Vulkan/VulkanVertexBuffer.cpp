#include "prpch.h"
#include "Platform/Vulkan/VulkanVertexBuffer.h"

#include "Platform/Vulkan/VulkanAllocator.h"

#include "Prism/Renderer/Renderer.h"
#include "Prism/Core/RenderThread.h"

namespace Prism
{

    VulkanVertexBuffer::VulkanVertexBuffer(void* data, uint32_t size, BufferUsage usage)
        : m_Size(size), m_Usage(usage)
    {
        m_LocalData = Buffer::Copy(data, size);

        if (RenderThread::IsCurrentThreadRT())
        {
            RT_Create();
        }
        else
        {
            Ref<VulkanVertexBuffer> instance = this;
            Renderer::Submit([instance]() mutable { instance->RT_Create(); });
        }
    }

    VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size, BufferUsage usage)
        : m_Size(size), m_Usage(usage)
    {
        if (RenderThread::IsCurrentThreadRT())
        {
            RT_Create();
        }
        else
        {
            Ref<VulkanVertexBuffer> instance = this;
            Renderer::Submit([instance]() mutable { instance->RT_Create(); });
        }
    }

    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
        VkBuffer buffer = m_VulkanBuffer;
        VmaAllocation allocation = m_Allocation;
        Renderer::SubmitResourceFree([buffer, allocation]()
        {
            if (buffer)
                VulkanAllocator::DestroyBuffer(buffer, allocation);
        });
    }

    void VulkanVertexBuffer::RT_Create()
    {
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = m_Size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        m_Allocation = VulkanAllocator::AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, m_VulkanBuffer);

        if (m_LocalData)
        {
            uint8_t* dst = VulkanAllocator::MapMemory<uint8_t>(m_Allocation);
            memcpy(dst, m_LocalData.Data, m_Size);
            VulkanAllocator::UnmapMemory(m_Allocation);
        }
    }

    void VulkanVertexBuffer::SetData(void* data, uint32_t size, uint32_t offset)
    {
        m_LocalData = Buffer::Copy(data, size);
        m_Size = size;

        Ref<VulkanVertexBuffer> instance = this;
        Renderer::Submit([instance, offset]()
        {
            if (!instance->m_VulkanBuffer)
                return;

            uint8_t* dst = VulkanAllocator::MapMemory<uint8_t>(instance->m_Allocation);
            memcpy(dst + offset, instance->m_LocalData.Data, instance->m_LocalData.Size);
            VulkanAllocator::UnmapMemory(instance->m_Allocation);
        });
    }

}
