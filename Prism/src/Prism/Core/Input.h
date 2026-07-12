#pragma once
#include "Core.h"
#include "KeyCodes.h"

namespace Prism
{

    typedef enum class MouseButton : uint16_t
    {
        Button0 = 0,
        Button1 = 1,
        Button2 = 2,
        Button3 = 3,
        Button4 = 4,
        Button5 = 5,
        Left = Button0,
        Right = Button1,
        Middle = Button2
    } Button;
    inline std::ostream& operator<<(std::ostream& os, MouseButton button)
    {
        os << static_cast<int32_t>(button);
        return os;
    }
    inline auto format_as(MouseButton button) { return static_cast<int32_t>(button); }

    enum class CursorMode
    {
        Normal = 0,
        Hidden = 1,
        Locked = 2
    };

    class PRISM_API Input
    {
    public:
        inline static bool IsKeyPressed(KeyCode keycode) { return s_Instance->IsKeyPressedImpl(keycode); }

        inline static bool IsMouseButtonPressed(MouseButton button) { return s_Instance->IsMouseButtonPressedImpl(button); }
        inline static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); }
        inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
        inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }

        inline static void SetCursorMode(CursorMode mode) { s_Instance->SetCursorModeImpl(mode); }
        inline static CursorMode GetCursorMode() { return s_Instance->GetCursorModeImpl(); }
    protected:
        virtual bool IsKeyPressedImpl(KeyCode keycode) = 0;

        virtual bool IsMouseButtonPressedImpl(MouseButton button) = 0;
        virtual std::pair<float, float> GetMousePositionImpl() = 0;
        virtual float GetMouseXImpl() = 0;
        virtual float GetMouseYImpl() = 0;

        virtual void SetCursorModeImpl(CursorMode mode) = 0;
        virtual CursorMode GetCursorModeImpl() = 0;
    private:
        static Input* s_Instance;
    };

}
