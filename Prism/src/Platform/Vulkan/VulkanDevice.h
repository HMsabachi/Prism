#pragma once

#include "Platform/Vulkan/Vulkan.h"
#include "Prism/Core/Ref.h"

#include <string>
#include <vector>
#include <unordered_set>

namespace Prism
{
    class VulkanPhysicalDevice : public RefCounted
    {
    public:
        struct QueueFamilyIndices
        {
            int32_t Graphics = -1;
            int32_t Compute = -1;
            int32_t Transfer = -1;
        };
    public:
        VulkanPhysicalDevice();
        ~VulkanPhysicalDevice();

        bool IsExtensionSupported(const std::string& extensionName) const;
        uint32_t GetExtensionCount() const { return (uint32_t)m_SupportedExtensions.size(); }
        uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) const;

        VkPhysicalDevice GetVulkanPhysicalDevice() const { return m_PhysicalDevice; }
        const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }

        const VkPhysicalDeviceProperties& GetProperties() const { return m_Properties; }
        const VkPhysicalDeviceLimits& GetLimits() const { return m_Properties.limits; }
        const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const { return m_MemoryProperties; }
        const VkPhysicalDeviceFeatures& GetSupportedFeatures() const { return m_Features; }

        VkFormat GetDepthFormat() const { return m_DepthFormat; }

        static Ref<VulkanPhysicalDevice> Select();
    private:
        VkFormat FindDepthFormat() const;
        QueueFamilyIndices GetQueueFamilyIndices(int queueFlags);
    private:
        QueueFamilyIndices m_QueueFamilyIndices;

        VkPhysicalDevice m_PhysicalDevice = nullptr;
        VkPhysicalDeviceProperties m_Properties;
        VkPhysicalDeviceFeatures m_Features;
        VkPhysicalDeviceMemoryProperties m_MemoryProperties;

        VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;

        std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
        std::unordered_set<std::string> m_SupportedExtensions;
        std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos;

        friend class VulkanDevice;
    };

    class VulkanDevice : public RefCounted
    {
    public:
        VulkanDevice(const Ref<VulkanPhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures enabledFeatures);
        ~VulkanDevice();

        void Destroy();

        VkQueue GetQueue() { return m_Queue; }
        VkQueue GetComputeQueue() { return m_ComputeQueue; }

        VkCommandBuffer GetCommandBuffer(bool begin, bool compute = false);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, bool compute = false);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool compute = false);

        VkCommandBuffer CreateSecondaryCommandBuffer();

        const Ref<VulkanPhysicalDevice>& GetPhysicalDevice() const { return m_PhysicalDevice; }
        VkDevice GetVulkanDevice() const { return m_LogicalDevice; }
    private:
        VkDevice m_LogicalDevice = nullptr;
        Ref<VulkanPhysicalDevice> m_PhysicalDevice;
        VkPhysicalDeviceFeatures m_EnabledFeatures;
        VkCommandPool m_CommandPool = VK_NULL_HANDLE;
        VkCommandPool m_ComputeCommandPool = VK_NULL_HANDLE;

        VkQueue m_Queue = nullptr;
        VkQueue m_ComputeQueue = nullptr;

        bool m_EnableDebugMarkers = false;
    };
}
