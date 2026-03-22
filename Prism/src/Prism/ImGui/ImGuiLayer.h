#pragma once

#include "Prism/Layer.h"

namespace Prism
{
	class PRISM_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();
		
		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
		void OnEvent(Event& event) override;
	private:
		void InitializeImGui();
		void ImGuiRender();

	private:
		float m_Time = 0.0f;
	};
}