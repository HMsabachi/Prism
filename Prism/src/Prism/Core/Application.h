#pragma once

#include "Core.h"

#include "LayerStack.h"
#include "Prism/Events/Event.h"
#include "Window.h"
#include "Prism/Core/RenderThread.h"

#include <deque>
#include <functional>
#include <mutex>

namespace Prism
{
    enum class RendererAPIType : uint8_t;
    class ImGuiLayer;
    class SceneRenderer;
    class WindowCloseEvent;
    class WindowResizeEvent;

    struct PRISM_API ApplicationProps
    {
        std::string Name;
        uint32_t WindowWidth = 1920, WindowHeight = 1080;
        bool VSync = true;
        ThreadingPolicy CoreThreadingPolicy = ThreadingPolicy::SingleThreaded;
        RendererAPIType RendererAPI = (RendererAPIType)(1);
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

        template<typename FuncT>
        void QueueEvent(FuncT&& func)
        {
            std::scoped_lock<std::mutex> lock(m_EventQueueMutex);
            m_EventQueue.emplace_back(std::forward<FuncT>(func));
        }
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
        void ProcessEvents();
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

        std::mutex m_EventQueueMutex;
        std::deque<std::function<void()>> m_EventQueue;
    private:
        static Application* s_Instance;
    };

    Application* CreateApplication(int argc, char** argv);
}


