import PrismNative as _Prism


class TimeMeta(type):
    @property
    def DeltaTime(cls) -> float:
        return _Prism.Prism_Time_GetDeltaTime()

    @property
    def UnscaledDeltaTime(cls) -> float:
        return _Prism.Prism_Time_GetUnscaledDeltaTime()

    @property
    def Time(cls) -> float:
        return _Prism.Prism_Time_GetTime()

    @property
    def UnscaledTime(cls) -> float:
        return _Prism.Prism_Time_GetUnscaledTime()

    @property
    def FixedDeltaTime(cls) -> float:
        return _Prism.Prism_Time_GetFixedDeltaTime()

    @property
    def FrameCount(cls) -> int:
        return _Prism.Prism_Time_GetFrameCount()

    @property
    def TimeScale(cls) -> float:
        return _Prism.Prism_Time_GetTimeScale()

    @TimeScale.setter
    def TimeScale(cls, value: float) -> None:
        _Prism.Prism_Time_SetTimeScale(value)

    @FixedDeltaTime.setter
    def FixedDeltaTime(cls, value: float) -> None:
        _Prism.Prism_Time_SetFixedDeltaTime(value)


class Time(metaclass=TimeMeta):
    pass
