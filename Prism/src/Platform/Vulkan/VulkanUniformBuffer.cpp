#include "prpch.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"

#include "Platform/Vulkan/VulkanAllocator.h"

#include "Prism/Renderer/Renderer.h"
#include "Prism/Core/RenderThread.h"

namespace Prism
{

    VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size)
        : m_Size(size)
    {
        if (RenderThread::IsCurrentThreadRT())
        {
            RT_Create();
        }
        else
        {
            Ref<VulkanUniformBuffer> instance = this;
            Renderer::Submit([instance]() mutable { instance->RT_Create(); });
        }
    }

    VulkanUniformBuffer::~VulkanUniformBuffer()
    {
        Release();
    }

    void VulkanUniformBuffer::Release()
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
                    VulkanAllocator::DestroyBuffer(buffers[i], allocations[i]);
            }
        });
    }

    void VulkanUniformBuffer::RT_Create()
    {
        Release();

        for (uint32_t i = 0; i < VulkanFramesInFlight; i++)
        {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            bufferInfo.size = m_Size;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            m_Allocations[i] = VulkanAllocator::AllocateBuffer(bufferInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, m_Buffers[i]);
            m_Mapped[i] = VulkanAllocator::MapMemory<uint8_t>(m_Allocations[i]);
        }
    }

    uint32_t VulkanUniformBuffer::CurrentSlotIndex() const
    {
        return Renderer::RT_GetCurrentFrameIndex() % VulkanFramesInFlight;
    }

    void VulkanUniformBuffer::SetData(const Buffer& buffer)
    {
        SetData(buffer.Data, (uint32_t)buffer.Size, 0);
    }

    void VulkanUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
    {
        const void* copy = Renderer::DataAllocate(data, size);
        Ref<VulkanUniformBuffer> instance = this;
        Renderer::Submit([instance, copy, size, offset]() mutable
        {
            instance->RT_SetData(copy, size, offset);
        });
    }

    void VulkanUniformBuffer::RT_SetData(const Buffer& buffer)
    {
        RT_SetData(buffer.Data, (uint32_t)buffer.Size, 0);
    }

    void VulkanUniformBuffer::RT_SetData(const void* data, uint32_t size, uint32_t offset)
    {
        uint32_t slot = CurrentSlotIndex();
        memcpy(m_Mapped[slot] + offset, data, size);
    }

    VkDescriptorBufferInfo VulkanUniformBuffer::GetDescriptor(uint32_t slotIndex) const
    {
        PR_CORE_ASSERT(slotIndex < VulkanFramesInFlight, "Invalid slot index for uniform buffer descriptor info!");
        VkDescriptorBufferInfo info{};
        info.buffer = m_Buffers[slotIndex];
        info.offset = 0;
        info.range = m_Size;
        return info;
    }

}
