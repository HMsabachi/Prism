import PrismNative as _Prism
from Prism.Math.Vector2 import Vector2


class CursorMode:
    Normal = 0
    Hidden = 1
    Locked = 2


class Input:
    @staticmethod
    def IsKeyPressed(keycode: int) -> bool:
        return _Prism.Prism_Input_IsKeyPressed(keycode)

    @staticmethod
    def GetMousePosition() -> Vector2:
        return Vector2(_Prism.Prism_Input_GetMousePosition())

    @staticmethod
    def SetCursorMode(mode: int):
        _Prism.Prism_Input_SetCursorMode(mode)

    @staticmethod
    def GetCursorMode() -> int:
        return _Prism.Prism_Input_GetCursorMode()
