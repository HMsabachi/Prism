using System;
using System.Text;

namespace Prism
{
    internal static class FormatUtils
    {
        internal static string Format(string format, object[] parameters)
        {
            if (parameters == null || parameters.Length == 0)
                return format;

            var sb = new StringBuilder();
            int paramIndex = 0;
            int len = format.Length;

            for (int i = 0; i < len; i++)
            {
                char c = format[i];

                if (c == '{')
                {
                    if (i + 1 < len && format[i + 1] == '}')
                    {
                        sb.Append('{');
                        sb.Append(paramIndex++);
                        sb.Append('}');
                        i++;
                    }
                    else if (i + 1 < len && format[i + 1] == '{')
                    {
                        sb.Append('{');
                        i++;
                    }
                    else
                    {
                        sb.Append(c);
                    }
                }
                else if (c == '}')
                {
                    if (i + 1 < len && format[i + 1] == '}')
                    {
                        sb.Append('}');
                        i++;
                    }
                    else
                    {
                        sb.Append(c);
                    }
                }
                else
                {
                    sb.Append(c);
                }
            }

            return string.Format(sb.ToString(), parameters);
        }

        internal static string Format(object value) => value?.ToString() ?? "null";
    }
}
