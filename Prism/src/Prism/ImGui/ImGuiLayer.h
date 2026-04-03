#pragma once

#include "Prism/Core/Layer.h"
#include "Prism/Events/MouseEvent.h"
#include "Prism/Events/KeyEvent.h"
#include "Prism/Events/ApplicationEvent.h"

struct ImGuiIO;

namespace Prism
{
	class PRISM_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		ImGuiLayer(const std::string& name);
		~ImGuiLayer();
		
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate() override;
		virtual void OnEvent(Event& event) override;
		virtual void OnImGuiRender() override;
	public:
		void Begin();
		void End();
	private:


	private:
		void InitializeImGui();
		void DestroyImGui();

	private:
		float m_Time = 0.0f;
	};
}