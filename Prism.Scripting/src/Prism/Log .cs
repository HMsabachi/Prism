using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Prism
{
    public class Log
    {
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
                if(Core.EngineFuncs.CoreTrace_Native != null)
                    Core.EngineFuncs.CoreTrace_Native(ptr);
            }
        }
        private unsafe static void PR_CORE_INFO_NATIVE(string message)
        {
            byte[] mes = Encoding.UTF8.GetBytes(message);
            fixed (byte* ptr = mes)
            {
                if (Core.EngineFuncs.CoreInfo_Native != null)
                    Core.EngineFuncs.CoreInfo_Native(ptr);
            }
        }
        private unsafe static void PR_CORE_WARN_NATIVE(string message)
        {
            byte[] mes = Encoding.UTF8.GetBytes(message);
            fixed (byte* ptr = mes)
            {
                if (Core.EngineFuncs.CoreWarn_Native != null)
                    Core.EngineFuncs.CoreWarn_Native(ptr);
            }
        }
        private unsafe static void PR_CORE_ERROR_NATIVE(string message)
        {
            byte[] mes = Encoding.UTF8.GetBytes(message);
            fixed (byte* ptr = mes)
            {
                if (Core.EngineFuncs.CoreError_Native != null)
                    Core.EngineFuncs.CoreError_Native(ptr);
            }
        }
        private unsafe static void PR_CORE_FATAL_NATIVE(string message)
        {
            byte[] mes = Encoding.UTF8.GetBytes(message);
            fixed (byte* ptr = mes)
            {
                if (Core.EngineFuncs.CoreFatal_Native != null)
                    Core.EngineFuncs.CoreFatal_Native(ptr);
            }
        }
    }
}
