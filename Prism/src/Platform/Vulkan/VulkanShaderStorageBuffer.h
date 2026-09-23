#pragma once

#include "Prism/Renderer/Buffer/ShaderStorageBuffer.h"
#include "Platform/Vulkan/Vulkan.h"
#include "VulkanMemoryAllocator/vk_mem_alloc.h"

#include <atomic>

namespace Prism
{

    class VulkanShaderStorageBufferReadback : public ShaderStorageBufferReadback
    {
    public:
        VulkanShaderStorageBufferReadback(size_t size);
        virtual ~VulkanShaderStorageBufferReadback();

        virtual bool IsDone() const override;
        virtual size_t GetSize() const override { return m_Size; }
        virtual void GetData(void* data) const override;

        void RT_Create();
        VkBuffer RT_GetStagingBuffer() const { return m_Staging; }
        void RT_MarkDone();

    private:
        void Release();

    private:
        size_t m_Size = 0;
        VkBuffer m_Staging = VK_NULL_HANDLE;
        VmaAllocation m_Allocation = VK_NULL_HANDLE;
        uint8_t* m_Mapped = nullptr;
        std::atomic<bool> m_Done{ false };
    };

    class VulkanShaderStorageBuffer : public ShaderStorageBuffer
    {
    public:
        VulkanShaderStorageBuffer(size_t size, BufferUsage usage);
        virtual ~VulkanShaderStorageBuffer();

        virtual void SetData(const void* data, size_t size, size_t offset = 0) override;
        virtual Ref<ShaderStorageBufferReadback> RequestReadback(size_t offset = 0, size_t size = 0) override;

        virtual size_t GetSize() const override { return m_Size; }

        void RT_SetData(const void* data, size_t size, size_t offset = 0);

        void RT_Create();

        void RT_RequestReadback(Ref<VulkanShaderStorageBufferReadback> request, size_t offset, size_t size);

        VkDescriptorBufferInfo GetDescriptor() const;

    private:
        void Release();
        uint32_t CurrentSlotIndex() const;

    private:
        BufferUsage m_Usage = BufferUsage::Dynamic;
        size_t m_Size = 0;
        uint32_t m_LastWrittenSlot = 0;
        uint32_t m_LastWriteFrame = 0;

        VkBuffer m_Buffers[VulkanFramesInFlight] = {};
        VmaAllocation m_Allocations[VulkanFramesInFlight] = {};
        uint8_t* m_Mapped[VulkanFramesInFlight] = {};
    };

}
