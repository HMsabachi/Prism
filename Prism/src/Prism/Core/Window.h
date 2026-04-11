#pragma once

#include "prpch.h"

#include "Prism/Core/Core.h"
#include "Prism/Events/Event.h"

namespace Prism {

	// Window properties 窗口属性
	struct WindowProps
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;

		WindowProps(const std::string& title = "Prism Engine",
			unsigned int width = 1920,
			unsigned int height = 1080)
			: Title(title), Width(width), Height(height)
		{
		}
	};

	// Interface representing a desktop system based Window
	// 一个基于桌面操作系统窗口的接口
	class PRISM_API Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		// Called when window refreshes 窗口刷新时调用
		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;
		virtual std::pair<float, float> GetWindowPos() const = 0;

		
		/// <summary>
		/// 设置事件回调函数
		/// </summary>
		/// <param name="callback">一个function指针 形如void func(Event& e)</param>
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;

		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		static Window* Create(const WindowProps& props = WindowProps());
	};

}