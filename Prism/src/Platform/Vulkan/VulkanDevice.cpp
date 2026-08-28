#include "prpch.h"
#include "Platform/Vulkan/VulkanDevice.h"

#include "Platform/Vulkan/VulkanContext.h"

namespace Prism
{
#pragma region Vulkan Physical Device
    VulkanPhysicalDevice::VulkanPhysicalDevice()
    {
        auto vkInstance = VulkanContext::GetInstance();

        uint32_t gpuCount = 0;
        vkEnumeratePhysicalDevices(vkInstance, &gpuCount, nullptr);
        PR_CORE_ASSERT(gpuCount > 0);
        std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
        VK_CHECK_RESULT(vkEnumeratePhysicalDevices(vkInstance, &gpuCount, physicalDevices.data()));

        VkPhysicalDevice selectedPhysicalDevice = nullptr;
        for (VkPhysicalDevice physicalDevice : physicalDevices)
        {
            vkGetPhysicalDeviceProperties(physicalDevice, &m_Properties);
            if (m_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                selectedPhysicalDevice = physicalDevice;
                break;
            }
        }

        if (!selectedPhysicalDevice)
        {
            PR_CORE_WARN("未找到独立显卡 Could not find discrete GPU.");
            selectedPhysicalDevice = physicalDevices.back();
        }

        PR_CORE_ASSERT(selectedPhysicalDevice, "Could not find any physical devices!");
        m_PhysicalDevice = selectedPhysicalDevice;

        vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_Features);
        vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_MemoryProperties);

        PR_CORE_INFO("物理设备 Physical Device: {0}", m_Properties.deviceName);

        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
        PR_CORE_ASSERT(queueFamilyCount > 0);
        m_QueueFamilyProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, m_QueueFamilyProperties.data());

        uint32_t extCount = 0;
        vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extCount, nullptr);
        if (extCount > 0)
        {
            std::vector<VkExtensionProperties> extensions(extCount);
            if (vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extCount, extensions.data()) == VK_SUCCESS)
            {
                PR_CORE_TRACE("Selected physical device has {0} extensions", extensions.size());
                for (const auto& ext : extensions)
                {
                    m_SupportedExtensions.emplace(ext.extensionName);
                    PR_CORE_TRACE("  {0}", ext.extensionName);
                }
            }
        }

        static const float defaultQueuePriority(0.0f);

        int requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
        m_QueueFamilyIndices = GetQueueFamilyIndices(requestedQueueTypes);

        if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
        {
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = m_QueueFamilyIndices.Graphics;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &defaultQueuePriority;
            m_QueueCreateInfos.push_back(queueInfo);
        }

        if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
        {
            if (m_QueueFamilyIndices.Compute != m_QueueFamilyIndices.Graphics)
            {
                VkDeviceQueueCreateInfo queueInfo{};
                queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueInfo.queueFamilyIndex = m_QueueFamilyIndices.Compute;
                queueInfo.queueCount = 1;
                queueInfo.pQueuePriorities = &defaultQueuePriority;
                m_QueueCreateInfos.push_back(queueInfo);
            }
        }

        if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
        {
            if ((m_QueueFamilyIndices.Transfer != m_QueueFamilyIndices.Graphics) && (m_QueueFamilyIndices.Transfer != m_QueueFamilyIndices.Compute))
            {
                VkDeviceQueueCreateInfo queueInfo{};
                queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueInfo.queueFamilyIndex = m_QueueFamilyIndices.Transfer;
                queueInfo.queueCount = 1;
                queueInfo.pQueuePriorities = &defaultQueuePriority;
                m_QueueCreateInfos.push_back(queueInfo);
            }
        }

        m_DepthFormat = FindDepthFormat();
        PR_CORE_ASSERT(m_DepthFormat);
    }

    VulkanPhysicalDevice::~VulkanPhysicalDevice()
    {
    }

    VkFormat VulkanPhysicalDevice::FindDepthFormat() const
    {
        std::vector<VkFormat> depthFormats = {
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM
        };

        for (auto& format : depthFormats)
        {
            VkFormatProperties formatProps;
            vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &formatProps);
            if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                return format;
        }
        return VK_FORMAT_UNDEFINED;
    }

    bool VulkanPhysicalDevice::IsExtensionSupported(const std::string& extensionName) const
    {
        return m_SupportedExtensions.find(extensionName) != m_SupportedExtensions.end();
    }

    VulkanPhysicalDevice::QueueFamilyIndices VulkanPhysicalDevice::GetQueueFamilyIndices(int flags)
    {
        QueueFamilyIndices indices;

        if (flags & VK_QUEUE_COMPUTE_BIT)
        {
            for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++)
            {
                auto& queueFamilyProperties = m_QueueFamilyProperties[i];
                if ((queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) && ((queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
                {
                    indices.Compute = i;
                    break;
                }
            }
        }

        if (flags & VK_QUEUE_TRANSFER_BIT)
        {
            for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++)
            {
                auto& queueFamilyProperties = m_QueueFamilyProperties[i];
                if ((queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT) && ((queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
                {
                    indices.Transfer = i;
                    break;
                }
            }
        }

        for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++)
        {
            if ((flags & VK_QUEUE_TRANSFER_BIT) && indices.Transfer == -1)
            {
                if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
                    indices.Transfer = i;
            }

            if ((flags & VK_QUEUE_COMPUTE_BIT) && indices.Compute == -1)
            {
                if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
                    indices.Compute = i;
            }

            if (flags & VK_QUEUE_GRAPHICS_BIT)
            {
                if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    indices.Graphics = i;
            }
        }

        return indices;
    }

    uint32_t VulkanPhysicalDevice::GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) const
    {
        for (uint32_t i = 0; i < m_MemoryProperties.memoryTypeCount; i++)
        {
            if ((typeBits & 1) == 1)
            {
                if ((m_MemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
                    return i;
            }
            typeBits >>= 1;
        }

        PR_CORE_ASSERT(false, "Could not find a suitable memory type!");
        return UINT32_MAX;
    }

    Ref<VulkanPhysicalDevice> VulkanPhysicalDevice::Select()
    {
        return Ref<VulkanPhysicalDevice>::Create();
    }
#pragma endregion

#pragma region Vulkan Device
    VulkanDevice::VulkanDevice(const Ref<VulkanPhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures enabledFeatures)
        : m_PhysicalDevice(physicalDevice), m_EnabledFeatures(enabledFeatures)
    {
        std::vector<const char*> deviceExtensions;
        PR_CORE_ASSERT(m_PhysicalDevice->IsExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME));
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        if (m_PhysicalDevice->IsExtensionSupported(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME))
            deviceExtensions.push_back(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME);

        if (m_PhysicalDevice->IsExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
        {
            deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
            m_EnableDebugMarkers = true;
        }

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(physicalDevice->m_QueueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = physicalDevice->m_QueueCreateInfos.data();
        deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

        if (deviceExtensions.size() > 0)
        {
            deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
            deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        }

        VkResult result = vkCreateDevice(m_PhysicalDevice->GetVulkanPhysicalDevice(), &deviceCreateInfo, nullptr, &m_LogicalDevice);
        PR_CORE_ASSERT(result == VK_SUCCESS);

        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = m_PhysicalDevice->m_QueueFamilyIndices.Graphics;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VK_CHECK_RESULT(vkCreateCommandPool(m_LogicalDevice, &cmdPoolInfo, nullptr, &m_CommandPool));

        cmdPoolInfo.queueFamilyIndex = m_PhysicalDevice->m_QueueFamilyIndices.Compute;
        VK_CHECK_RESULT(vkCreateCommandPool(m_LogicalDevice, &cmdPoolInfo, nullptr, &m_ComputeCommandPool));

        vkGetDeviceQueue(m_LogicalDevice, m_PhysicalDevice->m_QueueFamilyIndices.Graphics, 0, &m_Queue);
        vkGetDeviceQueue(m_LogicalDevice, m_PhysicalDevice->m_QueueFamilyIndices.Compute, 0, &m_ComputeQueue);
    }

    VulkanDevice::~VulkanDevice()
    {
    }

    void VulkanDevice::Destroy()
    {
        vkDestroyCommandPool(m_LogicalDevice, m_CommandPool, nullptr);
        vkDestroyCommandPool(m_LogicalDevice, m_ComputeCommandPool, nullptr);

        vkDeviceWaitIdle(m_LogicalDevice);
        vkDestroyDevice(m_LogicalDevice, nullptr);
    }

    VkCommandBuffer VulkanDevice::GetCommandBuffer(bool begin, bool compute)
    {
        VkCommandBuffer cmdBuffer;

        VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
        cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufAllocateInfo.commandPool = compute ? m_ComputeCommandPool : m_CommandPool;
        cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBufAllocateInfo.commandBufferCount = 1;

        VK_CHECK_RESULT(vkAllocateCommandBuffers(m_LogicalDevice, &cmdBufAllocateInfo, &cmdBuffer));

        if (begin)
        {
            VkCommandBufferBeginInfo cmdBufferBeginInfo{};
            cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo));
        }

        return cmdBuffer;
    }

    void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, bool compute)
    {
        FlushCommandBuffer(commandBuffer, compute ? m_ComputeQueue : m_Queue, compute);
    }

    void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool compute)
    {
        const uint64_t DEFAULT_FENCE_TIMEOUT = 100000000000;

        PR_CORE_ASSERT(commandBuffer != VK_NULL_HANDLE);

        VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = 0;
        VkFence fence;
        VK_CHECK_RESULT(vkCreateFence(m_LogicalDevice, &fenceCreateInfo, nullptr, &fence));

        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
        VK_CHECK_RESULT(vkWaitForFences(m_LogicalDevice, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

        vkDestroyFence(m_LogicalDevice, fence, nullptr);
        vkFreeCommandBuffers(m_LogicalDevice, compute ? m_ComputeCommandPool : m_CommandPool, 1, &commandBuffer);
    }

    VkCommandBuffer VulkanDevice::CreateSecondaryCommandBuffer()
    {
        VkCommandBuffer cmdBuffer;

        VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
        cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufAllocateInfo.commandPool = m_CommandPool;
        cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        cmdBufAllocateInfo.commandBufferCount = 1;

        VK_CHECK_RESULT(vkAllocateCommandBuffers(m_LogicalDevice, &cmdBufAllocateInfo, &cmdBuffer));
        return cmdBuffer;
    }
#pragma endregion
}
