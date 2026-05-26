import PrismNative as _Prism


class Input:
    @staticmethod
    def IsKeyPressed(keycode):
        """检查按键是否被按下"""
        return _Prism.Prism_Input_IsKeyPressed(keycode)
