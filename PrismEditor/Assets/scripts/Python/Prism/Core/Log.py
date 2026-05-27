import PrismNative as _Prism


class LogLevel:
    Trace: int = 1
    Debug: int = 2
    Info: int = 4
    Warn: int = 8
    Error: int = 16
    Critical: int = 32


class Log:
    @staticmethod
    def _Log(level: int, message: str, *args) -> None:
        if args:
            message = message.format(*args)
        _Prism.Prism_Log_LogMessage(level, message)

    @staticmethod
    def Trace(message: str, *args) -> None:
        Log._Log(LogLevel.Trace, str(message), *args)

    @staticmethod
    def Debug(message: str, *args) -> None:
        Log._Log(LogLevel.Debug, str(message), *args)

    @staticmethod
    def Info(message: str, *args) -> None:
        Log._Log(LogLevel.Info, str(message), *args)

    @staticmethod
    def Warn(message: str, *args) -> None:
        Log._Log(LogLevel.Warn, str(message), *args)

    @staticmethod
    def Error(message: str, *args) -> None:
        Log._Log(LogLevel.Error, str(message), *args)

    @staticmethod
    def Critical(message: str, *args) -> None:
        Log._Log(LogLevel.Critical, str(message), *args)

Log.Info("Log API loaded successfully.")
