#pragma once

#include "Prism/Renderer/Buffer/IndexBuffer.h"
#include "Prism/Core/Buffer.h"

#include "Platform/Vulkan/Vulkan.h"
#include "VulkanMemoryAllocator/vk_mem_alloc.h"

namespace Prism
{

    class PRISM_API VulkanIndexBuffer : public IndexBuffer
    {
    public:
        VulkanIndexBuffer(uint32_t size);
        VulkanIndexBuffer(void* data, uint32_t size);
        virtual ~VulkanIndexBuffer();

        virtual void SetData(void* data, uint32_t size, uint32_t offset = 0) override;
        virtual void Bind() const override;

        virtual uint32_t GetSize() const override { return m_Size; }
        virtual uint32_t GetCount() const override { return m_Size / sizeof(uint32_t); }

        void RT_Create();

        VkBuffer GetVulkanBuffer() const { return m_VulkanBuffer; }
    private:
        uint32_t m_Size = 0;

        Buffer m_LocalData;

        VkBuffer m_VulkanBuffer = VK_NULL_HANDLE;
        VmaAllocation m_Allocation = VK_NULL_HANDLE;
    };

}
