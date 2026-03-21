#pragma once

#include "Prism/Core.h"

#include <string>
#include <functional>


namespace Prism
{
	// TODO： 这里的注释做提示作用记得删除
	// Events in Prism are currently blocking, meaning when an event occurs it
	// immediately gets dispatched and must be dealt with right then an there.
	// For the future, a better strategy might be to buffer events in an event
	// bus and process them during the "event" part of the update stage.

	// 事件系统在Prism中是阻塞式的，这意味着当事件发生时，它立即被派发，必须立即处理。
	// 对于未来，更好的策略可能是将事件缓存在事件总线中，并在“事件”部分的更新阶段处理它们。

	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput = BIT(1),
		EventCategoryKeyboard = BIT(2),
		EventCategoryMouse = BIT(3),
		EventCategoryMouseButton = BIT(4)
	};

	/// Event class macros for easy creation of event classes
	/// 事件类宏用于简化创建事件类的过程
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class PRISM_API Event
	{
		friend class EventDispatcher;
	public:
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;

		/// It is only used in debugging to return a string(Debug tool)
		/// 一般只在调试中使用返回一个字符串(调试工具)
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
	protected:
		bool m_Handled = false; /// Is event handled 事件是否被处理过
	};


	/// Event dispatcher class to dispatch events to event handlers
	/// 事件调度器类用于将事件派发到事件处理程序中
	class EventDispatcher
	{
		template<typename T>
		using EventFn = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& event)
			: m_Event(event)
		{
		}

		// TODO: 这个函数目前还不能理解
		template<typename T>
		bool Dispatch(EventFn<T> func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.m_Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};

	inline std::string format_as(const Event& e)
	{
		return e.ToString();
	}
}




