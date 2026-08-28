#pragma once

#include "Prism/Core/Window.h"
#include "Prism/Renderer/RendererContext.h"

#include <GLFW/glfw3.h>

namespace Prism {

    class WindowsWindow : public Window
    {
    public:
        WindowsWindow(const WindowProps& props);
        virtual ~WindowsWindow();

        void ProcessEvents() override;
        void SwapBuffers() override;

        inline unsigned int GetWidth() const override { return m_Data.Width; }
        inline unsigned int GetHeight() const override { return m_Data.Height; }
        virtual std::pair<float, float> GetWindowPos() const override;

        
        /// <summary>
        /// 设置事件回调函数
        /// </summary>
        /// <param name="callback">一个function指针 形如void func(Event& e)</param>
        inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
        void SetVSync(bool enabled) override;
        bool IsVSync() const override;

        void Maximize() override;

        // Get the native Windows 获取底层窗口句柄
        virtual const std::string& GetTitle() const override { return m_Data.Title; }
        virtual void SetTitle(const std::string& title) override;

        inline virtual void* GetNativeWindow() const { return m_Window; }

        virtual Ref<RendererContext> GetRenderContext() override { return m_RendererContext; }
    private:
        virtual void Init(const WindowProps& props);
        virtual void Shutdown();
    private:
        void SetGlfwEventCallback();
        void CreateGraphicsApiContext();
    private:
        GLFWwindow* m_Window;
        Ref<RendererContext> m_RendererContext;

        struct WindowData
        {
            std::string Title;
            uint32_t Width, Height;
            bool VSync = false;

            EventCallbackFn EventCallback;
        };

        WindowData m_Data;
    };

}
