import PrismNative as _Prism


class TimeMeta(type):
    """Metaclass enabling Time.DeltaTime etc. as class-level properties (同 C# 静态属性)."""

    @property
    def DeltaTime(cls):
        return _Prism.Prism_Time_GetDeltaTime()

    @property
    def UnscaledDeltaTime(cls):
        return _Prism.Prism_Time_GetUnscaledDeltaTime()

    @property
    def Time(cls):
        return _Prism.Prism_Time_GetTime()

    @property
    def UnscaledTime(cls):
        return _Prism.Prism_Time_GetUnscaledTime()

    @property
    def FixedDeltaTime(cls):
        return _Prism.Prism_Time_GetFixedDeltaTime()

    @property
    def FrameCount(cls):
        return _Prism.Prism_Time_GetFrameCount()

    @property
    def TimeScale(cls):
        return _Prism.Prism_Time_GetTimeScale()

    @TimeScale.setter
    def TimeScale(cls, value):
        _Prism.Prism_Time_SetTimeScale(value)


class Time(metaclass=TimeMeta):
    """Python Time API — 用法同 C#: Time.DeltaTime, Time.TimeScale = 1.0"""
    pass
