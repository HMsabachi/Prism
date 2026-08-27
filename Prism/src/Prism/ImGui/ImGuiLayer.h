#pragma once

#include "Prism/Core/Layer.h"

namespace Prism
{
    class PRISM_API ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer() : Layer("ImGuiLayer") {}
        virtual ~ImGuiLayer() = default;

        virtual void Begin() = 0;
        virtual void End() = 0;

        void SetDarkThemeColors();

        static ImGuiLayer* Create();
    };
}
