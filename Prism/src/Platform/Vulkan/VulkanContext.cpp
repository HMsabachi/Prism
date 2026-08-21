#include "prpch.h"
#include "Platform/Vulkan/VulkanContext.h"

#include "Prism/Core/Application.h"

#include <cstring>

#include <GLFW/glfw3.h>

#include "Platform/Vulkan/Vulkan.h"
#include "Platform/Vulkan/VulkanAllocator.h"

namespace Prism
{
#ifdef PR_DEBUG
    static bool s_Validation = true;
#else
    static bool s_Validation = false;
#endif

    static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugUtilsMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        (void)messageType;
        (void)pUserData;

        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
            return VK_FALSE;

        switch (messageSeverity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                PR_CORE_ERROR("Vulkan 验证层 Validation: {0}", pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                PR_CORE_WARN("Vulkan 验证层 Validation: {0}", pCallbackData->pMessage);
                break;
            default:
                PR_CORE_INFO("Vulkan 验证层 Validation: {0}", pCallbackData->pMessage);
                break;
        }
        return VK_FALSE;
    }

    static bool CheckDriverAPIVersionSupport(uint32_t minimumSupportedVersion)
    {
        uint32_t instanceVersion = 0;
        vkEnumerateInstanceVersion(&instanceVersion);

        if (instanceVersion < minimumSupportedVersion)
        {
            PR_CORE_FATAL("Vulkan 驱动版本过低 Incompatible Vulkan driver version!");
            PR_CORE_FATAL("  当前 Current: {0}.{1}.{2}", VK_API_VERSION_MAJOR(instanceVersion), VK_API_VERSION_MINOR(instanceVersion), VK_API_VERSION_PATCH(instanceVersion));
            PR_CORE_FATAL("  需要 Required: {0}.{1}.{2}", VK_API_VERSION_MAJOR(minimumSupportedVersion), VK_API_VERSION_MINOR(minimumSupportedVersion), VK_API_VERSION_PATCH(minimumSupportedVersion));
            return false;
        }
        return true;
    }

    VulkanContext::VulkanContext(GLFWwindow* windowHandle)
        : m_WindowHandle(windowHandle)
    {
        PR_CORE_ASSERT(windowHandle, "Window handle is null!");
        PR_CORE_ASSERT(!s_VulkanContext, "VulkanContext already exists!");
        s_VulkanContext = this;
    }

    VulkanContext::~VulkanContext()
    {
        m_SwapChain.Cleanup();

        if (m_PipelineCache)
            vkDestroyPipelineCache(m_Device->GetVulkanDevice(), m_PipelineCache, nullptr);

        VulkanAllocator::Shutdown();

        m_Device->Destroy();

        if (m_DebugUtilsMessenger != VK_NULL_HANDLE)
        {
            auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_VulkanInstance, "vkDestroyDebugUtilsMessengerEXT");
            if (vkDestroyDebugUtilsMessengerEXT)
                vkDestroyDebugUtilsMessengerEXT(s_VulkanInstance, m_DebugUtilsMessenger, nullptr);
        }

        vkDestroyInstance(s_VulkanInstance, nullptr);
        s_VulkanInstance = VK_NULL_HANDLE;
        s_VulkanContext = nullptr;
    }

    void VulkanContext::Create()
    {
        PR_CORE_INFO("VulkanContext::Create");

        PR_CORE_ASSERT(glfwVulkanSupported(), "GLFW must support Vulkan!");
        PR_CORE_ASSERT(CheckDriverAPIVersionSupport(VK_API_VERSION_1_2), "Incompatible Vulkan driver version!");

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Prism";
        appInfo.pEngineName = "Prism";
        appInfo.apiVersion = VK_API_VERSION_1_2;

#define VK_KHR_WIN32_SURFACE_EXTENSION_NAME "VK_KHR_win32_surface"
        std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        if (s_Validation)
        {
            instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }

        VkInstanceCreateInfo instanceCreateInfo = {};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext = NULL;
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();

        if (s_Validation)
        {
            const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
            uint32_t instanceLayerCount;
            vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
            std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
            vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
            bool validationLayerPresent = false;
            PR_CORE_TRACE("Vulkan Instance Layers:");
            for (const VkLayerProperties& layer : instanceLayerProperties)
            {
                PR_CORE_TRACE("  {0}", layer.layerName);
                if (strcmp(layer.layerName, validationLayerName) == 0)
                {
                    validationLayerPresent = true;
                    break;
                }
            }
            if (validationLayerPresent)
            {
                instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
                instanceCreateInfo.enabledLayerCount = 1;
            }
            else
            {
                PR_CORE_ERROR("验证层不存在 Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled");
            }
        }

        VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &s_VulkanInstance));

        if (s_Validation)
        {
            auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_VulkanInstance, "vkCreateDebugUtilsMessengerEXT");
            PR_CORE_ASSERT(vkCreateDebugUtilsMessengerEXT != NULL, "");
            VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{};
            debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugUtilsCreateInfo.pfnUserCallback = VulkanDebugUtilsMessengerCallback;
            debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

            VK_CHECK_RESULT(vkCreateDebugUtilsMessengerEXT(s_VulkanInstance, &debugUtilsCreateInfo, nullptr, &m_DebugUtilsMessenger));
        }

        m_PhysicalDevice = VulkanPhysicalDevice::Select();

        VkPhysicalDeviceFeatures enabledFeatures;
        memset(&enabledFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
        enabledFeatures.samplerAnisotropy = true;
        enabledFeatures.robustBufferAccess = true;
        m_Device = Ref<VulkanDevice>::Create(m_PhysicalDevice, enabledFeatures);

        VulkanAllocator::Init(m_Device);

        m_SwapChain.Init(s_VulkanInstance, m_Device);
        m_SwapChain.InitSurface(m_WindowHandle);

        uint32_t width = 1280, height = 720;
        m_SwapChain.Create(&width, &height);

        VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        VK_CHECK_RESULT(vkCreatePipelineCache(m_Device->GetVulkanDevice(), &pipelineCacheCreateInfo, nullptr, &m_PipelineCache));
    }

    void VulkanContext::OnResize(uint32_t width, uint32_t height)
    {
        m_SwapChain.OnResize(width, height, Application::Get().GetWindow().IsVSync());
    }

    void VulkanContext::BeginFrame()
    {
        m_SwapChain.BeginFrame();
    }

    void VulkanContext::SwapBuffers()
    {
        m_SwapChain.Present();
    }

    Ref<VulkanDevice> VulkanContext::GetCurrentDevice()
    {
        return s_VulkanContext->GetDevice();
    }
}
