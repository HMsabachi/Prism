#include "prpch.h"
#include "Platform/Vulkan/VulkanShaderStorageBuffer.h"

#include "Platform/Vulkan/VulkanAllocator.h"
#include "Prism/Renderer/Renderer.h"
#include "Prism/Core/RenderThread.h"

namespace Prism
{

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
            bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
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

    void VulkanShaderStorageBuffer::GetData(void* data, size_t size, size_t offset, bool sync) const
    {
        // TODO: GPU 回读路径待定
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
