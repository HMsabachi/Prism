import PrismNative as _Prism
from Prism.Math.Vector2 import Vector2


class MouseButton:
    Button0 = 0
    Button1 = 1
    Button2 = 2
    Button3 = 3
    Button4 = 4
    Button5 = 5
    Left = Button0
    Right = Button1
    Middle = Button2


class CursorMode:
    Normal = 0
    Hidden = 1
    Locked = 2


class Input:
    @staticmethod
    def IsKeyPressed(keycode: int) -> bool:
        return _Prism.Prism_Input_IsKeyPressed(keycode)

    @staticmethod
    def IsMouseButtonPressed(button: int) -> bool:
        return _Prism.Prism_Input_IsMouseButtonPressed(button)

    @staticmethod
    def GetMousePosition() -> Vector2:
        return Vector2(_Prism.Prism_Input_GetMousePosition())

    @staticmethod
    def SetCursorMode(mode: int):
        _Prism.Prism_Input_SetCursorMode(mode)

    @staticmethod
    def GetCursorMode() -> int:
        return _Prism.Prism_Input_GetCursorMode()
