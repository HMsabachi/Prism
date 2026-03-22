#pragma once
#include "Prism/Core.h"
#include "Prism/Events/Event.h"

namespace Prism 
{

	class PRISM_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		// Called when the layer is attached to the Application 
		// 当层被附加到应用程序时调用
		virtual void OnAttach() {} 

		// Called when the layer is detached from the Application
		// 当层从应用程序中分离时调用
		virtual void OnDetach() {}
		
		// Called when the layer is updated
		// 当层被更新时调用
		virtual void OnUpdate() {}

		// Called when an event is dispatched by the Application
		// 当应用程序分派事件时调用
		virtual void OnEvent(Event& event) {}

		// Only be used by the Debugging  仅限于调试使用
		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};

}

