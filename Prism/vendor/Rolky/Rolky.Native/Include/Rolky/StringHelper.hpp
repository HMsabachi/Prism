#pragma once

#include "Core.hpp"

namespace Rolky {

	class StringHelper
	{
	public:
		static UCString ConvertUtf8ToWide(std::string_view InString);
		static std::string ConvertWideToUtf8(UCStringView InString);
	};

}
