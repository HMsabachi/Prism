#pragma once

#include "Prism/Renderer/Buffer/ShaderStorageBuffer.h"
#include "Platform/Vulkan/Vulkan.h"
#include "VulkanMemoryAllocator/vk_mem_alloc.h"

namespace Prism
{

    class VulkanShaderStorageBuffer : public ShaderStorageBuffer
    {
    public:
        VulkanShaderStorageBuffer(size_t size, BufferUsage usage);
        virtual ~VulkanShaderStorageBuffer();

        virtual void SetData(const void* data, size_t size, size_t offset = 0) override;
        virtual void GetData(void* data, size_t size, size_t offset = 0, bool sync = false) const override;

        virtual size_t GetSize() const override { return m_Size; }

        void RT_SetData(const void* data, size_t size, size_t offset = 0);

        void RT_Create();

        VkDescriptorBufferInfo GetDescriptor(uint32_t slotIndex) const;

    private:
        void Release();
        uint32_t CurrentSlotIndex() const;

    private:
        size_t m_Size = 0;
        BufferUsage m_Usage = BufferUsage::Dynamic;

        VkBuffer m_Buffers[VulkanFramesInFlight] = {};
        VmaAllocation m_Allocations[VulkanFramesInFlight] = {};
        uint8_t* m_Mapped[VulkanFramesInFlight] = {};
    };

}
