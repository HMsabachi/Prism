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

		// -----------------ToString-----------------------
		std::string PRISM_API ToString(const Vector2& value)
		{
			return fmt::format("Vector2(x: {:.2f}, y: {:.2f})", value.x, value.y);
		}
		std::string PRISM_API ToString(const Vector3& value)
		{
			return fmt::format("Vector3(x: {:.2f}, y: {:.2f}, z: {:.2f})", value.x, value.y, value.z);
		}
		std::string PRISM_API ToString(const Vector4& value)
		{
			return fmt::format("Vector4(x: {:.2f}, y: {:.2f}, z: {:.2f}, w: {:.2f})", value.x, value.y, value.z, value.w);
		}
		std::string PRISM_API ToString(const Range& value)
		{
			return fmt::format("Range(min: {:.2f}, max: {:.2f}, value: {:.2f})", value.min, value.max, value.value);
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