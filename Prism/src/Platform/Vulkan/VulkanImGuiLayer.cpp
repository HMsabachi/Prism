#include "prpch.h"
#include "VulkanImGuiLayer.h"
#include "Prism/Core/Application.h"

#include "Prism/Core/Log.h"
#include "Prism/Renderer/Renderer.h"

#include "imgui.h"
#include "Prism/ImGui/ImGuizmo.h"

#include "VulkanContext.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include <filesystem>
#include <GLFW/glfw3.h>

namespace Prism
{

    static VkCommandBuffer s_ImGuiCommandBuffer[VulkanFramesInFlight];

    static void check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            abort();
    }


    VulkanImGuiLayer::VulkanImGuiLayer() : ImGuiLayer()
    {

    }
    VulkanImGuiLayer::VulkanImGuiLayer(const std::string& name) : ImGuiLayer()
    {

    }
    VulkanImGuiLayer::~VulkanImGuiLayer()
    {

    }


    void VulkanImGuiLayer::Begin()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
    }
    void VulkanImGuiLayer::End()
    {
        ImGui::Render();

        Ref<VulkanContext> context = VulkanContext::Get();
        VulkanSwapChain& swapChain = context->GetSwapChain();
        VkCommandBuffer drawCommandBuffer = swapChain.GetCurrentDrawCommandBuffer();
        VkCommandBuffer imGuiCommandBuffer = s_ImGuiCommandBuffer[swapChain.GetCurrentFrameIndex()];

        // 交换链 renderpass 仅 1 个 color attachment（无 depth）
        VkClearValue clearValues[1];
        clearValues[0].color = { {0.1f, 0.1f,0.1f, 1.0f} };

        uint32_t width = swapChain.GetWidth();
        uint32_t height = swapChain.GetHeight();

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.pNext = nullptr;
        renderPassBeginInfo.renderPass = swapChain.GetRenderPass();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = width;
        renderPassBeginInfo.renderArea.extent.height = height;
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = clearValues;
        renderPassBeginInfo.framebuffer = swapChain.GetCurrentFramebuffer();

        vkCmdBeginRenderPass(drawCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        VkCommandBufferInheritanceInfo inheritanceInfo = {};
        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritanceInfo.renderPass = swapChain.GetRenderPass();
        inheritanceInfo.framebuffer = swapChain.GetCurrentFramebuffer();

        VkCommandBufferBeginInfo cmdBufInfo = {};
        cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        cmdBufInfo.pInheritanceInfo = &inheritanceInfo;

        VK_CHECK_RESULT(vkBeginCommandBuffer(imGuiCommandBuffer, &cmdBufInfo));

        ImDrawData* main_draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(main_draw_data, imGuiCommandBuffer);

        VK_CHECK_RESULT(vkEndCommandBuffer(imGuiCommandBuffer));

        std::vector<VkCommandBuffer> commandBuffers;
        commandBuffers.push_back(imGuiCommandBuffer);

        vkCmdExecuteCommands(drawCommandBuffer, (uint32_t)commandBuffers.size(), commandBuffers.data());

        vkCmdEndRenderPass(drawCommandBuffer);

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }


    void VulkanImGuiLayer::OnAttach()
    {
        InitializeImGui();
        SetDarkThemeColors();
    }
    void VulkanImGuiLayer::OnDetach()
    {
        DestroyImGui();
    }
    void VulkanImGuiLayer::OnImGuiRender()
    {

    }


    void VulkanImGuiLayer::InitializeImGui()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext(); // Create the ImGui context 创建ImGui上下文
        ImGui::StyleColorsDark(); // Set the color scheme to dark mode  设置颜色模式为暗黑模式

        ImGuiIO& io = ImGui::GetIO(); // Get the IO structure 获取IO结构体
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // Enable mouse cursor  启用鼠标光标
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos; // Enable setting mouse position  启用设置鼠标位置

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking  启用停靠
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable viewports  启用viewports（RT 验证：imgui_impl_glfw multi-viewport 非主线程安全，暂关）
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard navigation  启用键盘导航

        // 构建多语言字形范围
        static const ImWchar jpGlyphRanges[] =
        {
            0x0020, 0x00FF, // ASCII + Latin-1
            0x3000, 0x30FF, // CJK 符号和标点、平假名、片假名
            0x31F0, 0x31FF, // 片假名语音扩展
            0xFF00, 0xFFEF, // 全角 ASCII、半角片假名
            0x4E00, 0x9FFF, // CJK 统一表意文字
            0
        };

        ImFont* pFont = io.Fonts->AddFontFromFileTTF("Assets\\Fonts\\TTF\\LXGWWenKai-Medium.ttf", 25.0f, nullptr, jpGlyphRanges);
        io.FontDefault = io.Fonts->Fonts.back();

        //SetKeyMap(io); // Set disable 暂时禁用
        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        // 当 viewports 启用时，我们调整 WindowRounding/WindowBg 以使平台窗口看起来与常规窗口相同。
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, style.Colors[ImGuiCol_WindowBg].w);

        Application& app = Application::Get();
        GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

        VulkanImGuiLayer* instance = this;
        Renderer::Submit([instance]()
        {
            Application& app = Application::Get();
            GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

            auto vulkanContext = VulkanContext::Get();
            auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

            VkDescriptorPool descriptorPool;

            // Create Descriptor Pool
            VkDescriptorPoolSize pool_sizes[] =
            {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
            };
            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
            pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
            pool_info.pPoolSizes = pool_sizes;
            auto err = vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool);
            check_vk_result(err);

            // Setup Platform/Renderer bindings
            ImGui_ImplGlfw_InitForVulkan(window, true);
            ImGui_ImplVulkan_InitInfo init_info = {};
            init_info.Instance = VulkanContext::GetInstance();
            init_info.PhysicalDevice = VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetVulkanPhysicalDevice();
            init_info.Device = device;
            init_info.QueueFamily = VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetQueueFamilyIndices().Graphics;
            init_info.Queue = VulkanContext::GetCurrentDevice()->GetQueue();
            init_info.PipelineCache = nullptr;
            init_info.DescriptorPool = descriptorPool;
            init_info.Allocator = nullptr;
            init_info.MinImageCount = 2;
            init_info.ImageCount = vulkanContext->GetSwapChain().GetImageCount();
            init_info.CheckVkResultFn = check_vk_result;
            ImGui_ImplVulkan_Init(&init_info, vulkanContext->GetSwapChain().GetRenderPass());

            // Load Fonts
            // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
            // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
            // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
            // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
            // - Read 'docs/FONTS.md' for more instructions and details.
            // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
            //io.Fonts->AddFontDefault();
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
            //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
            //IM_ASSERT(font != NULL);

            // Upload Fonts
            {
                // Use any command queue

                VkCommandBuffer commandBuffer = vulkanContext->GetCurrentDevice()->GetCommandBuffer(true);
                ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
                vulkanContext->GetCurrentDevice()->FlushCommandBuffer(commandBuffer);

                err = vkDeviceWaitIdle(device);
                check_vk_result(err);
                ImGui_ImplVulkan_DestroyFontUploadObjects();
            }

            for (uint32_t i = 0; i < VulkanFramesInFlight; i++)
                s_ImGuiCommandBuffer[i] = VulkanContext::GetCurrentDevice()->CreateSecondaryCommandBuffer();
        });
    }


    void VulkanImGuiLayer::DestroyImGui()
    {
        Renderer::Submit([]()
        {
            auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

            auto err = vkDeviceWaitIdle(device);
            check_vk_result(err);
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        });
    }

}
