#pragma once

#include <string>

namespace Rolky
{
	class DotnetServices
	{
	public:
		static bool RunMSBuild(const std::string& InSolutionPath, bool InBuildDebug = true);
	};
}
