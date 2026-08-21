#pragma once

#include "Platform/Vulkan/Vulkan.h"
#include "Platform/Vulkan/VulkanDevice.h"

#include <vector>

struct GLFWwindow;

namespace Prism
{
    class VulkanSwapChain
    {
    public:
        VulkanSwapChain() = default;

        void Init(VkInstance instance, const Ref<VulkanDevice>& device);
        void InitSurface(GLFWwindow* windowHandle);
        void Create(uint32_t* width, uint32_t* height, bool vsync = false);

        void OnResize(uint32_t width, uint32_t height, bool isVSync = false);

        void BeginFrame();
        void Present();

        uint32_t GetImageCount() const { return m_ImageCount; }

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }

        VkRenderPass GetRenderPass() { return m_RenderPass; }

        VkFramebuffer GetCurrentFramebuffer() { return GetFramebuffer(m_CurrentImageIndex); }
        VkCommandBuffer GetCurrentDrawCommandBuffer() { return GetDrawCommandBuffer(m_CurrentFrameIndex); }

        VkFormat GetColorFormat() { return m_ColorFormat; }

        uint32_t GetCurrentImageIndex() const { return m_CurrentImageIndex; }
        uint32_t GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }

        VkFramebuffer GetFramebuffer(uint32_t index)
        {
            PR_CORE_ASSERT(index < m_ImageCount);
            return m_Framebuffers[index];
        }
        VkCommandBuffer GetDrawCommandBuffer(uint32_t index)
        {
            PR_CORE_ASSERT(index < m_CommandBuffers.size());
            return m_CommandBuffers[index].CommandBuffer;
        }

        void Cleanup();
    private:
        uint32_t AcquireNextImage();

        void CreateDrawBuffers();
        void FindImageFormatAndColorSpace();
    private:
        VkInstance m_Instance = VK_NULL_HANDLE;
        Ref<VulkanDevice> m_Device;

        VkFormat m_ColorFormat = VK_FORMAT_UNDEFINED;
        VkColorSpaceKHR m_ColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

        VkSwapchainKHR m_SwapChain = nullptr;
        uint32_t m_ImageCount = 0;

        struct SwapChainImage
        {
            VkImage Image;
            VkImageView ImageView;
        };
        std::vector<SwapChainImage> m_Images;

        struct FrameCommandBuffer
        {
            VkCommandPool CommandPool = VK_NULL_HANDLE;
            VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
        };
        std::vector<FrameCommandBuffer> m_CommandBuffers;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_WaitFences;

        std::vector<VkFramebuffer> m_Framebuffers;
        VkRenderPass m_RenderPass = VK_NULL_HANDLE;

        uint32_t m_CurrentFrameIndex = 0;
        uint32_t m_CurrentImageIndex = 0;

        uint32_t m_QueueNodeIndex = UINT32_MAX;
        uint32_t m_Width = 0, m_Height = 0;
        bool m_VSync = false;

        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    };
}
