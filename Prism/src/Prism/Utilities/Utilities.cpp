#include "prpch.h"
#include "Utilities.h"

namespace Prism
{
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