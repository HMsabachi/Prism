#pragma once

#include "Prism/Renderer/Buffer/VertexBuffer.h"
#include "Prism/Core/Buffer.h"

#include "Platform/Vulkan/Vulkan.h"
#include "VulkanMemoryAllocator/vk_mem_alloc.h"

namespace Prism
{

    class PRISM_API VulkanVertexBuffer : public VertexBuffer
    {
    public:
        VulkanVertexBuffer(void* data, uint32_t size, BufferUsage usage = BufferUsage::Static);
        VulkanVertexBuffer(uint32_t size, BufferUsage usage = BufferUsage::Dynamic);
        virtual ~VulkanVertexBuffer();

        virtual void SetData(void* data, uint32_t size, uint32_t offset = 0) override;

        virtual const VertexBufferLayout& GetLayout() const override { return m_Layout; }
        virtual void SetLayout(const VertexBufferLayout& layout) override { m_Layout = layout; }

        virtual uint32_t GetSize() const override { return m_Size; }

        void RT_Create();

        VkBuffer GetVulkanBuffer() const { return m_VulkanBuffer; }
    private:
        uint32_t m_Size = 0;
        BufferUsage m_Usage;
        VertexBufferLayout m_Layout;

        Buffer m_LocalData;

        VkBuffer m_VulkanBuffer = VK_NULL_HANDLE;
        VmaAllocation m_Allocation = VK_NULL_HANDLE;
    };

}
