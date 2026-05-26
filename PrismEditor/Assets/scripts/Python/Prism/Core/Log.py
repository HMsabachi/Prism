import PrismNative as _Prism


class LogLevel:
    Trace = 1
    Debug = 2
    Info = 4
    Warn = 8
    Error = 16
    Critical = 32


class Log:
    @staticmethod
    def _Log(level, message):
        _Prism.Prism_Log_LogMessage(level, message)

    @staticmethod
    def Trace(message):
        Log._Log(LogLevel.Trace, message)
    
    @staticmethod
    def Debug(message):
        Log._Log(LogLevel.Debug, message)

    @staticmethod
    def Info(message):
        Log._Log(LogLevel.Info, message)

    @staticmethod
    def Warn(message):
        Log._Log(LogLevel.Warn, message)

    @staticmethod
    def Error(message):
        Log._Log(LogLevel.Error, message)

    @staticmethod
    def Critical(message):
        Log._Log(LogLevel.Critical, message)

Log.Info("Log API loaded successfully.")
