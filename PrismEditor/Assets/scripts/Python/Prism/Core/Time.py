import PrismNative as _Prism


class Time:
    @staticmethod
    def GetDeltaTime():
        return _Prism.Prism_Time_GetDeltaTime()

    @staticmethod
    def GetTime():
        return _Prism.Prism_Time_GetTime()
