#pragma once
#include <string>
#include <filesystem>
#include "ShaderParserData.h"

namespace Prism
{
	class ShaderParser
	{
	public:
		static void Init(const std::string& includeRoot);

		ParseResult Parse(const std::string& source);

	private:
		static std::string s_IncludeRoot;
		static std::string s_VersionHeader;
	};
}
