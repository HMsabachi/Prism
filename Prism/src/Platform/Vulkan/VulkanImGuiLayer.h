#pragma once
#include "Prism/ImGui/ImGuiLayer.h"


namespace Prism
{
    class VulkanImGuiLayer : public ImGuiLayer
    {
    public:
        VulkanImGuiLayer();
        VulkanImGuiLayer(const std::string& name);
        virtual ~VulkanImGuiLayer();

        virtual void Begin() override;
        virtual void End() override;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnImGuiRender() override;
    private:
        void InitializeImGui();
        void DestroyImGui();
    private:
        float m_Time = 0.0f;
    };
}
