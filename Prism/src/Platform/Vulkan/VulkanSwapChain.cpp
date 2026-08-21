#include "prpch.h"
#include "Platform/Vulkan/VulkanSwapChain.h"

#include <GLFW/glfw3.h>

#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                        \
{                                                                       \
    fp##entrypoint = reinterpret_cast<PFN_vk##entrypoint>(vkGetInstanceProcAddr(inst, "vk"#entrypoint)); \
    PR_CORE_ASSERT(fp##entrypoint);                                     \
}

#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                           \
{                                                                       \
    fp##entrypoint = reinterpret_cast<PFN_vk##entrypoint>(vkGetDeviceProcAddr(dev, "vk"#entrypoint));   \
    PR_CORE_ASSERT(fp##entrypoint);                                     \
}

static PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
static PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
static PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
static PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
static PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
static PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
static PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
static PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
static PFN_vkQueuePresentKHR fpQueuePresentKHR;

namespace Prism
{
    void VulkanSwapChain::Init(VkInstance instance, const Ref<VulkanDevice>& device)
    {
        m_Instance = instance;
        m_Device = device;

        VkDevice vulkanDevice = m_Device->GetVulkanDevice();
        GET_DEVICE_PROC_ADDR(vulkanDevice, CreateSwapchainKHR);
        GET_DEVICE_PROC_ADDR(vulkanDevice, DestroySwapchainKHR);
        GET_DEVICE_PROC_ADDR(vulkanDevice, GetSwapchainImagesKHR);
        GET_DEVICE_PROC_ADDR(vulkanDevice, AcquireNextImageKHR);
        GET_DEVICE_PROC_ADDR(vulkanDevice, QueuePresentKHR);

        GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceSupportKHR);
        GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
        GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceFormatsKHR);
        GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfacePresentModesKHR);
    }

    void VulkanSwapChain::InitSurface(GLFWwindow* windowHandle)
    {
        VkPhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice();

        VK_CHECK_RESULT(glfwCreateWindowSurface(m_Instance, windowHandle, nullptr, &m_Surface));

        uint32_t queueCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, NULL);
        PR_CORE_ASSERT(queueCount >= 1);

        std::vector<VkQueueFamilyProperties> queueProps(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProps.data());

        std::vector<VkBool32> supportsPresent(queueCount);
        for (uint32_t i = 0; i < queueCount; i++)
        {
            fpGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_Surface, &supportsPresent[i]);
        }

        uint32_t graphicsQueueNodeIndex = UINT32_MAX;
        uint32_t presentQueueNodeIndex = UINT32_MAX;
        for (uint32_t i = 0; i < queueCount; i++)
        {
            if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
            {
                if (graphicsQueueNodeIndex == UINT32_MAX)
                {
                    graphicsQueueNodeIndex = i;
                }

                if (supportsPresent[i] == VK_TRUE)
                {
                    graphicsQueueNodeIndex = i;
                    presentQueueNodeIndex = i;
                    break;
                }
            }
        }
        if (presentQueueNodeIndex == UINT32_MAX)
        {
            for (uint32_t i = 0; i < queueCount; ++i)
            {
                if (supportsPresent[i] == VK_TRUE)
                {
                    presentQueueNodeIndex = i;
                    break;
                }
            }
        }

        PR_CORE_ASSERT(graphicsQueueNodeIndex != UINT32_MAX);
        PR_CORE_ASSERT(presentQueueNodeIndex != UINT32_MAX);

        m_QueueNodeIndex = graphicsQueueNodeIndex;

        FindImageFormatAndColorSpace();
    }

    void VulkanSwapChain::Create(uint32_t* width, uint32_t* height, bool vsync)
    {
        m_VSync = vsync;

        VkDevice device = m_Device->GetVulkanDevice();
        VkPhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice();

        VkSwapchainKHR oldSwapchain = m_SwapChain;

        VkSurfaceCapabilitiesKHR surfCaps;
        VK_CHECK_RESULT(fpGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &surfCaps));

        uint32_t presentModeCount;
        VK_CHECK_RESULT(fpGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, NULL));
        PR_CORE_ASSERT(presentModeCount > 0);

        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        VK_CHECK_RESULT(fpGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, presentModes.data()));

        VkExtent2D swapchainExtent = {};
        if (surfCaps.currentExtent.width == (uint32_t)-1)
        {
            swapchainExtent.width = *width;
            swapchainExtent.height = *height;
        }
        else
        {
            swapchainExtent = surfCaps.currentExtent;
            *width = surfCaps.currentExtent.width;
            *height = surfCaps.currentExtent.height;
        }

        m_Width = *width;
        m_Height = *height;

        if (*width == 0 || *height == 0)
            return;

        VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

        if (!vsync)
        {
            for (size_t i = 0; i < presentModeCount; i++)
            {
                if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                    break;
                }
                if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
                {
                    swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
                }
            }
        }

        uint32_t desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;
        if ((surfCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCaps.maxImageCount))
        {
            desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
        }

        VkSurfaceTransformFlagsKHR preTransform;
        if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        {
            preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        }
        else
        {
            preTransform = surfCaps.currentTransform;
        }

        VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        };
        for (auto& compositeAlphaFlag : compositeAlphaFlags)
        {
            if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag)
            {
                compositeAlpha = compositeAlphaFlag;
                break;
            }
        }

        VkSwapchainCreateInfoKHR swapchainCI = {};
        swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCI.pNext = NULL;
        swapchainCI.surface = m_Surface;
        swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
        swapchainCI.imageFormat = m_ColorFormat;
        swapchainCI.imageColorSpace = m_ColorSpace;
        swapchainCI.imageExtent = { swapchainExtent.width, swapchainExtent.height };
        swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
        swapchainCI.imageArrayLayers = 1;
        swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCI.queueFamilyIndexCount = 0;
        swapchainCI.pQueueFamilyIndices = NULL;
        swapchainCI.presentMode = swapchainPresentMode;
        swapchainCI.oldSwapchain = oldSwapchain;
        swapchainCI.clipped = VK_TRUE;
        swapchainCI.compositeAlpha = compositeAlpha;

        if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
        {
            swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        {
            swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        VK_CHECK_RESULT(fpCreateSwapchainKHR(device, &swapchainCI, nullptr, &m_SwapChain));

        if (oldSwapchain != VK_NULL_HANDLE)
            fpDestroySwapchainKHR(device, oldSwapchain, nullptr);

        for (auto& image : m_Images)
            vkDestroyImageView(device, image.ImageView, nullptr);
        m_Images.clear();

        VK_CHECK_RESULT(fpGetSwapchainImagesKHR(device, m_SwapChain, &m_ImageCount, NULL));

        std::vector<VkImage> vulkanImages(m_ImageCount);
        VK_CHECK_RESULT(fpGetSwapchainImagesKHR(device, m_SwapChain, &m_ImageCount, vulkanImages.data()));

        m_Images.resize(m_ImageCount);
        for (uint32_t i = 0; i < m_ImageCount; i++)
        {
            m_Images[i].Image = vulkanImages[i];

            VkImageViewCreateInfo colorAttachmentView = {};
            colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            colorAttachmentView.pNext = NULL;
            colorAttachmentView.format = m_ColorFormat;
            colorAttachmentView.image = m_Images[i].Image;
            colorAttachmentView.components = {
                VK_COMPONENT_SWIZZLE_R,
                VK_COMPONENT_SWIZZLE_G,
                VK_COMPONENT_SWIZZLE_B,
                VK_COMPONENT_SWIZZLE_A
            };
            colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            colorAttachmentView.subresourceRange.baseMipLevel = 0;
            colorAttachmentView.subresourceRange.levelCount = 1;
            colorAttachmentView.subresourceRange.baseArrayLayer = 0;
            colorAttachmentView.subresourceRange.layerCount = 1;
            colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
            colorAttachmentView.flags = 0;

            VK_CHECK_RESULT(vkCreateImageView(device, &colorAttachmentView, nullptr, &m_Images[i].ImageView));
        }

        CreateDrawBuffers();

        if (m_ImageAvailableSemaphores.size() != VulkanFramesInFlight)
        {
            m_ImageAvailableSemaphores.resize(VulkanFramesInFlight);
            m_RenderFinishedSemaphores.resize(VulkanFramesInFlight);
            VkSemaphoreCreateInfo semaphoreCreateInfo{};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            for (size_t i = 0; i < VulkanFramesInFlight; i++)
            {
                VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &m_ImageAvailableSemaphores[i]));
                VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &m_RenderFinishedSemaphores[i]));
            }
        }

        if (m_WaitFences.size() != VulkanFramesInFlight)
        {
            VkFenceCreateInfo fenceCreateInfo{};
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            m_WaitFences.resize(VulkanFramesInFlight);
            for (auto& fence : m_WaitFences)
            {
                VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));
            }
        }

        VkAttachmentDescription colorAttachmentDesc = {};
        colorAttachmentDesc.format = m_ColorFormat;
        colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorReference = {};
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorReference;
        subpassDescription.inputAttachmentCount = 0;
        subpassDescription.pInputAttachments = nullptr;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments = nullptr;
        subpassDescription.pResolveAttachments = nullptr;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachmentDesc;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescription;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (m_RenderPass != VK_NULL_HANDLE)
            vkDestroyRenderPass(device, m_RenderPass, nullptr);
        VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass));

        for (auto& framebuffer : m_Framebuffers)
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        m_Framebuffers.clear();

        VkFramebufferCreateInfo frameBufferCreateInfo = {};
        frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferCreateInfo.pNext = NULL;
        frameBufferCreateInfo.renderPass = m_RenderPass;
        frameBufferCreateInfo.attachmentCount = 1;
        frameBufferCreateInfo.width = m_Width;
        frameBufferCreateInfo.height = m_Height;
        frameBufferCreateInfo.layers = 1;

        m_Framebuffers.resize(m_ImageCount);
        for (uint32_t i = 0; i < m_Framebuffers.size(); i++)
        {
            frameBufferCreateInfo.pAttachments = &m_Images[i].ImageView;
            VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &m_Framebuffers[i]));
        }
    }

    void VulkanSwapChain::CreateDrawBuffers()
    {
        VkDevice device = m_Device->GetVulkanDevice();

        for (auto& commandBuffer : m_CommandBuffers)
        {
            if (commandBuffer.CommandPool != VK_NULL_HANDLE)
                vkDestroyCommandPool(device, commandBuffer.CommandPool, nullptr);
        }
        m_CommandBuffers.clear();

        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = m_QueueNodeIndex;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;

        m_CommandBuffers.resize(VulkanFramesInFlight);
        for (auto& commandBuffer : m_CommandBuffers)
        {
            VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &commandBuffer.CommandPool));

            commandBufferAllocateInfo.commandPool = commandBuffer.CommandPool;
            VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer.CommandBuffer));
        }
    }

    void VulkanSwapChain::OnResize(uint32_t width, uint32_t height, bool isVSync)
    {
        PR_CORE_WARN("交换链重建 VulkanSwapChain::OnResize {0}x{1}", width, height);

        VkDevice device = m_Device->GetVulkanDevice();

        vkDeviceWaitIdle(device);

        Create(&width, &height, isVSync);

        vkDeviceWaitIdle(device);
    }

    void VulkanSwapChain::BeginFrame()
    {
        m_CurrentImageIndex = AcquireNextImage();

        VK_CHECK_RESULT(vkResetCommandPool(m_Device->GetVulkanDevice(), m_CommandBuffers[m_CurrentFrameIndex].CommandPool, 0));
    }

    void VulkanSwapChain::Present()
    {
        VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pWaitDstStageMask = &waitStageMask;
        submitInfo.pWaitSemaphores = &m_ImageAvailableSemaphores[m_CurrentFrameIndex];
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrameIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrameIndex].CommandBuffer;
        submitInfo.commandBufferCount = 1;

        VK_CHECK_RESULT(vkResetFences(m_Device->GetVulkanDevice(), 1, &m_WaitFences[m_CurrentFrameIndex]));

        VK_CHECK_RESULT(vkQueueSubmit(m_Device->GetQueue(), 1, &submitInfo, m_WaitFences[m_CurrentFrameIndex]));

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = NULL;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_SwapChain;
        presentInfo.pImageIndices = &m_CurrentImageIndex;
        presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrameIndex];
        presentInfo.waitSemaphoreCount = 1;
        VkResult result = fpQueuePresentKHR(m_Device->GetQueue(), &presentInfo);

        if (result != VK_SUCCESS)
        {
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            {
                OnResize(m_Width, m_Height);
            }
            else
            {
                VK_CHECK_RESULT(result);
            }
        }
    }

    uint32_t VulkanSwapChain::AcquireNextImage()
    {
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % VulkanFramesInFlight;

        VK_CHECK_RESULT(vkWaitForFences(m_Device->GetVulkanDevice(), 1, &m_WaitFences[m_CurrentFrameIndex], VK_TRUE, UINT64_MAX));

        uint32_t imageIndex;
        VkResult result = fpAcquireNextImageKHR(m_Device->GetVulkanDevice(), m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrameIndex], (VkFence)nullptr, &imageIndex);
        if (result != VK_SUCCESS)
        {
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            {
                OnResize(m_Width, m_Height);
                VK_CHECK_RESULT(fpAcquireNextImageKHR(m_Device->GetVulkanDevice(), m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrameIndex], (VkFence)nullptr, &imageIndex));
            }
        }

        return imageIndex;
    }

    void VulkanSwapChain::Cleanup()
    {
        VkDevice device = m_Device->GetVulkanDevice();

        vkDeviceWaitIdle(device);

        if (m_SwapChain)
            fpDestroySwapchainKHR(device, m_SwapChain, nullptr);

        for (auto& image : m_Images)
            vkDestroyImageView(device, image.ImageView, nullptr);
        m_Images.clear();

        for (auto& commandBuffer : m_CommandBuffers)
        {
            if (commandBuffer.CommandPool != VK_NULL_HANDLE)
                vkDestroyCommandPool(device, commandBuffer.CommandPool, nullptr);
        }
        m_CommandBuffers.clear();

        if (m_RenderPass)
            vkDestroyRenderPass(device, m_RenderPass, nullptr);

        for (auto framebuffer : m_Framebuffers)
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        m_Framebuffers.clear();

        for (auto& semaphore : m_ImageAvailableSemaphores)
            vkDestroySemaphore(device, semaphore, nullptr);
        m_ImageAvailableSemaphores.clear();

        for (auto& semaphore : m_RenderFinishedSemaphores)
            vkDestroySemaphore(device, semaphore, nullptr);
        m_RenderFinishedSemaphores.clear();

        for (auto& fence : m_WaitFences)
            vkDestroyFence(device, fence, nullptr);
        m_WaitFences.clear();

        if (m_Surface)
            vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        m_Surface = VK_NULL_HANDLE;
        m_SwapChain = VK_NULL_HANDLE;
    }

    void VulkanSwapChain::FindImageFormatAndColorSpace()
    {
        VkPhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice();

        uint32_t formatCount;
        VK_CHECK_RESULT(fpGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, NULL));
        PR_CORE_ASSERT(formatCount > 0);

        std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
        VK_CHECK_RESULT(fpGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, surfaceFormats.data()));

        if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
        {
            m_ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
            m_ColorSpace = surfaceFormats[0].colorSpace;
        }
        else
        {
            bool found_B8G8R8A8_UNORM = false;
            for (auto&& surfaceFormat : surfaceFormats)
            {
                if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
                {
                    m_ColorFormat = surfaceFormat.format;
                    m_ColorSpace = surfaceFormat.colorSpace;
                    found_B8G8R8A8_UNORM = true;
                    break;
                }
            }

            if (!found_B8G8R8A8_UNORM)
            {
                m_ColorFormat = surfaceFormats[0].format;
                m_ColorSpace = surfaceFormats[0].colorSpace;
            }
        }
    }
}
