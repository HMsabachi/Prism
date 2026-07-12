#pragma once

#include "Prism/Core/Input.h"

namespace Prism
{

    class WindowsInput : public Input
    {
    protected:
        virtual bool IsKeyPressedImpl(KeyCode keycode) override;

        virtual bool IsMouseButtonPressedImpl(MouseButton button) override;
        virtual std::pair<float, float> GetMousePositionImpl() override;
        virtual float GetMouseXImpl() override;
        virtual float GetMouseYImpl() override;

        virtual void SetCursorModeImpl(CursorMode mode) override;
        virtual CursorMode GetCursorModeImpl() override;
    };

}

