#pragma once

#include "Prism/Renderer/Buffer/UniformBuffer.h"
#include "Platform/Vulkan/Vulkan.h"
#include "VulkanMemoryAllocator/vk_mem_alloc.h"

namespace Prism
{
    class PRISM_API VulkanUniformBuffer : public UniformBuffer
    {
    public:
        VulkanUniformBuffer(uint32_t size);
        virtual ~VulkanUniformBuffer();

        virtual void SetData(const Buffer& buffer) override;
        virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
        virtual void RT_SetData(const Buffer& buffer) override;
        virtual void RT_SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

        virtual uint32_t GetSize() const override { return m_Size; }

        void RT_Create();

        VkDescriptorBufferInfo GetDescriptor() const;
    private:
        void Release();
        uint32_t CurrentSlotIndex() const;
    private:
        uint32_t m_Size = 0;
        uint32_t m_LastWrittenSlot = 0;
        uint32_t m_LastWriteFrame = 0;

        VkBuffer m_Buffers[VulkanFramesInFlight] = {};
        VmaAllocation m_Allocations[VulkanFramesInFlight] = {};
        uint8_t* m_Mapped[VulkanFramesInFlight] = {};
    };

}
