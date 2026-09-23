#include "prpch.h"
#include "Platform/Vulkan/VulkanShaderStorageBuffer.h"

#include "Platform/Vulkan/VulkanAllocator.h"
#include "VulkanRenderer.h"
#include "Prism/Renderer/Renderer.h"
#include "Prism/Core/RenderThread.h"

namespace Prism
{

    VulkanShaderStorageBufferReadback::VulkanShaderStorageBufferReadback(size_t size)
        : m_Size(size)
    {
    }

    VulkanShaderStorageBufferReadback::~VulkanShaderStorageBufferReadback()
    {
        Release();
    }

    void VulkanShaderStorageBufferReadback::Release()
    {
        VkBuffer staging = m_Staging;
        VmaAllocation allocation = m_Allocation;
        m_Staging = VK_NULL_HANDLE;
        m_Allocation = VK_NULL_HANDLE;
        m_Mapped = nullptr;

        if (!staging)
            return;

        Renderer::SubmitResourceFree([staging, allocation]()
        {
            VulkanAllocator::UnmapMemory(allocation);
            VulkanAllocator::DestroyBuffer(staging, allocation);
        });
    }

    void VulkanShaderStorageBufferReadback::RT_Create()
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.size = m_Size;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        m_Allocation = VulkanAllocator::AllocateBuffer(bufferInfo, VMA_MEMORY_USAGE_CPU_ONLY, m_Staging);
        m_Mapped = VulkanAllocator::MapMemory<uint8_t>(m_Allocation);
    }

    bool VulkanShaderStorageBufferReadback::IsDone() const
    {
        return m_Done.load(std::memory_order_acquire);
    }

    void VulkanShaderStorageBufferReadback::RT_MarkDone()
    {
        m_Done.store(true, std::memory_order_release);
    }

    void VulkanShaderStorageBufferReadback::GetData(void* data) const
    {
        PR_CORE_ASSERT(IsDone(), "ShaderStorageBufferReadback::GetData 在 IsDone 之前被调用");
        if (data == nullptr || m_Size == 0 || !m_Mapped)
            return;
        memcpy(data, m_Mapped, m_Size);
    }

    VulkanShaderStorageBuffer::VulkanShaderStorageBuffer(size_t size, BufferUsage usage)
        : m_Size(size), m_Usage(usage)
    {
        if (RenderThread::IsCurrentThreadRT())
        {
            RT_Create();
        }
        else
        {
            Ref<VulkanShaderStorageBuffer> instance = this;
            Renderer::Submit([instance]() mutable
            {
                instance->RT_Create();
            });
        }
    }

    VulkanShaderStorageBuffer::~VulkanShaderStorageBuffer()
    {
        Release();
    }

    void VulkanShaderStorageBuffer::Release()
    {
        VkBuffer buffers[VulkanFramesInFlight];
        VmaAllocation allocations[VulkanFramesInFlight];
        for (uint32_t i = 0; i < VulkanFramesInFlight; i++)
        {
            buffers[i] = m_Buffers[i];
            allocations[i] = m_Allocations[i];
            m_Buffers[i] = VK_NULL_HANDLE;
            m_Allocations[i] = VK_NULL_HANDLE;
            m_Mapped[i] = nullptr;
        }

        Renderer::SubmitResourceFree([buffers, allocations]()
        {
            for (uint32_t i = 0; i < VulkanFramesInFlight; i++)
            {
                if (buffers[i])
                {
                    VulkanAllocator::UnmapMemory(allocations[i]);
                    VulkanAllocator::DestroyBuffer(buffers[i], allocations[i]);
                }
            }
        });
    }

    void VulkanShaderStorageBuffer::RT_Create()
    {
        Release();

        for (uint32_t i = 0; i < VulkanFramesInFlight; i++)
        {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferInfo.size = m_Size;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            m_Allocations[i] = VulkanAllocator::AllocateBuffer(bufferInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, m_Buffers[i]);
            m_Mapped[i] = VulkanAllocator::MapMemory<uint8_t>(m_Allocations[i]);
        }
    }

    uint32_t VulkanShaderStorageBuffer::CurrentSlotIndex() const
    {
        return Renderer::RT_GetCurrentFrameIndex() % VulkanFramesInFlight;
    }

    void VulkanShaderStorageBuffer::SetData(const void* data, size_t size, size_t offset)
    {
        const void* copy = Renderer::DataAllocate(data, size);
        Ref<VulkanShaderStorageBuffer> instance = this;
        Renderer::Submit([instance, copy, size, offset]() mutable
        {
            instance->RT_SetData(copy, size, offset);
        });
    }

    void VulkanShaderStorageBuffer::RT_SetData(const void* data, size_t size, size_t offset)
    {
        uint32_t frame = Renderer::RT_GetCurrentFrameIndex();
        if (frame != m_LastWriteFrame)
        {
            uint32_t cur = frame % VulkanFramesInFlight;
            m_LastWrittenSlot = (cur != m_LastWrittenSlot) ? cur : (m_LastWrittenSlot + 1) % VulkanFramesInFlight;
            m_LastWriteFrame = frame;
        }
        memcpy(m_Mapped[m_LastWrittenSlot] + offset, data, size);
    }

    Ref<ShaderStorageBufferReadback> VulkanShaderStorageBuffer::RequestReadback(size_t offset, size_t size)
    {
        if (offset >= m_Size)
        {
            PR_CORE_ERROR("ShaderStorageBuffer::RequestReadback: 起始偏移 {} 超出 buffer 大小 {}", offset, m_Size);
            return nullptr;
        }

        if (size == 0)
        {
            size = m_Size - offset;
        }
        else if (size > m_Size - offset)
        {
            PR_CORE_ERROR("ShaderStorageBuffer::RequestReadback: 读回范围 [{}, {}) 超出 buffer 大小 {}",
                offset, offset + size, m_Size);
            return nullptr;
        }

        Ref<VulkanShaderStorageBufferReadback> request = Ref<VulkanShaderStorageBufferReadback>::Create(size);
        Ref<VulkanShaderStorageBuffer> instance = this;
        Renderer::Submit([instance, request, offset, size]() mutable
        {
            instance->RT_RequestReadback(request, offset, size);
        });
        return request;
    }

    void VulkanShaderStorageBuffer::RT_RequestReadback(Ref<VulkanShaderStorageBufferReadback> request, size_t offset, size_t size)
    {
        request->RT_Create();

        VkBuffer source = m_Buffers[m_LastWrittenSlot];
        VkBuffer staging = request->RT_GetStagingBuffer();
        VkCommandBuffer cmdBuf = VulkanRenderer::RT_GetActiveCommandBuffer();

        VkBufferMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.buffer = source;
        barrier.offset = offset;
        barrier.size = size;
        vkCmdPipelineBarrier(cmdBuf,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 1, &barrier, 0, nullptr);

        VkBufferCopy region{};
        region.srcOffset = offset;
        region.dstOffset = 0;
        region.size = size;
        vkCmdCopyBuffer(cmdBuf, source, staging, 1, &region);

        Renderer::SubmitResourceFree([request]() mutable
        {
            request->RT_MarkDone();
        });
    }

    VkDescriptorBufferInfo VulkanShaderStorageBuffer::GetDescriptor() const
    {
        VkDescriptorBufferInfo info{};
        info.buffer = m_Buffers[m_LastWrittenSlot];
        info.offset = 0;
        info.range = m_Size;
        return info;
    }

}
