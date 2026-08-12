#pragma once

#include "Core.h"

#include "LayerStack.h"
#include "Prism/Events/Event.h"
#include "Window.h"
#include "Prism/Core/RenderThread.h"

namespace Prism
{
    class ImGuiLayer;
    class SceneRenderer;
    class WindowCloseEvent;
    class WindowResizeEvent;

    struct PRISM_API ApplicationProps
    {
        std::string Name;
        uint32_t WindowWidth, WindowHeight;
        bool VSync = true;
        // Phase 1 默认 SingleThreaded，保证行为不变；Phase 3 切 MultiThreaded 起渲染线程。
        ThreadingPolicy CoreThreadingPolicy = ThreadingPolicy::SingleThreaded;
    };

    class PRISM_API Application
    {
    public:
        Application(const ApplicationProps& props = { "Prism Engine", 1920, 1080, true});
        virtual ~Application();

        // The main loop of the application 启动应用主循环
        void Run();

        void Close();

        // Event handling function 事件处理函数
        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);

        std::string OpenFile(const char* filter = "All\0*.*\0") const;
        std::string SaveFile(const char* filter = "All\0*.*\0") const;

        inline static Application& Get() { return *s_Instance; }
        inline Window& GetWindow() const { return *m_Window; }

        bool IsRunning() const { return m_Running; }
        static const char* GetConfigurationName();
        static const char* GetPlatformName();

        RenderThread& GetRenderThread() { return m_RenderThread; }
        uint32_t GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }
    protected:
        virtual void OnInit();
        virtual void OnShutdown();
        virtual void RenderImGui();
    private:
        void ImGuiRenderer();

    private:
        void Initialize();
        // Update function for the application(frame update) 应用更新函数(帧更新)
        void OnUpdate();
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);
    private:
        ApplicationProps m_Props;
    private:
        std::unique_ptr<Window> m_Window;
        std::unique_ptr<SceneRenderer> m_SceneRenderer;
        RenderThread m_RenderThread;
        uint32_t m_CurrentFrameIndex = 0; // TODO: 当交换链建立后，渲染线程的帧索引应由交换链提供，而不是应用自己维护。
        bool m_Running = true;
        bool m_Minimized = false;
        ImGuiLayer* m_ImGuiLayer;
        LayerStack m_LayerStack;
    private:
        static Application* s_Instance;
    };

    // To be defined in CLIENT 需要在客户端定义
    Application* CreateApplication();
}


