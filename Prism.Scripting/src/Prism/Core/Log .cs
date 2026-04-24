using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Prism
{
    public static class Log
    {
        static FunctionTable InternalCalls => Prism.InternalCalls.Funcs;
        internal enum LogLevel
        {
            Trace = 1 << 0,
            Debug = 1 << 1,
            Info = 1 << 2,
            Warn = 1 << 3,
            Error = 1 << 4,
            Critical = 1 << 5
        }

        public static void Trace(string format, params object[] parameters)
        {
            unsafe { InternalCalls.Log_LogMessage_Native(LogLevel.Trace, FormatUtils.Format(format, parameters)); }
        }

        public static void Debug(string format, params object[] parameters)
        {
            unsafe { InternalCalls.Log_LogMessage_Native(LogLevel.Debug, FormatUtils.Format(format, parameters)); }
        }

        public static void Info(string format, params object[] parameters)
        {
            unsafe { InternalCalls.Log_LogMessage_Native(LogLevel.Info, FormatUtils.Format(format, parameters)); }
        }

        public static void Warn(string format, params object[] parameters)
        {
            unsafe { InternalCalls.Log_LogMessage_Native(LogLevel.Warn, FormatUtils.Format(format, parameters)); }
        }

        public static void Error(string format, params object[] parameters)
        {
            unsafe { InternalCalls.Log_LogMessage_Native(LogLevel.Error, FormatUtils.Format(format, parameters)); }
        }

        public static void Critical(string format, params object[] parameters)
        {
            unsafe { InternalCalls.Log_LogMessage_Native(LogLevel.Critical, FormatUtils.Format(format, parameters)); }
        }

        public static void Trace(object value)
        {
            unsafe { InternalCalls.Log_LogMessage_Native(LogLevel.Trace, FormatUtils.Format(value)); }
        }

        public static void Debug(object value)
        {
            unsafe { InternalCalls.Log_LogMessage_Native(LogLevel.Debug, FormatUtils.Format(value)); }
        }

        public static void Info(object value)
        {
            unsafe { InternalCalls.Log_LogMessage_Native(LogLevel.Info, FormatUtils.Format(value)); }
        }

        public static void Warn(object value)
        {
            unsafe { InternalCalls.Log_LogMessage_Native(LogLevel.Warn, FormatUtils.Format(value)); }
        }

        public static void Error(object value)
        {
            unsafe { InternalCalls.Log_LogMessage_Native(LogLevel.Error, FormatUtils.Format(value)); }
        }

        public static void Critical(object value)
        {
            unsafe { InternalCalls.Log_LogMessage_Native(LogLevel.Critical, FormatUtils.Format(value)); }
        }

        #region Legacy
        private static string Format(string format, params object[] args)
        {
            if (format == null)
                return string.Empty;

            StringBuilder sb = new StringBuilder();
            int argIndex = 0;

            for (int i = 0; i < format.Length; i++)
            {
                if (format[i] == '{' && i + 1 < format.Length && format[i + 1] == '}')
                {
                    if (argIndex < args.Length)
                    {
                        sb.Append(args[argIndex]?.ToString());
                        argIndex++;
                    }
                    else
                    {
                        sb.Append("{}"); // 参数不够，保留原样
                    }
                    i++; // 跳过 '}'
                }
                else
                {
                    sb.Append(format[i]);
                }
            }

            return sb.ToString();
        }
        public static void PR_CORE_TRACE(string format, params object[] args)
        {
            var mes = Format(format, args);
            PR_CORE_TRACE_NATIVE(mes);
        }
        public static void PR_CORE_INFO(string format, params object[] args)
        {
            var mes = Format(format, args);
            PR_CORE_INFO_NATIVE(mes);
        }
        public static void PR_CORE_WARN(string format, params object[] args)
        {
            var mes = Format(format, args);
            PR_CORE_WARN_NATIVE(mes);
        }
        public static void PR_CORE_ERROR(string format, params object[] args)
        {
            var mes = Format(format, args);
            PR_CORE_ERROR_NATIVE(mes);
        }
        public static void PR_CORE_FATAL(string format, params object[] args)
        {
            var mes = Format(format, args);
            PR_CORE_FATAL_NATIVE(mes);
        }

        private unsafe static void PR_CORE_TRACE_NATIVE(string message)
        {
            byte[] mes = Encoding.UTF8.GetBytes(message);
            fixed (byte* ptr = mes)
            {
                if(Prism.InternalCalls.Funcs.CoreTrace_Native != null)
                    Prism.InternalCalls.Funcs.CoreTrace_Native(ptr);
            }
        }
        private unsafe static void PR_CORE_INFO_NATIVE(string message)
        {
            byte[] mes = Encoding.UTF8.GetBytes(message);
            fixed (byte* ptr = mes)
            {
                if (Prism.InternalCalls.Funcs.CoreInfo_Native != null)
                    Prism.InternalCalls.Funcs.CoreInfo_Native(ptr);
            }
        }
        private unsafe static void PR_CORE_WARN_NATIVE(string message)
        {
            byte[] mes = Encoding.UTF8.GetBytes(message);
            fixed (byte* ptr = mes)
            {
                if (Prism.InternalCalls.Funcs.CoreWarn_Native != null)
                    Prism.InternalCalls.Funcs.CoreWarn_Native(ptr);
            }
        }
        private unsafe static void PR_CORE_ERROR_NATIVE(string message)
        {
            byte[] mes = Encoding.UTF8.GetBytes(message);
            fixed (byte* ptr = mes)
            {
                if (Prism.InternalCalls.Funcs.CoreError_Native != null)
                    Prism.InternalCalls.Funcs.CoreError_Native(ptr);
            }
        }
        private unsafe static void PR_CORE_FATAL_NATIVE(string message)
        {
            byte[] mes = Encoding.UTF8.GetBytes(message);
            fixed (byte* ptr = mes)
            {
                if (Prism.InternalCalls.Funcs.CoreFatal_Native != null)
                    Prism.InternalCalls.Funcs.CoreFatal_Native(ptr);
            }
        }
    #endregion



    }
}
