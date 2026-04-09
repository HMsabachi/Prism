#include "prpch.h"
#include "Utilities.h"

namespace Prism
{
	namespace Type
	{
		// -----------------Range-----------------------
		Range::operator float() const
		{
			return value;
		}
		Range::Range(float min, float max, float value) : min(min), max(max), value(value)
		{
		}
		void Range::SetValue(float v)
		{
			value = std::clamp(v, min, max);
		}
		void Range::SetMin(float v)
		{
			min = v; if (value < min) value = min;
		}
		void Range::SetMax(float v)
		{
			max = v; if (value > max) value = max;
		}
		std::string Range::ToString() const
		{
			return fmt::format("Range(min: {:.2f}, max: {:.2f}, value: {:.2f})", min, max, value);
		}
		// -----------------Range-------------------------
	}
	namespace StrParse
	{
		std::string Sanitize(std::string str)
		{
			// 不要用 remove_if 删除，而是用 replace 替换为合法分隔符（空格）
			for (char& c : str)
			{
				if (c == '{' || c == '}' || c == '(' || c == ')' ||
					c == '[' || c == ']' || c == ',' || c == '=')
				{
					c = ' ';
				}
			}
			return str;
		}

	}

}