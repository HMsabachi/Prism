#pragma once

#include "Prism/Renderer/RendererContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanSwapChain.h"

struct GLFWwindow;

namespace Prism
{
    class PRISM_API VulkanContext : public RendererContext
    {
    public:
        VulkanContext(GLFWwindow* windowHandle);
        virtual ~VulkanContext();

        virtual void Create() override;
        virtual void BeginFrame() override;
        virtual void SwapBuffers() override;
        virtual void OnResize(uint32_t width, uint32_t height) override;
        virtual void SetVSync(bool enabled) override;

        virtual uint32_t GetCurrentFrameIndex() const override { return m_SwapChain.GetCurrentFrameIndex(); }
        virtual uint32_t GetImageCount() const override { return m_SwapChain.GetImageCount(); }

        Ref<VulkanDevice> GetDevice() { return m_Device; }
        VulkanSwapChain& GetSwapChain() { return m_SwapChain; }

        static VkInstance GetInstance() { return s_VulkanInstance; }
        static VulkanContext* Get() { return s_VulkanContext; }
        static Ref<VulkanDevice> GetCurrentDevice();
    private:
        GLFWwindow* m_WindowHandle;

        Ref<VulkanPhysicalDevice> m_PhysicalDevice;
        Ref<VulkanDevice> m_Device;

        inline static VkInstance s_VulkanInstance = VK_NULL_HANDLE;
        inline static VulkanContext* s_VulkanContext = nullptr;
        VkDebugUtilsMessengerEXT m_DebugUtilsMessenger = VK_NULL_HANDLE;

        VulkanSwapChain m_SwapChain;
    };
}
