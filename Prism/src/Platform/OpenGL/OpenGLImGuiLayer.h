#pragma once

#include "Prism/ImGui/ImGuiLayer.h"

namespace Prism
{
    class OpenGLImGuiLayer : public ImGuiLayer
    {
    public:
        OpenGLImGuiLayer();
        OpenGLImGuiLayer(const std::string& name);
        virtual ~OpenGLImGuiLayer();

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
