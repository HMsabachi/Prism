import PrismNative as _Prism


class Input:
    @staticmethod
    def IsKeyPressed(keycode: int) -> bool:
        return _Prism.Prism_Input_IsKeyPressed(keycode)
